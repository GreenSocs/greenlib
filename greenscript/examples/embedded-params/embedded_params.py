# LICENSETEXT
#
#   Copyright (C) 2007 : GreenSocs Ltd
#       http://www.greensocs.com/ , email: info@greensocs.com
#
#   Developed by :
#
#   James Aldis
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

from gs import *


def printall():
  g = gsp_sc_param_list(".*")
  for gi in range(g.length()):
    n = g.read(gi)
    print n.ljust(24), " => ", read_param(n)


def test_db_access(prefix = ""):
  if prefix:  prefix = prefix + "."

  print "Testing DB Access"

  print "=> making parameters <="
  write_param(prefix + "james",10)
  write_param(prefix + "jamesh.bad.dog",False)
  write_param(prefix + "james2",10)
  write_param(prefix + "jamesh2.bad.dog",False)
  write_param(prefix + "a_boolean",False)
  printall()
  print

  print "=> reading parameters <="
  print read_param(prefix + "james")
  print read_param(prefix + "jamesh.bad.dog")
  try: read_param(prefix + "jamesh.bad.dig")
  except GSParamNonExistent:  print "OK failed read"
  print

  print "=> making array parameters and reading as a list <="
  write_param(prefix + "jon.0","zero")
  write_param(prefix + "jon.1","one")
  write_param(prefix + "jon.2","two")
  print read_param_as_list(prefix + "jon")
  print

  print "=> making array parameters from a list <="
  write_param_from_list(prefix + "kate.fish.names",["helga","holga","hubert","humphrey"])
  write_param_from_list(prefix + "kate.fish.names2",["helga","holga","hubert","humphrey"])
  write_param_from_list(prefix + "kate.fish.names3",["helga","holga","hubert","humphrey"])
  print read_param_as_list(prefix + "kate.fish.names")
  print read_param_as_list(prefix + "kate.fish")
  print

  print "=> making array parameters from a dict <="
  write_param_from_dict(prefix + "mixup", dict(
    larry = 10,
    curly = [3,4,5],
    curly2 = [3,4,5],
    moe = ["trunk", "ears", dict(
      arabic = dict(a=10, b=20),
      english = "ugly",
      french = ["odd", "really"],
    )],
    jimmy = [[1,2,3], dict(a=10, b= 45)],
    bob = dict(quality = "terrible"),
  ))
  print

  print "=> reading array parameters as a dict <="
  print read_param_as_dict(prefix + "kate")
  print

  print read_param_as_dict(prefix + "mixup")
  print

  print read_param_as_dict(prefix + "mixup.moe.2")
  print read_param_as_dict(prefix + "mixup.jimmy")
  print read_param_as_dict(prefix + "mixup.bob")
  print

  printall()
  print


def test_params():

  print "Testing PARAM class"
  printall()
  print

  print "=> leaf parameters <="
  a = param("james")
  print a()
  b = param("jamesh.bad.dog", force_string=True)
  print b(), b()+"_with_addendum"
  b2 = param("a_boolean")
  try:
    print b2(), bool(b2())
    print b2()+"_with_addendum"
  except TypeError:
    print "OK failed to append string to bool"
  print

  print "=> ordered array parameters <="
  c = param("jon")
  print "uninitialised", c
  print "length", len(c)
  print "now initialised", c
  for v in c:  print "uninitialised", v
  for v in c:  print "value", v()
  for v in c:  print "now initialised", v
  print c

  d = param("kate.fish.names")
  for v in d:  print v()
  d[2].write("hubba")
  print d

  e = param("mixup.curly2")
  if 4 in e:  print "found 4"
  else:  print "not found 4"
  if "quisling" in e:  print "found quisling"
  else:  print "not found quisling"
  print e
  e.resize(10)
  print e
  e.write([23, 32, 45, 54, 65, 56, 76, 67])
  print e
  try:
    for x in e.iterkeys():  print x
  except BadParamAccess:
    print "OK failed to iterate over keys"
  print

  print "=> ordered array parameters that do not exist <="
  e1 = param("does_not_exist")
  print "missing array has length:", len(e1)
  print e1
  for x in e1:  print "Bad, we found an element"
  print

  print "=> array parameters with names <="
  f = param("mixup")
  print "uninitialised", f
  for k in f.iterkeys():  print k, "=", f[k]
  print f["jimmy"]
  print "now initialised", f
  print f["moe"]
  print f["curly"]
  print f["larry"]
  print f["larry"]()
  print f["larry"]
  print f
  f["curly"] = "flutter"
  f["moe"] = range(20)
  for k in f.iterkeys():  print k, "=", f[k]
  print f

  print len(f["jimmy"])
  for x in f["jimmy"]:  print x
  for k in f.iteritems():  print k
  try:
    for v in f:  print v
  except BadParamAccess:
    print "OK failed to iterate over values"

  g = param("woggle")
  g.write(dict(
    a = 10,
    b = 20,
    c = [2,3,4],
    d = dict(
      a = "hello",
      b = "goodbye",
      c = dict(
        a = [1,2,3],
        b = "enough",
      ),
    ),
  ))
  print g
  print

  print "=> default values: leaves <="
  h = param("mixup.moe.2.french.1", 707)
  print h()
  i = param("nonexistent", 707)
  print i()
  print

  print "=> default values: ordered arrays <="
  j = param("kate.fish.names2", ['u','v','w','x','y','z'])
  for jj in j:  print jj()
  for jj in range(6):  print j[jj]()

  k = param("kate.fish.names3", size=12)
  print k
  for kk in k:  print ">", kk()
  print

  print "=> default values: arrays with names <="
  l = param("jamesh2", dict(bad=dict(dog=27, cat=45)))
  print l, l["bad"]
  print l["bad"]["dog"](), l["bad"]["cat"]()
  print


# always do the database stuff during elaboration
if len(interpreter_name()) == 0:
  print "STANDALONE"
  test_db_access("")
else:
  print "EMBEDDED: adding params for GS"
  test_db_access("gs_module")
  print "EMBEDDED: adding params for SC"
  test_db_access("sc_module")

# and play with parameters during the simulation
spawn(test_params)

# run the sim if needed
if len(interpreter_name()) == 0:  start()

