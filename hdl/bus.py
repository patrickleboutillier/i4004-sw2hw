import sys
import hdl
import hdl.uart as uart


class sensor:
    def __init__(self, *signals):
        for signal in signals:
            self.addSignal(signal)

    def addSignal(self, signal):
        bus = signal
        if type(signal) is hdl.pwire:
            bus = signal._bus
        bus.connect(self, signal)


class pbus:
    def __init__(self, n=4, v=0):
        self._n = n
        self._v = v
        self._signals = []
        self._wires = {}

    def len(self):
        return self._n

    def v(self, v):
        if v != self._v:
            changed = v ^ self._v
            self._v = v
            for (sensor, filter) in self._signals:
                if filter is None or filter & changed:    # The sensor is impacted by the change
                    sensor.always()

    def pwire(self, bit):
        if bit not in self._wires:
            self._wires[bit] = hdl.pwire(None, self, bit)
        return self._wires[bit]

    def connect(self, sensor, signal):
        filter = None
        if type(signal) is hdl.pwire:
            filter = 1 << signal._bit
        self._signals.append((sensor, filter))


class bus:
    def __init__(self, n=4, v=0, id=None):
        self.id = id
        self._v = v

    def len(self):
        return self.n

    @property
    def v(self):
        if self.id is not None and uart.port is not None:
            return uart.busRead(self.id)
        else:
            return self._v

    @v.setter
    def v(self, v):
        if self.id is not None and uart.port is not None:
            if v is None:
                uart.busZ(self.id)
            else:
                uart.busWrite(self.id, v)
        else:
            self._v = v