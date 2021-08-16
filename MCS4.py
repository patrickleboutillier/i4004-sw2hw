import sys, fileinput
import chips.i4001 as i4001, chips.i4002 as i4002, chips.i4004 as i4004
import chips.clock as clock
from hdl import *


class MCS4:
    def __init__(self):
        self.ram_chip = 0                   # This represents the currently active RAM chip

        self.clock = clock.clock()
 
        self.data = bus()
        self.cm_rom = wire()
        self.cm_ram = bus()
        self.test = wire()
        self.CPU = i4004.i4004(self, self.clock.ph1, self.clock.ph2, self.data, self.cm_rom, self.cm_ram, self.test)

        self.PROM = []
        self.RAM = [None, [], [], None, [], None, None, None, []]
        self.SR = []
      

    def addROM(self, rom):
        self.PROM.append(rom)

    def addRAM(self, bank, ram):
        idx = 1 << bank
        self.RAM[idx].append(ram)

    def addSR(self, sr):
        self.SR.append(sr)

    def setRAMAddrHigh(self, addrh):
        self.ram_chip = addrh >> 2
        self.data.v(addrh)
        self.RAM[self.cm_ram._v][self.ram_chip].setReg()

    def setRAMAddrLow(self, addrl):
        self.data.v(addrl)
        self.RAM[self.cm_ram._v][self.ram_chip].setChar()

    def setRAMAddr(self, addrh, addrl):
        self.setRAMAddrHigh(addrh)
        self.setRAMAddrLow(addrl)

    def getRAM(self):
        self.RAM[self.cm_ram._v][self.ram_chip].enableRAM()
        return self.data._v

    def setRAM(self, byte):
        self.data.v(byte)
        self.RAM[self.cm_ram._v][self.ram_chip].setRAM()

    def getStatus(self, char):
        self.RAM[self.cm_ram._v][self.ram_chip].enableStatus(char)
        return self.data._v

    def setStatus(self, char, byte):
        self.data.v(byte)
        self.RAM[self.cm_ram._v][self.ram_chip].setStatus(char)

    def setOutput(self, nib):
        self.data.v(nib)
        self.RAM[self.cm_ram._v][self.ram_chip].setOutput()

    def program(self):
        fi = fileinput.input()
        if self.PROM[0].program(fi) == 0:
            sys.exit("ERROR: No instructions loaded!") 
        elif len(self.PROM) > 1:
            for p in self.PROM[1:]:
                p.program(fi)
        # TODO: Make sure stdin is empty
        fi.close()

    def run(self, callback=None, dump=False):
        nb = 0
        while (True):
            if callback is not None:
                callback(nb)
            for _ in range(5):
                self.clock.tick(4)
            if dump:
                self.dump(nb)
            self.CPU.execute()
            for _ in range(3):
                self.clock.tick(4)
            nb += 1

    def dump(self, nb):
        self.CPU.dump(nb)
        self.RAM[1][0].dump()
        self.RAM[1][1].dump()
        for r in self.PROM:
            r.dump()
        print()
        for s in self.SR:
            s.dump()
        print()