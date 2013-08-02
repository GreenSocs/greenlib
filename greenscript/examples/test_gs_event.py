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


e1 = gs.event()
e2 = gs.event()
e3 = gs.event()


def test1():

    print "test1:", gs.time(), "start"
    print "test1:", gs.time(), "wait for 10 default units (ns)"
    gs.wait(10)

    print "test1:", gs.time(), "notify e1 in the same delta"
    e1.notify()
    print "test1:", gs.time(), "wait for one delta"
    gs.wait(0)
    print "test1:", gs.time(), "at this delta e1 was already notified"
    print "test1:", gs.time(), "wait for 10 default units (ns)"
    gs.wait(10)

    print "test1:", gs.time(), "notify e2 in the next delta"
    e2.notify(0)
    print "test1:", gs.time(), "wait for one delta"
    gs.wait(0)
    print "test1:", gs.time(), "at this same delta e2 gets notified"
    print "test1:", gs.time(), "wait for 10 default units (ns)"
    gs.wait(10)

    print "test1:", gs.time(), "notify e3 delayed by 5 us"
    e3.notify(5, gs.US)
    print "test1:", gs.time(), "wait for 4 us"
    gs.wait(4, gs.US)
    print "test1:", gs.time(), "at this time e3 wasn't notifyed yet"
    print "test1:", gs.time(), "wait for 10 default units (ns)"
    gs.wait(10)

    print "test1:", gs.time(), "end"


def test2():
    print "test2:", gs.time(), "          start"

    print "test2:", gs.time(), "          waiting for e1"
    gs.wait(e1)
    print "test2:", gs.time(), "          e1 fired"

    print "test2:", gs.time(), "          waiting for e2"
    gs.wait(e2)
    print "test2:", gs.time(), "          e2 fired"

    print "test2:", gs.time(), "          waiting for e3"
    gs.wait(e3)
    print "test2:", gs.time(), "          e3 fired"

    print "test2:", gs.time(), "          end"


gs.spawn(test1,"test1")
gs.spawn(test2,"test2")

gs.start()
