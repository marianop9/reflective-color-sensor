import sys
import cmd
import numpy as np

from .serialpal import SerialPal
from .plotter import Plotter


class Cli(cmd.Cmd):
    prompt = "(serialpal) "
    intro = "Welcome to SerialPal"

    def __init__(self):
        super().__init__()
        self.serial = SerialPal()
        self.plotter = Plotter()
        self.last_measurements = ()

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

    def receive(self, should_print=False):
        result = self.serial.receive()
        if should_print:
            if not result.decode():
                print("failed to decode response")
                return result
            print(f"RECV: {result.format()}")
            # print(f"RECV: ({result.type}) {result.data}")

        return result

    def do_recv(self, _):
        self.receive(True)

    def do_send(self, arg: str):
        cmd_and_args = arg.split(" ")
        self.serial.send(*cmd_and_args)
        self.receive(True)

    def do_ping(self, _):
        self.serial.send("PING")
        self.receive(True)

    def do_get_res(self, _):
        self.serial.send("SET_RES")
        self.receive(True)

    def do_set_res(self, arg):
        if arg not in ["+", "-"]:
            print("invalid arg ", arg)
            return

        self.serial.send("SET_RES", arg)
        self.receive(True)

    def do_adc(self, arg):
        plot = arg == "plot"

        self.serial.send("ADC")
        resp = self.receive(should_print=False)
        if not resp.decode(data_size=16):
            print("failed to decode response")
            return

        self.last_measurements = np.array(resp.payload)
        if plot:
            self.plotter.add_batch(self.last_measurements)
        else:
            print(f"Data: {self.last_measurements}")

        mean = self.last_measurements.mean()
        print(f"Mean: {mean}")
        print(f"Std: {self.last_measurements.std(mean=mean)}")

    def do_plot_last(self, arg):
        # plot last measurements
        # self.plotter.add_batch(self.last_measurements)
        data = self.plotter.add_test_data(arg)
        for d in data:
            arr = np.array(d)
            mean = arr.mean()
            print(f"Mean: {mean}")
            print(f"Std: {arr.std(mean=mean)}")

    def do_mem(self, _):
        "Get memory stats"
        self.serial.send("MEM")
        self.receive(True)

    def do_set_led(self, arg):
        """
            Set LED

            set_led <led_idx> <24-bit-rgb>
            set_led <led_idx> <R(0-255)> <G(0-255)> <B(0-255)>
        """
        args = arg.split(' ')
        argc = len(args)
        if argc != 2 and argc != 4:
            print('invalid args')
            return

        led: int
        rgb: int
        try:
            led = int(args[0])
            if led < 0:
                raise ValueError
        except ValueError:
            print("invalid led_idx:", args[0])
            return

        try:
            rgb = build_rgb(args[1:])
        except ValueError:
            print("invalid rgb:", args[1:])
            return

        self.serial.send("SET_LED", str(led), hex(rgb))
        self.receive(True)


def build_rgb(input: list[str]) -> int:
    """ Parses user input into the necessary RGB value.

        If int conversion fails or user input is out-of-bounds,
        a `ValueError` is raised.
    """
    rgb: int

    if len(input) == 1:
        # 24-bit-rgb (in hex)
        rgb = int(input[0], 16)
        if rgb < 0 or rgb >= (1 << 24):
            raise ValueError

    if len(input) == 3:
        # individual r-g-b values (in base-10)
        r, g, b = [int(x) for x in input]
        if any([x < 0 or x >= (1 << 8) for x in (r, g, b)]):
            raise ValueError
        rgb = r << 16 | g << 8 | b

    return rgb


def main():
    try:
        cli = Cli()
        if len(sys.argv) > 1:
            if not cli.serial.connect(sys.argv[1]):
                return

        cli.cmdloop()
    except KeyboardInterrupt:
        print("exiting...")
        cli.do_close(None)


if __name__ == "__main__":
    main()
