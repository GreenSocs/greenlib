# LICENSETEXT
#
#   Copyright (C) 2007 : GreenSocs Ltd
#       http://www.greensocs.com/ , email: info@greensocs.com
#
#   Developed by :
#
#   Marcus Bartholomeu
#     GreenSocs Ltd
#
#
#   This program is free software.
#
#   If you have no applicable agreement with GreenSocs Ltd, this software
#   is licensed to you, and you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   If you have a applicable agreement with GreenSocs Ltd, the terms of that
#   agreement prevail.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
#   02110-1301  USA
#
# ENDLICENSETEXT


from gsp_sc_sa import *


##################################################
# Standalone mode (embedded change this to False)

__standalone__ = True


##################################################
# For context management
__interpreter_name__ = ""


def get_current_callback():
  # may be a spawned thread or a call-in from C++, or in elaboration #
  ph = gsp_sc_get_curr_process_handle()
  if ph in callback.active_callbacks.keys():
    st = callback.active_callbacks[ph]
    if st:  return st[-1]

def interpreter_name():
  if __standalone__:  return ""
  ccb = get_current_callback()
  if ccb:  return ccb.interpreter_name
  else:  return __interpreter_name__


##################################################
# Time constants renaming

FS = SC_FS
PS = SC_PS
NS = SC_NS
US = SC_US
MS = SC_MS
SEC = SC_SEC

time_units = {FS:"fs", PS:"ps", NS:"ns", US:"us", MS:"ms", SEC:"s"}


##################################################
# Event

class FailedEventNotify(Exception):  pass

class event:
    "The event class"

    def __init__(self, obj=None, etype=None):
        if obj == None:
            gsp_sc_event_bind(self)
        elif etype == None:
            gsp_sc_event_bind(self, obj)
        else:
            gsp_sc_event_bind(self, obj, etype)

    def __del__(self):
      if gsp_sc_event_remove:  gsp_sc_event_remove(self)

    def notify(self, time=None, tu=None):
        if time == None:
            ng = gsp_sc_event_notify(self)
        elif tu == None:
            ng = gsp_sc_event_notify(self, time)
        else:
            ng = gsp_sc_event_notify(self, time, tu)
        if not ng:  raise FailedEventNotify

    def wait(self):
        gsp_sc_wait(self)
        thread_control()


##################################################
# Event Tree

class event_tree:
  '''public interface:
    wait()
  creation:
    {event | event_tree} {"&" | "|"} {event | event_tree}
  objects of this class never exist.  It is a base class for two derived
  classes, in which the operator (& or |) is implicit.
  '''
  # constructor:  both left and right must have a method "wait"
  def __init__(self, left, right):
    self.left = left
    self.right = right

  # operator overloading for creation of event_tree objects.
  # these are the only officially 'public' constructors.
  # the arguments must both be objects with a wait() method
  def __and__(self, tree):  return event_tree_and(self, tree)
  def __or__(self, tree):  return event_tree_or(self, tree)

  # add the operator overloads to the event class as well so that
  # combinations of event become event_tree
  event.__and__ = __and__
  event.__or__ = __or__


class event_tree_and(event_tree):
  '''class for an event tree whose topmost operator is an AND
  '''
  # public interface consists of "wait", which waits for both left.wait()
  # and right.wait()
  def wait(self):
    fork([self.left.wait, self.right.wait], wait_for=2)
    thread_control()

class event_tree_or(event_tree):
  '''class for an event tree whose topmost operator is an OR
  '''
  # public interface consists of "wait", which waits for the earlier of
  # left.wait() and right.wait()
  def wait(self):
    fork([self.left.wait, self.right.wait], wait_for=1)
    thread_control()


##################################################
# Primitive Channel

# only one primitive channel proxy is created.  created from Python to
# ensure properly cleaned up when Python closes down.
_prim_channel_proxy = gsp_sc_prim_channel_proxy()

# there is a base class for primitive channels available.  user
# primitive channels should derive from it and define the
# update() method
class primitive_channel:
  def __init__(self, debug = False, name = "Update", time_unit = NS):
    self.update_requested = False
    self.debug = debug
    self.m_name = name
    self.interpreter_name = interpreter_name()
    self.time_unit = time_unit
  def request_update(self):
    if not self.update_requested:  _prim_channel_proxy.push(self.cb)
    self.update_requested = True
  def cb(self):
    self.update_requested = False
    immediate_callback(self.update, \
      self.m_name, self.debug, self.time_unit, "", self.interpreter_name)


