# LICENSETEXT
#
#   Copyright (C) 2007 : GreenSocs Ltd
#       http://www.greensocs.com/ , email: info@greensocs.com
#
#   Developed by :
#
#   James Aldis
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


"""
Wrapper class to create runnables that are remotely debuggable by WinPdb.


Status:
  Working.  "import winpdb_gs" will change gs debugger to be WinPdb not Pdb
  Thread ID and thread name are visible in WinPdb thread list.
  Got feedback from WinPdb owner saying the hacks are safe for the time being

Issues:
  Does not work for standalone GreenScript, only for embedded.
"""


import rpdb2
import gs


print "Creating an Endpoint for a WinPdb Remote Debugger"
print "You will now be prompted for a password, then the simulation"
print "will stall 5 mins or until a debugger command is received"
rpdb2.start_embedded_debugger_interactive_password(
  fAllowRemote = True
)

## "go" from here to reach user code, after setting breakpoints ##
sc_elab_threads = set()


class sc_callback_wind:
    """
    Wrapper to set a breakpoint with the Winpdb debugger, for end-of-elab, etc
    called from the elaboration thread of SystemC
    """
    def __init__(self, runnable, prompt):
        self.runnable = runnable
        if hasattr(runnable, "func_name"):
          self.func_name = runnable.func_name
        global sc_elab_threads
        sc_elab_threads.add(orig_get_ident())

    def __call__(self, oargs=(), nargs={}):
        self.runnable(*oargs, **nargs)

class wind(sc_callback_wind):
    "Wrapper to run a thread with the Winpdb debugger"
    def __call__(self, oargs=(), nargs={}):
        rpdb2.settrace()
        return self.runnable(*oargs, **nargs)

gs.sc_callback_debug_wrapper = sc_callback_wind
gs.debug_wrapper = wind


def get_ident():
  "replacement function for thread.get_ident() which is gs-aware"
  tmp = orig_get_ident()
  if tmp in sc_elab_threads:
    cb = gs.get_current_callback()
    if cb:  return cb.nr
  return tmp


def get_name():
  "function to get the name of the active gs thread if possible"
  try:
    cb = gs.get_current_callback()
    inm = cb.interpreter_name
    tnm = cb.name
    return "%s*%s" % (inm,tnm)
  except AttributeError:
    return rpdb2.threading._newname("Dummy-%d")


class wind_DummyThread(rpdb2.threading.Thread):
    "replacement for threading._DummyThread with gs thread name"

    def __init__(self):
        rpdb2.threading.Thread.__init__(self, name=get_name())
        self._Thread__started = True
        rpdb2.threading._active_limbo_lock.acquire()
        rpdb2.threading._active[rpdb2.threading._get_ident()] = self
        rpdb2.threading._active_limbo_lock.release()

    def _set_daemon(self):
        return True

    def join(self, timeout=None):
        assert False, "cannot join a dummy thread"


# replace things in thread and threading modules
# note that threading tries to keep a reference to original thread.get_ident()
orig_get_ident = rpdb2.thread.get_ident
rpdb2.thread.get_ident = get_ident
rpdb2.threading._get_ident = get_ident
rpdb2.threading._DummyThread = wind_DummyThread

