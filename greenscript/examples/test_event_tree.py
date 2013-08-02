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
import sys


e0 = gs.event()
e1 = gs.event()
e2 = gs.event()
e3 = gs.event()
e4 = gs.event()


def producer():
    print "producer:", gs.time(), "start"
    print "producer:", gs.time(), "notify e0 after 1ns"
    e0.notify(1)
    print "producer:", gs.time(), "notify e1 after 8ns"
    e1.notify(8)
    print "producer:", gs.time(), "notify e2 after 5ns"
    e2.notify(5)
    print "producer:", gs.time(), "notify e3 after 3ns"
    e3.notify(3)
    print "producer:", gs.time(), "notify e4 after 10ns"
    e4.notify(10)
    print "producer:", gs.time(), "end"


def consumer():

    tests = [ \
"(e1 & e2)",
"((e1 | e2) & e3)",
"(e1 & (e2 | e3))",
"(e0 | e1 & e2 & e3 | e4)",
"((e1 | e2) | (e3 & e4))",
]

    try:  test = tests[int(sys.argv[1])]
    except:  test = tests[1]

    print "consumer", gs.time(), "waiting for", test
    exec "gs.wait" + test
    print "consumer", gs.time(), "end"


gs.spawn(producer)
gs.spawn(consumer)

gs.start()