##################################################
# Signal

class signal(primitive_channel):
    "The signal class"

    def __init__(self, value=False, *oags, **nags):
        primitive_channel.__init__(self, *oags, **nags)
        self.value = value
        self.new_value = 0
        self.changed = False
        self.m_write_event = event()

    def read(self):
        return self.value

    def write(self,new_value):
        self.new_value = new_value
        if self.value != self.new_value:
            self.request_update()
            self.m_write_event.notify(0)

    def update(self):
        if self.value != self.new_value:
            self.changed = True
            self.value = self.new_value
        else:
            self.changed = False

    def write_event(self):
        return self.m_write_event


##################################################
# FIFO

class fifo(primitive_channel):
  "The FIFO class"

  def __init__(self, max_size=0, *oags, **nags):
    primitive_channel.__init__(self, *oags, **nags)
    # private state variables
    self.fifo = {}
    self.max_size = max_size
    self.m_read_event = event()
    self.m_write_event = event()
    # visible state of FIFO - change only at update()
    self.visible_load = 0
    self.allowed_write = max_size - 1
    self.allowed_read = - 1
    # internal accounting variables - change all the time
    self.next_read = 0
    self.next_write = 0
    self.nr_pending_writes = 0
    self.nr_pending_reads = 0

  def num_available(self):
    return self.visible_load

  def num_free(self):
    if self.max_size <= 0:  return 1
    return self.max_size - self.visible_load

  def write(self, datum):
    ticket = self.next_write
    self.next_write += 1
    self.fifo[ticket] = datum
    while self.max_size > 0 and ticket > self.allowed_write:
      wait(self.m_read_event)
    self.nr_pending_writes += 1
    self.request_update()

  def nb_write(self, datum):
    if self.max_size > 0 and self.next_write > self.allowed_write:
      return False
    self.fifo[self.next_write] = datum
    self.next_write += 1
    self.nr_pending_writes += 1
    self.request_update()
    return True

  def read(self):
    ticket = self.next_read
    self.next_read += 1
    while ticket > self.allowed_read:
      wait(self.m_write_event)
    answer = self.fifo[ticket]
    self.nr_pending_reads += 1
    self.request_update()
    return answer

  def nb_read(self):
    if self.next_read > self.allowed_read:
      return (False, None)
    answer = self.fifo[self.next_read]
    self.next_read += 1
    self.nr_pending_reads += 1
    self.request_update()
    return (True, answer)

  def peek(self):
    while self.visible_load == 0:
      wait(self.m_write_event)
    oldest = self.allowed_read - self.visible_load + 1
    return self.fifo[oldest]

  def nb_peek(self):
    if self.visible_load == 0:  return (False, None)
    oldest = self.allowed_read - self.visible_load + 1
    return (True, self.fifo[oldest])

  def update(self):
    # writes
    # data has already been written to the FIFO; just make it visible
    if self.nr_pending_writes > 0:
      self.visible_load += self.nr_pending_writes
      self.allowed_read += self.nr_pending_writes
      self.nr_pending_writes = 0
      self.m_write_event.notify(0)

    # reads
    # need to pop the FIFO and make the old data invisible
    # pop _here_ rather than in [nb_]read, otherwise peek can fail
    if self.nr_pending_reads > 0:
      oldest = self.allowed_read - self.visible_load + 1
      for i in range(oldest, oldest + self.nr_pending_reads):
        self.fifo.pop(i)
      self.visible_load -= self.nr_pending_reads
      self.allowed_write += self.nr_pending_reads
      self.nr_pending_reads = 0
      self.m_read_event.notify(0)

  def write_event(self):
      return self.m_write_event

  def read_event(self):
      return self.m_read_event


##################################################
# Spawn and Callback

# In case user wants to debug some thread
import pdb

def use_winpdb():  import winpdb_gs


class promptstr:
  def __init__(self, iname, tname, tunit = NS):
    self.iname = iname
    self.tname = tname
    self.tunit = tunit
    self.tunitstr = time_units[tunit]
  def __str__(self):
    return "(%s*%s @ %.6g %s) " % \
      (self.iname, self.tname, simulation_time(self.tunit), self.tunitstr)
  def strip(self):
    return "(%s::%s)" % (self.iname, self.tname)


