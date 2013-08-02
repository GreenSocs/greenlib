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

test_debug = False
import gs
if test_debug:  gs.use_winpdb()

import sys

class blocker(gs.writeif_base_blocking):
  prev = dict(fish = -1)
  def write(self, message):
    print "hey I got a message with fish =", message["fish"], blocker.prev["fish"]
    gs.wait(1,gs.NS)
    blocker.prev = message

class easy(gs.writeif_base_always):
  prev = dict(fish = -2)
  def nb_write(self, message):
    print "cool,", message["fish"], easy.prev["fish"]
    easy.prev = message
    return True

class tiresome(gs.writeif_base_nonblocking):
  prev = dict(fish = -3)
  def nb_write(self, message):
    if self.num_free() > 0:
      print "finally,", message["fish"], tiresome.prev["fish"]
      tiresome.prev = message
      self.nf -= 1
      self.nu += 1
      return True
    return False
  def num_free(self):  return self.nf
  def pop(self):
    while True:
      gs.wait(1.0,gs.NS)
      if self.nu > 0:
        self.nu -= 1
        self.nf += 1
        self.m_data_read_event.notify()
        if tiresome.prev["fish"] == 81:
          break
  def __init__(self, name = None):
    gs.writeif_base.__init__(self, name)
    self.nf = 2
    self.nu = 0
    if not name is None:  gs.spawn(self.pop)

# select one of these three lines - done by SystemC now
#no_process = blocker
#no_process = easy
#no_process = tiresome

class bidir:

    def name(self):
        return "py-bidir"

    def receiver(self):
        print self.name(), ": waiting for messages on", self.msg1fifo.name()
        msg = self.msg1fifo.read()
        print self.name(), ": the sender was", msg["sender"]
        print self.name(), ": the command was", msg["command"]
        msg["foo.test"] = 123.456

        print self.name(), ": replying"
        gs.findReceiver(msg["reply"]).write(msg)
        print "receiver done in", self.name(), "in", gs.interpreter_name()
        try: t = gs.findReceiver("bad_name___opfjrafp")
        except gs.BadReceiverName:
          print "findReceiver did not work, as wanted"

    def sender(self):
        gs.wait(10,gs.NS)
        print self.name(), ": sending a message"
        msg = gs.message()
        msg["command"] = "hello"
        msg["value"] = 2.1734
        msg["reply"] = self.msg2fifo.name()
        msg["boolt"] = True
        msg["boolf"] = False
        ss = gs.message(hello = 10, fish = "james")
        msg["sub"] = ss
        remote = gs.findReceiver("sc-bidir.sc-bidir")
        remote.write(msg)
        print self.name(), ": waiting for messages on", self.msg2fifo.name()
        msg1 = self.msg2fifo.read()
        print self.name(), ": got a reply", msg1["command"], ". Field foo.pi =", msg1["foo.pi"]

    def internal(self):
      gs.wait(3,gs.NS)
      tmp = no_process("py_no_process")
      tmp.debug = test_debug
      gs.wait(35,gs.NS)
      m = gs.findReceiver("gs_module.py_no_process")
      # can use "tmp" directly of course #
      for i in range(10):
        ms = gs.message()
        ms["fish"] = i*i
        m.nb_write(ms)
        gs.wait(5,gs.NS)
      gs.stop()

    def rx2(self):
      prev = gs.message(fish = -1)
      nr_rx = 0
      while nr_rx < 20:
        t = self.msg3fifo.read()
        nr_rx += 1
        print nr_rx, "now I read a message with fish =", t["fish"], prev["fish"]
        prev = t

    def __init__(self):
        self.msg1fifo = gs.msgfifo("receiver")
        self.msg2fifo = gs.msgfifo()
        self.msg3fifo = gs.msgfifo("second_rx")
        gs.spawn(self.sender)
        gs.spawn(self.receiver)
        gs.spawn(self.internal)
        gs.spawn(self.rx2)
        gs.end_of_simulation(self.cb, args=("end-of-sim",))
        gs.end_of_elaboration(self.cb, args=("end-of-elab",))
        gs.start_of_simulation(self.cb, args=("start-of-sim",))
        tmp = no_process()
        print tmp.name, tmp.full_name
        tmp1 = no_process()
        print tmp1.name, tmp1.full_name
        tmp2 = no_process()
        print tmp2.name, tmp2.full_name

    def cb(self, which):
      print which, "callback activated for", self.name(), "in", gs.interpreter_name()

def windup():
  import gc
  print "GC", gc.isenabled()
  print "GC", gc.collect()
  return

