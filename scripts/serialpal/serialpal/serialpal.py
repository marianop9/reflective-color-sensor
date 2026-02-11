import serial
import serial.tools.list_ports
from typing import Literal
import struct

# from . import decoders


class Response:
    DATASIZE_BITS_8 = 8
    DATASIZE_BITS_16 = 16
    DATASIZE_BITS_DEFAULT = DATASIZE_BITS_8

    def __init__(self, type: Literal["TEXT", "DATA"],
                 raw_payload: bytes,
                 #  data: str | tuple[int],
                 len_bytes):
        self.type = type
        self.raw_payload = raw_payload
        self.payload_len_bytes = len_bytes
        self.payload = None

    def is_error(self):
        if (self.payload is None):
            ok = self.decode()
            if not ok:
                return True

        return self.type == "TEXT" and self.payload.startswith("ERR")

    def decode(self, *, data_size=DATASIZE_BITS_DEFAULT) -> bool:
        if self.payload is not None:
            return True

        if self.type == "TEXT":
            if data_size != Response.DATASIZE_BITS_DEFAULT:
                print("Warning: specified data_size, but response type is TEXT")

            self.payload = self.raw_payload.decode("ascii")
        elif self.type == "DATA":
            # default data_size is 16

            if data_size == Response.DATASIZE_BITS_8:
                format_specifier = "B"
            elif data_size == Response.DATASIZE_BITS_16:
                format_specifier = "H"
            else:
                print('unsupported data size: ', data_size)
                return False

            payload_len = int(self.payload_len_bytes*8/data_size)
            self.payload = struct.unpack("<{}{}".format(payload_len,
                                                        format_specifier),
                                         self.raw_payload)

        return True

    def format(self):
        if self.payload is None:
            return ""

        return "{} ({}): {}".format(self.type,
                                    self.payload_len_bytes,
                                    self.payload)


_port_settings = dict(
    baudrate=9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=5,
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

    def connect(self, port: str) -> bool:
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
            print("failed to open port: ", e)
            return False

        print(f"{self.serial_port.port} open!")
        return True

    def receive(self, data_size=0) -> Response:
        if not self.serial_port.is_open:
            print("no open ports")
            return ""

        recv = self.serial_port.read_until(b";", 12)
        header = recv.decode("ascii")

        if not header.startswith("&"):
            return f"unknwon resp (no startchar): {header}"

        parts = header[1:-1].split(",")
        if len(parts) != 2:
            return f"unknwon resp (invalid header): {header}"

        resp_type = parts[0]
        payload_len_bytes = int(parts[1])

        if resp_type != "TEXT" and resp_type != "DATA":
            return f"unknown resp type: {header}"

        raw_payload = self.serial_port.read(payload_len_bytes)
        if len(raw_payload) == 0:
            return f"received no payload for: {header}"

        if len(raw_payload) != payload_len_bytes:
            return f"incompl. payload ({len(raw_payload)}/{payload_len_bytes})"

        resp = Response(resp_type, raw_payload, payload_len_bytes)

        return resp

    def send(self, cmd: str, arg1="", arg2=""):
        if not self.serial_port.is_open:
            print("no open ports")
            return

        cmd_fmt = "&{},{},{}*"
        out: bytes
        try:
            out = cmd_fmt.format(cmd, arg1, arg2).encode("ascii")
            print(out)
        except UnicodeEncodeError:
            print("command can't be ASCII-encoded")
            return

        n = self.serial_port.write(out)
        status = "OK" if n == len(out) else "ERR"
        print(f"{status} ({n})")

# def build_response(msg: bytes, should_print=False) -> Response:
#     """ Expected format:

#         id/type  | payload_length  | payload            | \\n

#         (uint8_t)| (uint8_t)       | (length*uint16_t)  | (char)
#     """
#     ID_TEXT = 0
#     ID_U16 = 1

#     if should_print:
#         print(msg)

#     if len(msg) <= 2:
#         return ''

#     id, length = decoders._decode_header(msg[:2])

#     if id == ID_TEXT:
#         return Response(type='text', data=decoders.decode_text(msg, length))

#     if id == ID_U16:
#         return Response(type='data', data=decoders.decode_data(msg, length))
