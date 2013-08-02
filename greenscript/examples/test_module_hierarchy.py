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


class producer:
    
    def doit(self):
        print "producer:", gs.time(), "start"
        print "producer:", gs.time(), "write 3 to output"
        self.output.write(3)
        print "producer:", gs.time(), "read input signal:", self.input.read()
        print "producer:", gs.time(), "wait for reply (write_event)"
        gs.wait(self.input.write_event())
        print "producer:", gs.time(), "read input signal again:", self.input.read()
        print "producer:", gs.time(), "end"

    def __init__(self, input_p=None, output_p=None):
        self.input = input_p
        self.output = output_p
        gs.spawn(self.doit)


class consumer:
    
    def doit(self):
        print "consumer:", gs.time(), "start"
        print "consumer:", gs.time(), "read input signal:", self.input.read()
        print "consumer:", gs.time(), "wait for input (write_event)"
        gs.wait(self.input.write_event())
        print "consumer:", gs.time(), "read input signal again:", self.input.read()
        print "consumer:", gs.time(), "double the input value and write to output"
        self.output.write(self.input.read()*2)
        print "consumer:", gs.time(), "end"

    def __init__(self, input_p=None, output_p=None):
        self.input = input_p
        self.output = output_p
        gs.spawn(self.doit)


# Signals
msg = gs.signal()
reply = gs.signal()

# Module procucer and port binding
p = producer()
p.output = msg
p.input = reply

# Module consumer and positional port binding
c = consumer(msg, reply)


gs.start()
