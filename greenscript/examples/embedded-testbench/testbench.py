import gs


gen1 = [10,20,30]
gen2 = [5,6,7]
expect = [15,26,37]

class testbench:
    
    def generate(self):
        # generate input
        for i in range(0,len(expect)):
            self.out1.write(gen1[i])
            self.out2.write(gen2[i])
            gs.wait(0)

    def compare(self):
        # compare expected results
        for i in range(0,len(expect)):
            gs.wait(self.in1.write_event())
            print "expected", expect[i],
            print "read", self.in1.read(),
            if expect[i] == self.in1.read():
                print "(ok)"
            else:
                print "(ERROR!)"

    def __init__(self, out1, out2, in1):
        self.out1 = gs.sc_signal(out1)
        self.out2 = gs.sc_signal(out2)
        self.in1 = gs.sc_signal(in1)
        gs.spawn(self.generate)
        gs.spawn(self.compare)
