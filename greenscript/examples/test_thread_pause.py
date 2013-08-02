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


def producer():
    print "producer:", gs.time(), "start"
    counter = 0
    while True:
        print "producer:", gs.time(), "counter", counter
        counter += 1
        gs.wait(1)


def controler(thread):
    print "controler:", gs.time(), "start"

    print "controler:", gs.time(), "waiting 5 ns to pause thread", thread.name
    gs.wait(5, gs.NS)
    print "controler:", gs.time(), "pausing thread", thread.name, "NOW!"
    thread.pause()
    print "controler:", gs.time(), "waiting 3 ns to resume thread", thread.name
    gs.wait(3)
    print "controler:", gs.time(), "resuming thread", thread.name, "NOW!"
    thread.resume()

    print "controler:", gs.time(), "waiting 5 ns to pause thread", thread.name
    gs.wait(5, gs.NS)
    print "controler:", gs.time(), "pausing thread", thread.name, "NOW!"
    thread.pause()
    print "controler:", gs.time(), "waiting 3 ns to resume thread", thread.name
    gs.wait(3)
    print "controler:", gs.time(), "resuming thread", thread.name, "NOW!"
    thread.resume()

    print "controler:", gs.time(), "waiting 10 ns to kill thread", thread.name
    gs.wait(10)
    print "controler:", gs.time(), "killing thread", thread.name, "NOW!"
    thread.kill()


prod = gs.spawn(producer)
gs.spawn(lambda:controler(prod))

gs.start()