class debug_wrapper:
    "Wrapper to run a thread inside the debugger"

    def __init__(self, runnable, prompt):
        self.debugger = pdb.Pdb()
        self.debugger.prompt = prompt
        self.runnable = runnable
        if hasattr(runnable, "func_name"):
          self.func_name = runnable.func_name

    def __call__(self, oargs=(), nargs={}):
        return self.debugger.runcall(self.runnable, *oargs, **nargs)


# Exceptions and magic constants
class thread_kill(Exception):  pass
class thread_reset(Exception):  pass
class DuplicateThreadHandle(Exception):  pass
class REPEAT_SPAWN:  pass


class callback:
  "Manage any call from SystemC into Python that may enter user Python code"

  # tracking for callbacks and spawned threads
  active_callbacks = {}

  # Unique numbers - used only for debugging
  # do not use low numbers as this confuses Winpdb debugger
  next_callback_nr = 1000
  def callback_nr(self):
    callback.next_callback_nr += 1
    return callback.next_callback_nr

  def __init__(self, runnable, name=None, debug=False, time_unit=NS, \
      name_base="Callback", interpreter_name=None, \
      completed_event=None, args=(), keyargs={}):
    # Error check
    if not callable(runnable):
      raise TypeError, "Callback is non-callable: %s" % runnable
    # unique number
    self.nr = self.callback_nr()
    # callback name
    if name is None:
      if hasattr(runnable, "func_name"):
        self.name = runnable.func_name
      else:
        self.name = "Anon%s-%d" % (name_base, self.nr)
    else:
      self.name = name
    # name of interpreter
    if interpreter_name is None:
      self.interpreter_name = globals()["interpreter_name"]()
    else:
      self.interpreter_name = interpreter_name
    # Treat debug
    if debug:
      self.runnable = debug_wrapper(runnable, \
        promptstr(self.interpreter_name, self.name, time_unit))
    else:
      self.runnable = runnable
    # args for callback function
    self.args = args
    self.keyargs = keyargs
    # callback status information
    self.kill_raised = False
    self.reset_raised = False
    self.pause_raised = False
    self.return_value = None
    self.started = False
    self.complete = False
    self.success = False
    self.resume_event = event()
    self.completed_event = completed_event

    # now do something derived-class-specific
    self.end_of_init()

  def is_spawn(self):  return False

  def end_of_init(self):
    # for a normal callback, just wait to be called
    pass

  def kill(self):
    self.kill_raised = True

  def reset(self):
    self.reset_raised = True

  def pause(self):
    if self.pause_raised == False:
      self.pause_raised = True

  def resume(self):
    if self.pause_raised == True:
      self.pause_raised = False
      self.resume_event.notify()

  def __call__(self):
    # manage the stack, then call the callback through run()
    sc_process = gsp_sc_get_curr_process_handle()
    if sc_process not in callback.active_callbacks.keys():
      callback.active_callbacks[sc_process] = []
    else:
      if self.is_spawn():  raise DuplicateThreadHandle
    callback.active_callbacks[sc_process].append(self)
    self.started = True
    try:
      self.run()
      self.success = True
    finally:
      # executed always, then any exception re-raised
      self.complete = True
      if self.completed_event:
        self.completed_event.notify()
      callback.active_callbacks[sc_process].pop()
      if len(callback.active_callbacks[sc_process]) == 0:
        del callback.active_callbacks[sc_process]

  def run(self):
    # actually call the runnable, with kill, pause, loop, etc..
    while True:
      try:
        # launch the thread - return REPEAT_SPAWN to repeat
        return_value = self.runnable(*self.args, **self.keyargs)
        if return_value != REPEAT_SPAWN:
          self.return_value = return_value
          break
      except thread_kill:
        # kills the thread cleanly
        return_value = None
        break
      except thread_reset:
        # starts the thread again by iterating the while loop
        self.reset_raised = False


class immediate_callback(callback):
  def end_of_init(self):  self()


class spawn(callback):
  "Thread processes are a special case of callbacks"
  def is_spawn(self):  return True
  def end_of_init(self):  gsp_sc_spawn(self, self.name)


##################################################
# Functions

def start(*args):
  # no sc_module exists, so we need to do the callbacks explicitly
  end_of_elaboration()
  start_of_simulation()
  gsp_sc_start(*args)
  end_of_simulation()

