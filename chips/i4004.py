import chips.modules.timing as timing
import chips.modules.addr as addr, chips.modules.inst as inst, chips.modules.scratch as scratch, chips.modules.alu as alu, chips.modules.control as control
from hdl import *


class i4004:
    def __init__(self, clk1, clk2, data, cm_rom, cm_ram, test):
        self.timing = timing.timing(clk1, clk2, None)
        self.sync = self.timing.sync
        self.data = data
        self.inst = inst.inst(self, self.timing, self.data, cm_rom, cm_ram)
        self.alu = alu.alu(self.inst, self.timing, data)
        self.scratch = scratch.scratch(self.inst, self.timing, data)
        self.addr = addr.addr(self.inst, self.timing, self.data, cm_rom)
        self.control = control.control(self.inst)

        self.test = test


    def testZero(self):
        return 1 if self.test.v == 0 else 0


    def DCL(self):
        if self.alu.acc & 0b0111 == 0:
            self.inst.ram_bank = 1
        elif self.alu.acc & 0b0111 == 1:
            self.inst.ram_bank = 2
        elif self.alu.acc & 0b0111 == 2:
            self.inst.ram_bank = 4
        elif self.alu.acc & 0b0111 == 3:
            self.inst.ram_bank = 3
        elif self.alu.acc & 0b0111 == 4:
            self.inst.ram_bank = 8
        elif self.alu.acc & 0b0111 == 5:
            self.inst.ram_bank = 10
        elif self.alu.acc & 0b0111 == 6:
            self.inst.ram_bank = 12
        elif self.alu.acc & 0b0111 == 7:
            self.inst.ram_bank = 14


    def dump(self, cycle):
        print("\nCYCLE #{}".format(cycle))
        self.addr.dump() ; print("  ", end='')
        self.inst.dump() ; print("  ", end='')
        self.scratch.dump() ; print("  ", end='')
        print("TEST:{:b}  ACC/CY:{:04b}/{}".format(self.test.v, self.alu.acc, self.alu.cy))
