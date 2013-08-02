import gs

class adder:
    
    def doit(self):
        while True:
            gs.wait(self.in1.write_event() | self.in2.write_event())
            self.out.write(self.in1.read() + self.in2.read())

    def __init__(self, in1, in2, out):
        self.in1 = gs.sc_signal(in1)
        self.in2 = gs.sc_signal(in2)
        self.out = gs.sc_signal(out)
        gs.spawn(self.doit)