stop = gsp_sc_stop
simulation_time = gsp_sc_simulation_time
delta_count = gsp_sc_delta_count
## deprecate create_thread because it bypasses the callback management ##
create_thread = gsp_sc_spawn
is_running = gsp_sc_is_running


def thread_control():
    this = get_current_callback()
    if this.kill_raised:   # kill has higher priority over reset
      raise thread_kill
    if this.reset_raised:  # reset has higher priority over pause
      raise thread_reset
    if this.pause_raised:
      wait(this.resume_event)


def wait(obj, tu=None):
    # if obj is event or event tree, call obj.wait(); else it is a scalar
    if hasattr(obj, "wait"):
        obj.wait()
        return
    if tu == None:
        gsp_sc_wait(obj)
    else:
        gsp_sc_wait(obj, tu)
    # support for thread manipulation: pause, reset, kill, etc
    thread_control()


class fork:
  def __init__(self, runnable_list, wait_for=-1, kill=False, \
      name="fork", args=None, keyargs=None, **spawnargs):

    assert gsp_sc_get_curr_process_handle() != 0, \
      "fatal error: gs.fork should be used only inside processes"

    if not args:
      args = [() for x in runnable_list]
    if not keyargs:
      keyargs = [{} for x in runnable_list]

    nr_runnables = len(runnable_list)
    if wait_for < 0 or wait_for > nr_runnables:
      wait_for = nr_runnables

    done_ev = event()
    self.spawns = [spawn(runnable_list[i], "%s[%d]" % (name,i), \
      args=args[i], keyargs=keyargs[i], completed_event=done_ev, \
      **spawnargs) for i in range(nr_runnables)]

    while len([True for s in self.spawns if s.complete]) < wait_for:
      wait(done_ev)

    if kill:
      for p in self.spawns:  p.kill()


##################################################
# Utilities

def time(tu=None):
    if tu==None: tu=NS
    return "time=%d (delta=%d)" % (simulation_time(tu), delta_count())


##################################################
# Callbacks and callback registration

# do not use the callback class because these are called from the SC
# elaboration process, not from an SC process.

# this binding can be changed by gs_winpdb
sc_callback_debug_wrapper = debug_wrapper

class sc_callback:
  def __init__(self, name):
    self.name = name
    self.all_runnables = dict()
  def __call__(self, runnable = False, \
    debug = False, time_unit = NS, args = (), keyargs = {}):
    if runnable:
      if debug:
        runnable = sc_callback_debug_wrapper(runnable, \
          promptstr(interpreter_name(), self.name, time_unit))
      innm = interpreter_name()
      if innm not in self.all_runnables:
        self.all_runnables[innm] = []
      self.all_runnables[innm].append((runnable,args,keyargs))
    else:
      a = self.all_runnables.pop(interpreter_name(),[])
      for r,args,keyargs in a:
        r(*args,**keyargs)


end_of_elaboration = sc_callback("End-of-elaboration")
start_of_simulation = sc_callback("Start-of-simulation")
end_of_simulation = sc_callback("End-of-simulation")


##################################################
# Init code

# Call the init function
gsp_sc_init()


##################################################
# Parameters

# define functions if using GreenConfig
if globals().has_key("gsp_sc_config_file"): config_file = gsp_sc_config_file
if globals().has_key("gsp_sc_config_lua"): config_lua = gsp_sc_config_lua


"""
1) GC Database Access
This allows GreenScript to act as a configuration tool or parser.

These functions use full hierarchical names.  The name of the Python
interpreter (the GreenScriptModule) is not prepended to the names supplied
by the user.
"""

class GSParamNonExistent(Exception):  pass

def add_index(name, index):
  return "%s.%s" % (name, index)

param_exists = gsp_sc_param_exists

def read_param(name):
  if not param_exists(name):  raise GSParamNonExistent
  return gsp_sc_read_param(name)

def write_param(name, val):
  if isinstance(val, bool):  val = int(val)
  gsp_sc_write_param(name, str(val))

def read_param_as_list(name):
  result = []
  index = 0
  while True:
    ni = add_index(name, index)
    if not param_exists(ni):  break
    result.append(read_param(ni))
    index += 1
  return result

def write_param_from_list(name, values):
  for i,v in enumerate(values):
    write_param(add_index(name,i), v)

