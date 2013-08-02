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


class sender:

    def name(self):
        return "sender"

    def doit(self):
        msg = gs.message(
          sender = "sender",
          configset = gs.message(
              cache = gs.message(size = 10),
              CPU = gs.message("adder", "multiplexor"),
          ),
          reply = self.msgfifo.name(),
        )

        # Send to receiver
        try:
            print self.name(), ": sending a message to receiver: start"
            receiver = gs.findReceiver("receiver")
            receiver.write(msg)
        except BadReceiverName, e:
            print self.name(), ": invalid receiver:", e.message
            return

        # Read reply
        print self.name(), ": waiting for reply"
        replyMsg = self.msgfifo.read()
        print self.name(), ": read reply. Field `foo' is", replyMsg["foo"]

    def __init__(self):
        self.msgfifo = gs.msgfifo()
        gs.spawn(self.doit)
