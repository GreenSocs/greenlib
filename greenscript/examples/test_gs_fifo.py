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
import random

print "Start the simulation before defining the system"
gs.start(1,gs.US)
print "Simulation started:", gs.time()


f = gs.fifo(2)

def producer():
    print "producer:", gs.time(), "start"
    f.write(1)
    print "producer:", gs.time(), "wrote 1"
    f.write(2)
    print "producer:", gs.time(), "wrote 2"
    f.write(3)
    print "producer:", gs.time(), "wrote 3"
    f.write(4)
    print "producer:", gs.time(), "wrote 4"
    print "producer:", gs.time(), "end"

    # prod_unlim()


def consumer():
    print "consumer:", gs.time(), "start"
    data = f.read()
    print "consumer:", gs.time(), "read ", data
    data = f.read()
    print "consumer:", gs.time(), "read ", data
    data = f.read()
    print "consumer:", gs.time(), "read ", data
    data = f.read()
    print "consumer:", gs.time(), "read ", data
    print "consumer:", gs.time(), "end"

    # cons_unlim()


gs.spawn(producer, "producer", False)
gs.spawn(consumer, "consumer", False)


unlimited = gs.fifo()

def prod_unlim():
  x = 100
  while x < 1000:
    gs.wait(1,gs.US)
    nr_to_write = x + random.randint(1,50)
    print "P Load now", unlimited.num_available()
    print "Writing to", nr_to_write-1
    while x < nr_to_write:
      unlimited.write(x)
      x += 1

def cons_unlim():
  y = 0
  while True:
    s = False
    if random.random() > 0.02:  (s,x) = unlimited.nb_read()
    if not s:
      print "Read to", y
      gs.wait(unlimited.write_event())
    else:
      y = x




gs.start()
