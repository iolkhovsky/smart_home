from bluepy.btle import Scanner, DefaultDelegate, Peripheral


class ScanDelegate(DefaultDelegate):
    def __init__(self):
        DefaultDelegate.__init__(self)

    def handleDiscovery(self, dev, isNewDev, isNewData):
        if isNewDev:
            print("Discovered device", dev.addr)
        elif isNewData:
            print("Received new data from", dev.addr)


class SmartLampController:

    LAMP_SERVICE_UUID = "00001101-0000-1000-8000-00805f9b34fb"
    DEVICE_RX_CHARACTERISTIC_UUID = "00001142-0000-1000-8000-00805f9b34fb"

    def __init__(self, device_name="SmartLamp", scan_time=10.):
        assert isinstance(device_name, str)
        self.name = device_name
        self.scanner = Scanner().withDelegate(ScanDelegate())
        print(self, "Start device scanning...")
        devices = self.scanner.scan(scan_time)
        print(self, "Scanning completed.")
        self.target_device = None
        for dev in devices:
            for adtype, desc, value in dev.getScanData():
                if desc == "Complete Local Name" and value == self.name:
                    self.target_device = dev
                    print(self, f"Found target device: {dev.addr}")
                    break
        assert self.target_device is not None
        self.peripheral = Peripheral(self.target_device.addr)
        print(self, f"Peripheral created.")
        services = self.peripheral.getServices()
        self.lamp_service = self.peripheral.getServiceByUUID(self.LAMP_SERVICE_UUID)
        print(self, f"Service is ready.")
        self.tx_characteristic = self.lamp_service.getCharacteristics(self.DEVICE_RX_CHARACTERISTIC_UUID)[0]
        print(self, f"Characteristic is ready.")

    def __del__(self):
        self.peripheral.disconnect()

    def _write(self, msg):
        print(self, f"Send command: {msg}")
        self.tx_characteristic.write(bytes(msg, "utf-8"))

    def switch_lamp(self, state):
        if isinstance(state, bool):
            if state:
                self._write("rgb 255 255 255")
            else:
                self._write("rgb 0 0 0")
        elif len(state) == 3:
            r, g, b = state
            self._write(f"rgb {r} {g} {b}")

    def __str__(self):
        return f"SmartLampController:{self.name}"


if __name__ == "__main__":
    import time
    lamp = SmartLampController()
    state = True
    while True:
        time.sleep(1)
        lamp.switch_lamp(state)
        state = not state
