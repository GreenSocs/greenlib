
import gs

class subber:

    def doit(self):
        k = False
        while True:
            gs.wait(self.in1.write_event() | self.in2.write_event())
            print "diff is", self.in1.read() - self.in2.read(), an_int
            print "->", self.h(), self.m()
            gs.wait(30,gs.NS)
            if not k:  k = gs.param("another")
            print "-->", k()

    def __init__(self, in1, in2, out):
        self.in1 = gs.sc_signal(in1)
        self.in2 = gs.sc_signal(in2)
        gs.spawn(self.doit)

        self.h = gs.param("hello")
        self.m = gs.param("message")

my_module = subber('A','B','C')

an_int = 200

print "SUB h =", my_module.h()
print "SUB m =", my_module.m()
try:
  print "SUB", gs.__standalone__
except:  pass


def f():
  while True:
    print "f"
    gs.wait(15,gs.NS)


