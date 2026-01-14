import struct


def decode_header(msg: bytes) -> tuple[int, int]:
    return struct.unpack("<BB", msg)


def decode_text(msg: bytes, payload_len: int) -> str:
    text: bytes
    # unpack* always returns a tupple!
    text, = struct.unpack_from('<{}s'.format(payload_len), msg, offset=2)
    return text.decode('ascii')


def decode_data(msg: bytes, payload_len: int) -> tuple[int]:
    # unpack* always returns a tupple!
    data = struct.unpack_from('<{}H'.format(payload_len), msg, offset=2)

    return data