def get_head(name):
  return name.split(".")[0]

def get_tail(name):
  return name[len(get_head(name))+1:]

def make_dict_of_params(name, list_of_names):
  heads = set([get_head(x) for x in list_of_names])
  breakdown = dict((x,[]) for x in heads)
  for n in list_of_names:  breakdown[get_head(n)].append(get_tail(n))
  try:
    is_array = (set(range(len(heads))) == set([float(x) for x in heads]))
  except ValueError:
    is_array = False
  if is_array:
    # make a Python list if only sequential integer names at this level
    result = []
    for head in range(len(heads)):
      tails = breakdown[str(head)]
      if tails[0]:
        # some further hierarchy
        result.append(make_dict_of_params(add_index(name,head),tails))
      else:
        # leaf
        result.append(read_param(add_index(name,head)))
  else:
    # mixed names so make a dictionary
    result = {}
    for head,tails in breakdown.iteritems():
      if tails[0]:
        # some further hierarchy
        result[head] = make_dict_of_params(add_index(name,head),tails)
      else:
        # leaf
        result[head] = read_param(add_index(name,head))
  return result

def read_param_as_dict(name):
  all_params = gsp_sc_param_list(add_index(name,"*"))
  ln = len(name) + 1
  all_params_list = [all_params.read(i)[ln:] for i in range(all_params.length())]
  return make_dict_of_params(name,all_params_list)

def write_param_from_dict(name, values):
  if isinstance(values,dict):
    # is it a dictionary?
    for n,v in values.iteritems():
      write_param_from_dict(add_index(name,n),v)
    return
  if isinstance(values,list) or isinstance(values, tuple):
    # may be an embedded list?
    for i,v in enumerate(values):
      write_param_from_dict(add_index(name,i),v)
    return
  # must be a leaf value
  write_param(name,values)


"""
2) GC Parameters
This allows GreenScript to instantiate GreenConfig "parameters".

Here the name given by the user is the local name.  The hierarchical
stem including the Python interpreter_name will be automatically
added ot it.

A gs.param object can represent a simple scalar parameter, or an
ordered sequence (vector) of parameters, or a hierarchical set of
parameters with names (a dictionary).
"""

class BadParamAccess(Exception):  pass

