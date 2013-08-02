import gs

class adder:
    
    def doit(self):
        while True:
            while self.in1.num_available() < 3 and self.in2.num_available() < 3:
                print "adder: waiting for more numbers at input fifos (in1 has", self.in1.num_available(), "; in2 has", self.in2.num_available(), ")"
                gs.wait(self.in1.write_event() | self.in2.write_event())
            print "adder: writing the 3 last results to output fifo"
            for i in range(0,3):
                self.out.write(self.in1.read() + self.in2.read())

    def __init__(self, in1, in2, out):
        self.in1 = gs.sc_fifo(in1)
        self.in2 = gs.sc_fifo(in2)
        self.out = gs.sc_fifo(out)
        gs.spawn(self.doit)
