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
        # test pause and resume
        self.change_parameters()
        self.file_output.pause()
        self.change_parameters()
        self.file_output.resume()
        self.change_parameters()

        # test pause until event fires
        self.e1 = gs.event()
        self.file_output.pause(self.e1)
        self.change_parameters()
        self.e1.notify()
        self.change_parameters()

    def change_parameters(self):
        for i in range(0,3):
            self.addr.write(int(random()*10))
            print "Requesting data at address", self.addr
            self.req.write(self.addr())
            self.data.write(self.ack.read())
            print "Answer:", self.data

    def __init__(self, req, ack):
        self.req = req
        self.ack = ack
        gs.spawn(self.doit)
        self.configure_gav_output()

    def configure_gav_output(self):
        # Parameters
        self.addr = gs.param("addr")
        self.data = gs.param("data")
        # need to initialise a parameter if we want it searcheable by name
        self.addr(0)
        # Configure output to file "test_gav_output.txt"
        print "Output parameter changes to file: test_gav_output.txt"
        self.file_output = gs.av.output(gs.av.TXT_FILE_OUT,"test_gav_output.txt")
        # Add parameters to the output (by name or reference)
        self.file_output.add("addr")
        self.file_output.add(self.data)


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

# Start!
gs.start()