class param:
  LEAF = 0
  BRANCH_WITH_NAMES = 1
  ORDERED_BRANCH = 2

  def __init__(self, name,
      default="", size=0, force_string=False, mother=None):
    self.name = name
    self.force_string = force_string
    self.mother = mother
    self.param_type = None
    self.default = default
    if isinstance(default, list) or isinstance(default, tuple):
      size = len(default)
    if size > 0:  self.resize(size)

  def be_a_leaf(self):
    if self.param_type is None:
      self.param_type = param.LEAF
      if not self.mother is None:  self.mother.the_param.set_mother()
      self.the_param = gsp_sc_param(self.name, str(self.default))
    if self.param_type != param.LEAF:  raise BadParamAccess

  def be_a_thing(self, ident, thing, container_class):
    if self.param_type is None:
      self.param_type = ident
      self.children = container_class()
      if not self.mother is None:  self.mother.the_param.set_mother()
      self.the_param = thing(self.name)
    if self.param_type != ident:  raise BadParamAccess

  def be_a_branch_with_names(self):
    self.be_a_thing(param.BRANCH_WITH_NAMES, gsp_sc_ext_array_param, dict)

  def be_an_ordered_branch(self):
    self.be_a_thing(param.ORDERED_BRANCH, gsp_sc_array_param, list)

  ## Methods forcing param object to be a Leaf ##
  def raw_string(self):
    self.be_a_leaf()
    return self.the_param.get_value()

  def __call__(self, new_val = None):
    self.be_a_leaf()
    if new_val is None:
      s = self.the_param.get_value()
      if self.force_string:  return s
      try:  se = eval(s, {}, {})
      except:  se = s
      return se
    else:
      if isinstance(new_val, bool):  new_val = int(new_val)
      self.the_param.set_value(str(new_val))

  def __int__(self):  return int(self())
  def __long__(self):  return long(self())
  def __float__(self):  return float(self())
  def __complex__(self):  return complex(self())

  def set_param_as_current(self):
    self.be_a_leaf()
    self.the_param.set_param_as_current()

  ## Methods forcing param object to be a Branch with Names ##
  def named_item(self, name, new_val = None):
    self.be_a_branch_with_names()
    if name not in self.children.keys():
      try:  cdef = self.default[name]
      except:  cdef = ""
      self.children[name] = param(name, cdef, 0, self.force_string, self)
    if new_val is None:
      return self.children[name]
    else:
      self.children[name].write(new_val)

  def write_branch_with_names(self, new_val):
    self.be_a_branch_with_names()
    for k,v in new_val.iteritems():  self.named_item(k, v)

  # iterate over items added explicitly by user
  # the idea is NOT to let the user just find out the structure
  # from the db at run time.
  def iterkeys(self):
    self.be_a_branch_with_names()
    return self.children.iterkeys()

  def iteritems(self):
    self.be_a_branch_with_names()
    return self.children.iteritems()

  def keys(self):
    self.be_a_branch_with_names()
    return self.children.keys()

  def items(self):
    self.be_a_branch_with_names()
    return self.children.items()

  def has_key(self, key):
    self.be_a_branch_with_names()
    return self.children.has_key(key)

  ## Methods forcing param object to be an Ordered Branch ##
  def __len__(self):
    self.be_an_ordered_branch()
    return self.the_param.get_length()

  def resize(self, new_len):
    self.be_an_ordered_branch()
    self.the_param.set_length(new_len)

  def ordered_item(self, index, new_val = None):
    self.be_an_ordered_branch()
    for i in range(len(self.children), index+1):
      try:  cdef = self.default[i]
      except:  cdef = ""
      self.children.append(param(i, cdef, 0, self.force_string, self))
    if new_val is None:
      return self.children[index]
    else:
      self.children[index].write(new_val)

  def write_ordered_branch(self, new_val):
    self.be_an_ordered_branch()
    self.the_param.set_length(len(new_val))
    for i,v in enumerate(new_val):  self.ordered_item(i, v)

  # different from a branch with names, in this case the
  # user may want to find out what is in the db, so work with the
  # parameter itself.
  class iterator:
    def __init__(self, container):
      self.the_container = container
      self.index = 0
    def __iter__(self):  return self
    def next(self):
      if self.index >= len(self.the_container):  raise StopIteration
      rv = self.the_container[self.index]
      self.index += 1
      return rv

  def __iter__(self):
    self.be_an_ordered_branch()
    return param.iterator(self)

  itervalues = __iter__
  def values(self):  return [v for v in self]

  def __contains__(self, val):
    self.be_an_ordered_branch()
    for v in self:
      if v() == val:  return True
    return False

  ## Methods operating on more than one type of param object ##
  def is_int(self, i):
    try:  return (int(i) == i)
    except ValueError:  return False

  def __getitem__(self, key):
    if self.is_int(key):  return self.ordered_item(key)
    else:  return self.named_item(key)

  def __setitem__(self, key, new_val):
    if self.is_int(key):  return self.ordered_item(key, new_val)
    else:  return self.named_item(key, new_val)

  def write(self, new_val):
    if isinstance(new_val, list) or isinstance(new_val, tuple):
      self.write_ordered_branch(new_val)
      return
    if isinstance(new_val, dict):
      self.write_branch_with_names(new_val)
      return
    self(new_val)

  def __nonzero__(self):
    return not self.param_type is None

  def csl1str(self, x):
    l = len(x)
    if l == 0:  return ""
    t = str(x[0])
    if l == 1:  return t
    return "%s, %s" % (t, self.csl1str(x[1:]))

  def csl2str(self, x):
    l = len(x)
    if l == 0:  return ""
    t = "(%s: %s)" % (x[0][0], x[0][1])
    if l == 1:  return t
    return "%s, %s" % (t, self.csl2str(x[1:]))

  def __str__(self):
    if self.param_type is None:
      return "<uninitialised param called %s>" % self.name
    if self.param_type == param.LEAF:
      return self.raw_string()
    if self.param_type == param.BRANCH_WITH_NAMES:
      return "{%s}" % self.csl2str(self.items())
    if self.param_type == param.ORDERED_BRANCH:
      return "[%s]" % self.csl1str(self.values())
    raise Exception


##################################################
# Analysis (output)

