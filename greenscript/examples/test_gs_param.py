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

import gs
from random import random


# The cpu module
class cpu:

    def doit(self):
        for i in range(0,10):
            addr = int(random()*10)
            print "Requesting data at address", addr
            self.req.write(addr)
            print "Answer:", self.ack.read()
        print "Test parameter hierarchy: foo.bar.zoo =",
        print self.testparam()

    def __init__(self, req, ack):
        self.req = req
        self.ack = ack
        gs.spawn(self.doit)
        self.testparam = gs.param("foo.bar.zoo")


# The memory module
class memory:

    def doit(self):
        while True:
            gs.wait(self.in1.write_event())  # avoid pending read() at the end
            addr = self.in1.read()
            self.out.write(self.MEM[addr]())

    def __init__(self, in1, out):
        self.in1 = in1
        self.out = out
        gs.spawn(self.doit)
        # Get memory contents as a parameter
        self.MEM = gs.param("MEM")


# Load config file
gs.config_lua("test_gs_param.cfg.lua")

# Describe the system
addr = gs.fifo()
data = gs.fifo()
cpu1 = cpu(addr,data)
memory1 = memory(addr,data)

# Try an Array Parameter
ap = gs.param("test_array")
print ap
for a in ap: print a()+20

# Test the callbacks
def pp(x): print x
gs.end_of_elaboration(lambda: pp("eoe"))
gs.end_of_simulation(lambda: pp("ssss"))
gs.end_of_simulation(lambda: pp("eos"))

# A simple process to keep the kernel alive a bit
def goon():
  for i in range(4):
    gs.wait(25+25*i,gs.NS)
    print gs.simulation_time()
gs.spawn(goon)

# Start!
gs.start(100,gs.NS)
#gs.start()

