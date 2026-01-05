import serial
import serial.tools.list_ports

from . import decoders

_port_settings = dict(
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=3,
)


class SerialPal():
    def __init__(self, port_settings=_port_settings):
        self.serial_port = serial.Serial(port=None, **port_settings)
        self._port_list = None

    @property
    def port_list(self):
        "List available ports"
        if self._port_list is None:
            self._port_list = serial.tools.list_ports.comports()

        return self._port_list

    def serial_port_name(self):
        return self.serial_port.port

    def is_serial_port_open(self):
        return self.serial_port.is_open

    def close(self):
        self.serial_port.close()

    def connect(self, port: str):
        """Connect to a specific port by index (must call `ports` first)

        Alternatively, provide the full port name (e.g., COMx)
        """
        port = port.strip()
        if port == "":
            return

        if port.isdigit():
            if len(self._port_list) == 0:
                print("no scanned ports")
                return

            self.serial_port.port = self._port_list[int(port)].device
        else:
            self.serial_port.port = port

        try:
            self.serial_port.open()
        except serial.SerialException as e:
            print("failed to open port: {}", e)
        else:
            print(f"{self.serial_port.port} open!")

    def receive(self) -> str:
        if not self.serial_port.is_open:
            print("no open ports")
            return ""

        recv = self.serial_port.read_until()
        return decoders.decode_message(recv)

    def send(self, arg):
        if not self.serial_port.is_open:
            print("no open ports")
            return

        cmd: str
        try:
            cmd = (str(arg) + "\n").encode("ascii")
        except UnicodeEncodeError:
            print("command can't be ASCII-encoded")
            return

        n = self.serial_port.write(cmd)
        status = "OK" if n == len(cmd) else "ERR"
        print(f"{status} ({n})")
