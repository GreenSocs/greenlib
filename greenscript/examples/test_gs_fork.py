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


def counter(name,last):
    for num in range(1,last+1):
        print gs.time(), name+"\t", num
        if num <> last: gs.wait(1)
    print gs.time(), name+"\t", "finished"


def fork_example():
    print "fork_example:", gs.time(), "     start"
    gs.fork( [lambda: counter("a",9),
              lambda: counter("bb",5),
              lambda: counter("ccc",7),
              lambda: counter("dddd",15),
              ],
             wait_for = 2,
             kill = True,
             debug = False)
    print "fork_example:", gs.time(), "     finish"

def fork_example_args():
    gs.wait(100)
    print "fork_example_args:", gs.time(), "     start"
    gs.fork([counter]*4,
      args=[("a",9),("bb",5),(),()],
      keyargs=[{},{},dict(name="ccc",last=7),dict(name="dddd",last=15)])
    print "fork_example_args:", gs.time(), "     finish"


gs.spawn(fork_example)
gs.spawn(fork_example_args)

gs.start()