class av:

    # copy output types to this scope (if they exist)
    if globals().has_key("NULL_OUT"): NULL_OUT = NULL_OUT
    if globals().has_key("DEFAULT_OUT"): DEFAULT_OUT = DEFAULT_OUT
    if globals().has_key("TXT_FILE_OUT"): TXT_FILE_OUT = TXT_FILE_OUT
    if globals().has_key("STDOUT_OUT"): STDOUT_OUT = STDOUT_OUT
    if globals().has_key("CSV_FILE_OUT"): CSV_FILE_OUT = CSV_FILE_OUT
    if globals().has_key("SCV_STREAM_OUT"): SCV_STREAM_OUT = SCV_STREAM_OUT

    #output = gsp_sc_gav_output
    class output:

        def __init__(self, output_type, name):
            self.output = gsp_sc_gav_output(output_type, name)

        def add(self, p):
            if isinstance(p, str):
                if not self.output.add(p):
                    raise NameError("parameter not found: " + p)
            else:
                if isinstance(p, param):
                    p.set_param_as_current()
                    self.output.add()
                else:
                    raise TypeError

        def add_all(self):
            self.output.add_all()

        def remove(self, p):
            if isinstance(p, str):
                if not self.output.remove(p):
                    raise NameError("parameter not found: " + p)
            else:
                if isinstance(p, param):
                    p.set_param_as_current()
                    self.output.remove()
                else:
                    raise TypeError

        def pause(self, *args):
            self.output.pause(*args)

        def resume(self):
            self.output.resume()



##################################################
# SystemC Wrapped Signal

class BadSignalName(Exception):  pass

class sc_signal:
    "The wrapped signal class, that is a proxy to a real sc_signal"

    def __init__(self, name):
        if not gsp_sc_signal_bind(self, name):  raise BadSignalName
        self.m_write_event = event(self)

    def read(self):
        return gsp_sc_signal_read(self)

    def write(self,new_value):
        gsp_sc_signal_write(self, new_value)

    def write_event(self):
        return self.m_write_event


##################################################
# SystemC Wrapped FIFO

class BadFifoName(Exception):  pass

class sc_fifo:
    "The wrapped fifo class, that is a proxy to a real sc_fifo"

    def __init__(self, name):
        if not gsp_sc_fifo_bind(self, name):  raise BadFifoName
        self.m_read_event = event(self, EVENT_READ)
        self.m_write_event = event(self, EVENT_WRITE)

    def read(self):
        return gsp_sc_fifo_read(self)
        # blocking call into C++:  GS thread may be reset/killed/paused
        thread_control()

    def nb_read(self):
        return gsp_sc_fifo_nb_read(self)

    def num_available(self):
        return gsp_sc_fifo_num_available(self)

    def write(self, val):
        gsp_sc_fifo_write(self, val)
        # blocking call into C++:  GS thread may be reset/killed/paused
        thread_control()

    def nb_write(self, val):
        gsp_sc_fifo_nb_write(self, val)

    def num_free(self):
        return gsp_sc_fifo_num_free(self)

    def write_event(self):
        return self.m_write_event

    def read_event(self):
        return self.m_read_event


##################################################
# GreenMessage

# Message
class message(gsp_sc_msg):
  def __init__(self, *oargs, **nargs):
    if oargs:  gsp_sc_msg.__init__(self, oargs)
    else:  gsp_sc_msg.__init__(self, nargs)
  def update(self, d = None, **nargs):
    if d:  gsp_sc_msg.update(self, d)
    else:  gsp_sc_msg.update(self, nargs)

class message_ref(gsp_sc_msg_ref):
  def update(self, d = None, **nargs):
    if d:  gsp_sc_msg_ref.update(self, d)
    else:  gsp_sc_msg_ref.update(self, nargs)

# inform gsp_sc how to create references to (sub-)messages
gsp_sc_init_msg_classes(message, message_ref)

def ismessage(m):
  return isinstance(m, message) or isinstance(m, message_ref)


# writeif
class writeif:
  """ wrapper class allowing detection of kill, pause, reset events
      during callbacks into C++ user code (which can call back into Python)
  """
  def __init__(self, p):
    self.the_writeif = p

  def write(self, datum):
    self.the_writeif.write(datum)
    thread_control()

  def nb_write(self, datum):
    rv = self.the_writeif.nb_write(datum)
    thread_control()
    return rv

  def num_free(self):
    rv = self.the_writeif.num_free()
    thread_control()
    return rv

  def data_read_event(self):
    rv = event(self.the_writeif, EVENT_WRITEIF)
    thread_control()
    return rv


