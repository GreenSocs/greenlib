
# dumb GreenScript for testing debugger integration

use_wind = False
to_debug = [False, False, False]


import gs
import sys

mysig = gs.signal(debug=False,name="MySig")

def f(wt,nm):
  it = 0
  while True:
    print nm, "reached iteration", it
    gs.wait(wt,gs.NS)
    it = it + 1
    mysig.write(it)

def g():
  it = 0
  while True:
    print "faster", "reached iteration", it
    gs.wait(33,gs.NS)
    it = it + 1
    print "loop starts"
    for i in range(40):
      print 'p',
      for j in range(1000): pass
    print
    print "signal is now", mysig.read()


if use_wind:  gs.use_winpdb()

gs.spawn(lambda : f(100, "slow"), debug = to_debug[0])
gs.spawn(lambda : f(55, "fast"), "fast", to_debug[1], time_unit = gs.MS)
gs.spawn(g, "faster", to_debug[2], time_unit = gs.MS)

def hello():
  print "hello there at end of elab"
  print "and start of sim"
gs.end_of_elaboration(hello)
gs.start_of_simulation(hello)

def goodbye():
  print "goodbye at end of sim"
gs.end_of_simulation(goodbye)

print "processes declared"

if gs.__standalone__:
  gs.start(1,gs.US)
  gs.stop()

