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

# signal with initial value
prod = gs.signal(123)
# signal without initial value, defaults to False
cons = gs.signal()

def producer():
    print "producer:", gs.time(), "start"
    print "producer:", gs.time(), "write 3 to prod"
    prod.write(3)
    print "producer:", gs.time(), "read signal cons default:", cons.read()
    print "producer:", gs.time(), "wait for reply (write_event on cons)"
    gs.wait(cons.write_event())
    print "producer:", gs.time(), "read signal cons again:", cons.read()
    print "producer:", gs.time(), "end"

def consumer():
    print "consumer:", gs.time(), "start"
    print "consumer:", gs.time(), "read signal prod default:", prod.read()
    print "consumer:", gs.time(), "wait for input (write_event on prod)"
    gs.wait(prod.write_event())
    print "consumer:", gs.time(), "read signal prod again:", prod.read()
    print "consumer:", gs.time(), "double the input value and write back"
    cons.write(prod.read()*2)
    print "consumer:", gs.time(), "end"

gs.spawn(producer,"producer")
gs.spawn(consumer,"consumer")

gs.start()
