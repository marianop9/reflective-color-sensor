import cmd

from .serialpal import SerialPal


class Cli(cmd.Cmd):
    prompt = "(serialpal) "
    intro = "Welcome to SerialPal"

    def __init__(self):
        super().__init__()
        self.serial = SerialPal()

    def do_exit(self, _):
        self.serial.close()
        print("bye!")
        return True

    def do_ports(self, _):
        "List available ports"

        ports = self.serial.port_list
        if len(ports) == 0:
            print("no ports found")
            return

        for i, port in enumerate(ports):
            print(f"{i}: {port.device}")
            print(f"\t{port.manufacturer}")
            print(f"\t{port.location}")

    def do_conn(self, arg):
        """
        Connect to a specific port by index (must call `ports` first)

        Alternatively, provide the full port name (e.g., COMx)
        """
        self.serial.connect(arg)

    def do_close(self, _):
        "Close currently active port"

        if self.serial.is_serial_port_open():
            self.serial.close()
            print(f"{self.serial.serial_port_name()} closed!")

    def do_recv(self, _):
        result = self.serial.receive()
        print(f"RECV: {result}")

    def do_send(self, arg):
        self.serial.send(arg)
        self.do_recv(None)


def main():
    try:
        cli = Cli()
        cli.cmdloop()
    except KeyboardInterrupt:
        print("exiting...")
        cli.do_close(None)


if __name__ == "__main__":
    main()