class NbWriteFailed(Exception): pass

# writeif_base
class writeif_base:
  # class-wide variable for unique name creation
  next_unique_name = 0
  # constructor
  def __init__(self, name = None, debug = False, time_unit = NS):
    if name is None:
      name = "gs_writeif_base_%d" % (writeif_base.next_unique_name)
      writeif_base.next_unique_name += 1
    self.name_end = name
    i = interpreter_name()
    if i:  name = i + "." + name
    self.full_name = name
    self.interpreter_name = i
    self.time_unit = time_unit
    self.debug = debug
    self.gsp_writeif_base = gsp_sc_writeif_base(name, self)
    # ticket-based algorithm to maintain first-in-first-out order
    self._next_ticket = 0
    self._next_write = 0
    self._private_event = event()
    self.m_data_read_event = event()

  def name(self): return self.full_name

  # callback functions: these are what SC actually uses
  def write_callback(self):
    immediate_callback(self.write, \
      self.name_end+"*write", self.debug, self.time_unit, "", \
      self.interpreter_name, args=(gsp_sc_msg(),))
  def nb_write_callback(self):
    return immediate_callback(self.nb_write, \
      self.name_end+"*nb_write", self.debug, self.time_unit, "", \
      self.interpreter_name, args=(gsp_sc_msg(),)).return_value
  def num_free_callback(self):
    return immediate_callback(self.num_free, \
      self.name_end+"*num_free", self.debug, self.time_unit, "", \
      self.interpreter_name).return_value
  def data_read_event_callback(self):
    return immediate_callback(self.data_read_event, \
      self.name_end+"*data_read_event", self.debug, self.time_unit, "", \
      self.interpreter_name).return_value

  # default implementations
  def num_free(self):
    return 1
  def data_read_event(self):
    return self.m_data_read_event

class writeif_base_blocking(writeif_base):
  """ Use this as the base class if you only define a blocking write(msg)
      method and not nb_write(msg) or num_free()
  """
  def write_callback(self):
    tmp = int(self._next_ticket)
    self._next_ticket += 1
    immediate_callback(self._ordered_write, \
      self.name_end+"*write", self.debug, self.time_unit, "", \
      self.interpreter_name, args=(gsp_sc_msg(),tmp))
  def nb_write(self, msg):
    tmp = int(self._next_ticket)
    self._next_ticket += 1
    spawn(self._ordered_write, \
      self.name_end+"*write", self.debug, self.time_unit, \
      interpreter_name=self.interpreter_name, args=(msg, tmp))
    return True
  def _ordered_write(self, msg, my_ticket):
    while my_ticket != self._next_write:
      wait(self._private_event)
    self._next_write += 1
    self._private_event.notify()
    self.write(msg)

class writeif_base_nonblocking(writeif_base):
  """ Use this as the base class if you define a nonblocking
      nb_write(msg) and a num_free() method but not a write(msg)
  """
  def nb_write_callback(self):
    if self._next_ticket != self._next_write:
      return False
    return immediate_callback(self.nb_write, \
      self.name_end+"*nb_write", self.debug, self.time_unit, "", \
      self.interpreter_name, args=(gsp_sc_msg(),)).return_value
  def write(self, msg):
    my_ticket = self._next_ticket
    self._next_ticket += 1
    while True:
      if my_ticket==self._next_write and self.nb_write(msg):
        self._next_write += 1
        self._private_event.notify()
        return
      wait(self.m_data_read_event | self._private_event)

class writeif_base_always(writeif_base):
  """ Use this as the base class if you define a nonblocking
      nb_write(msg) that always succeeds, but neither num_free()
      nor write(msg)
  """
  def write(self, msg):
    if not self.nb_write(msg):  raise NbWriteFailed


# msgfifo
class msgfifo(fifo, writeif_base):
  def __init__(self, name_end = None, size = 16):
    fifo.__init__(self, size)
    writeif_base.__init__(self, name_end)
  def data_read_event(self):
      return self.m_read_event



class BadReceiverName(Exception): pass

# findReceiver
def findReceiver(name):
  t = gsp_sc_findReceiver(name)
  if t.invalid(): raise BadReceiverName(name)
  return writeif(t)

