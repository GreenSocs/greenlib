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

class bidir:
  def receiver(self):
    while True:
      gs.wait(21,gs.NS);
      while rx.num_available() == 0:
        gs.wait(rx.write_event())
      print "PY received msg", rx.read()["data"]

  def sender(self):
    dest = gs.findReceiver("sc-bidir.sc-bidir")
    x = 0
    while True:
      msg = gs.message(sender = "python", command = "test", data = x)
      x += 1
      print "          send new msg", x, "to", dest.num_free()
      while not dest.nb_write(msg):
        gs.wait(dest.data_read_event());

  def __init__(self):
    gs.spawn(self.receiver)
    gs.spawn(self.sender)

bd = bidir()
rx = gs.msgfifo("rx", 3)

