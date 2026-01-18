import serial
import serial.tools.list_ports
import sys

from serialpal.serialpal import _port_settings
from serialpal import decoders


def main():
    serial_port = serial.Serial(port=None, **_port_settings)
    serial_port.timeout = 25
    # if not specified, print available ports
    if len(sys.argv) < 2:
        ports = serial.tools.list_ports.comports()
        if len(ports) == 0:
            print("no ports found")
            return

        for i, port in enumerate(ports):
            print(f"{i}: {port.device}")
            print(f"\t{port.manufacturer}")
            print(f"\t{port.location}")

        inp = input("Seleccionar puerto:")
        if inp.isdigit():
            serial_port.port = ports[int(inp)].device
        else:
            serial_port.port = inp

    if len(sys.argv) == 2:
        serial_port.port = sys.argv[1]

    serial_port.open()

    try:
        while (True):
            buffer = serial_port.read_until(b";")
            print(f"Received: {buffer}")
            resp_type, len_bytes = decoders._decode_header(buffer)
            print(f"Got header: {resp_type} ({len_bytes})")

            # blocking until timeout if not enough
            payload = serial_port.read(len_bytes)
            if len(payload) != len_bytes:
                raise TimeoutError(
                    f"short payload: got {len(payload)}/{len_bytes}")
            
            print("got payload:", decoders._decode_payload(resp_type, payload, len_bytes))
    except KeyboardInterrupt:
        print("exiting...")


if __name__ == "__main__":
    main()
