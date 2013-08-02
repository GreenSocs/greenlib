
import gs

class adder:

    def doit(self):
        k = False
        while True:
            gs.wait(self.in1.write_event() | self.in2.write_event())
            self.out.write(self.in1.read() + self.in2.read())
            print "adder out", self.out.read(), an_int
            gs.wait(10,gs.NS)
            print "->", self.h(), self.m()
            if not k:  k = gs.param("test")
            print "-->", k()

    def __init__(self, in1, in2, out):
        self.in1 = gs.sc_signal(in1)
        self.in2 = gs.sc_signal(in2)
        self.out = gs.sc_signal(out)
        gs.spawn(self.doit)

        self.h = gs.param("hello")
        self.m = gs.param("message")

my_module = adder('A','B','C')

an_int = 100

print "ADD h =", my_module.h()
print "ADD m =", my_module.m
try:
  print "ADD", gs.__standalone__
except:  pass
