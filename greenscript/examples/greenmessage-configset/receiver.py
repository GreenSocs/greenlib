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


class receiver:

    def name(self):
        return "receiver"

    def doit(self):
        print self.name(), ": waiting for messages"
        msg = self.msgfifo.read()
        print self.name(), ": the sender was", msg["sender"]

        # Get the configset inside the message
        configset = msg["configset"]

        # Did we get a configset inside the message?
        if gs.ismessage(configset):

            # Print all key->values pairs
            print self.name(), ": received a configset. Key-values pairs:"

            def printmsg(m, indent = "", newline = False):
                if newline:  print
                for k,v in m.items():
                    print self.name(), ":    ", indent, k, "=",
                    if gs.ismessage(v):  printmsg(v, indent + "    ", True)
                    else:  print v

            printmsg(configset)

            # Test existence of a specific parameter
            print self.name(), ": testing if cache.size parameter exists: cache.size = ",
            if "cache" in configset and "size" in configset["cache"]:
                print configset["cache"]["size"]
            else:
                print "(not defined)"
        else:
            print self.name(), ": received an empty configset."

        # Reply
        print self.name(), ": replying"
        msg["foo"] = 123.456
        gs.findReceiver(msg["reply"]).write(msg)

    def __init__(self):
        self.msgfifo = gs.msgfifo("receiver")
        gs.spawn(self.doit)
