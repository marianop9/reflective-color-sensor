import sys
import cmd
import time
import json
import numpy as np

from .serialpal import SerialPal, Response
from .json_exporter import Measurement, export_measurements_json

# constants
VREF = 3.3
VDD = 5


class Cli(cmd.Cmd):
    prompt = "(serialpal) "
    intro = "Welcome to SerialPal"

    def __init__(self):
        super().__init__()
        self.serial = SerialPal()
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
        if should_print and result is not None:
            if not result.decode():
                print("failed to decode response")
                return result
            print(f"RECV: {result.format()}")

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
        Rf = self.set_Rf()
        print("Rf = ", Rf)

    def do_set_res(self, arg):
        Rf = self.set_Rf(arg)
        print("Rf = ", Rf)

    def do_adc(self, arg):
        plot = arg == "plot"

        resp = self.sample_adc()
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
            rgb = parse_rgb(args[1:])
        except ValueError:
            print("invalid rgb:", args[1:])
            return

        resp = self.set_led(led, rgb)
        if resp.decode(data_size=8):
            print(resp.format())
        else:
            print("failed to decode response")

    def do_sweep(self, args: str):
        args = args.split(" ")
        if len(args) > 2:
            print("bad args")
            return

        version = 0
        if len(args) == 2:
            version = int(args[1])

        channels = ["B", "G", "R"]
        chan = args[0].upper()

        if chan not in channels:
            print("must specify channel R/G/B")
            return
        chan_idx = channels.index(chan)

        # get initial Rf value
        Rf = self.set_Rf()
        if Rf == 0:
            # failed to get Rf
            return

        measurements = []
        # log-spaced intensities in range [1%-100%]
        intensities = np.logspace(-2, 0, 40) * 255
        # remove duplicates and add '0'
        intensities = np.concatenate(([0], np.unique(intensities.astype(int))))

        for i, intensity in enumerate(intensities):
            # cast numpy int -> python int for json export:
            intensity = int(intensity)
            # build 24-bit RGB
            rgb = intensity << (8*chan_idx)

            should_retry = True
            while should_retry:
                # 1) start - set LED
                resp = self.set_led(0, rgb)
                if resp.is_error():
                    print("encountered an error: ", resp.payload)
                    return
                # 2) wait for LDR
                time.sleep(0.2)
                # 3) collect samples
                adc_resp = self.sample_adc()
                if not adc_resp.decode(data_size=16):
                    print("failed to decode ADC response")
                    return
                # 3.1) get avg
                mean = np.mean(adc_resp.payload)

                # report results
                intensity_str = "{} ({:.0%})".format(intensity, intensity/255)
                print(f"({i}) intensity: {intensity_str} - result: {mean}")

                # 4) check saturation
                should_retry, Rf = self._check_saturation(mean, Rf)
                # 4.1) check for errors when changing Rf
                if Rf == 0:
                    return
                # 5) let the loop handle the retry...

            else:
                # 6) once done, save measurement data
                measurements.append(Measurement.from_response(
                    chan, i, intensity, adc_resp, mean, Rf))

        export_measurements_json(measurements, chan, version)
        print("sweep done")

    def do_measure(self, _):
        LDR_params: dict
        try:
            with open("LDR_params.json") as fp:
                LDR_params = json.load(fp)
        except:
            print("failed to open LDR_params.json")
            return

        # get initial Rf value
        Rf = self.set_Rf()
        if Rf == 0:
            # failed to get Rf
            return
        
        # set Rf to highest val
        # Rf = 0
        # while Rf < 100:
        #     Rf = self.set_Rf("+")
        #     if Rf == 0:
        #         # failed to get Rf
        #         return

        channels = ["R", "G", "B"]
        results = []

        for i, chan in enumerate(channels):
            # 1) get params
            Ka = LDR_params[chan]["Ka"]
            gamma = LDR_params[chan]["gamma"]

            should_retry = True
            while should_retry:
                # 2) start - set LED
                resp = self.set_led(0, 255 << (8*(2-i)))
                if resp.is_error():
                    print("encountered an error: ", resp.payload)
                    return
                # 3) wait for LDR
                time.sleep(0.2)
                # 4) collect samples
                adc_resp = self.sample_adc()
                if not adc_resp.decode(data_size=16):
                    print("failed to decode ADC response")
                    return
                # 4.1) get avg
                adc = np.mean(adc_resp.payload)
                print(f"chan {chan}: adc={adc}")

                # 5) check saturation
                should_retry, Rf = self._check_saturation(adc, Rf)
                # 5.1) check for errors when changing Rf
                if Rf == 0:
                    return
                # 5.2) let the loop handle the retry...
            else:
                
                # 7) calculate color
                K = (VREF - adc * 3.3/4095) / ((VDD-VREF) * Rf*Ka)
                color = int(np.exp(np.log(K)/gamma))
                print(f"chan {chan}: color={color}")
                color = min(255, color)

                results.append(color)

        self.set_led(0, 0)
        print(results)

    def set_led(self, index: int, rgb: int) -> Response:
        self.serial.send("SET_LED", str(index), hex(rgb))
        return self.receive()

    def sample_adc(self) -> Response:
        self.serial.send("ADC")
        return self.receive()

    def set_Rf(self, arg="") -> int:
        if arg not in ["", "+", "-"]:
            print("invalid arg ", arg)
            return 0

        self.serial.send("SET_RES", arg)
        resp = self.receive()
        if resp.decode():
            return resp.payload[0]
        else:
            print("failed to decode Rf response")
            return 0

    def _check_saturation(self, measurement: int, current_Rf: int) -> tuple[bool, int]:
        """Verifica que la medicion no este saturada, y en caso de ser necesario
            (y posible) ajusta Rf.
            Devuelve el valor de Rf actualizado.
        """
        # upperlim = 3900
        upperlim = 3800
        lowerlim = 800
        minRf = 1
        maxRf = 100

        new_Rf = current_Rf
        if measurement > upperlim and current_Rf < maxRf:
            new_Rf = self.set_Rf("+")
        elif measurement < lowerlim and current_Rf > minRf:
            new_Rf = self.set_Rf("-")

        shouldRetry = current_Rf != new_Rf

        return shouldRetry, new_Rf


def parse_rgb(input: list[str]) -> int:
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
