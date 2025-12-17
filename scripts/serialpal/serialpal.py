import cmd
import serial
import serial.tools.list_ports

import serialpal_utils

port_settings = dict(
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=3,
)


class SerialPal(cmd.Cmd):
    prompt = "(serialpal) "
    intro = "Welcome to SerialPal"
    port = serial.Serial(None, **port_settings)
    port_list = []

    def cleanup():
        SerialPal.port.close()

    def do_exit(self, _):
        SerialPal.cleanup()
        print("bye!")
        return True

    def do_ports(self, _):
        "List available ports"

        SerialPal.port_list = serial.tools.list_ports.comports()
        if len(SerialPal.port_list) == 0:
            print("no ports found")
            return

        for i, port in enumerate(SerialPal.port_list):
            print(f"{i}: {port.device}")
            print(f"\t{port.manufacturer}")
            print(f"\t{port.location}")

    def do_conn(self, arg):
        """Connect to a specific port by index (must call `ports` first)

        Alternatively, provide the full port name (e.g., COMx)
        """

        if arg is None:
            return

        input = str(arg).strip()
        if input == "":
            return

        if str(input).isdigit():
            if len(SerialPal.port_list) == 0:
                print("no scanned ports, run 'ports' first")
                return

            idx = int(input)
            SerialPal.port.port = SerialPal.port_list[idx].device
        else:
            SerialPal.port.port = input

        try:
            SerialPal.port.open()
        except serial.SerialException as e:
            print("failed to open port: {}", e)

        print(f"{SerialPal.port.port} open!")

    def do_close(self, _):
        "Close currently active port"

        SerialPal.port.close()
        print(f"{SerialPal.port.port} closed!")

    def do_recv(self, arg):
        if not SerialPal.port.is_open:
            print("no open ports")
            return

        recv = SerialPal.port.read_until()

        text = serialpal_utils.decode_message(recv)
        print("Response: " + text)

    def do_send(self, arg):
        if not SerialPal.port.is_open:
            print("no open ports")
            return

        cmd: str
        try:
            cmd = (str(arg) + "\n").encode("ascii")
        except UnicodeEncodeError:
            print("command can't be ASCII-encoded")
            return

        n = SerialPal.port.write(cmd)
        status = "OK" if n == len(cmd) else "ERR"
        print(f"{status} ({n})")

        self.do_recv(None)
