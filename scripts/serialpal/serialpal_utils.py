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


def decode_message(msg: bytes, should_print=False) -> str:
    """ Expected format:

        id/type  | payload_length  | payload            | \\n

        (uint8_t)| (uint8_t)       | (length*uint16_t)  | (char)
    """
    ID_TEXT = 0
    ID_U16 = 1

    if should_print:
        print(msg)

    if len(msg) <= 2:
        return ''

    id, length = decode_header(msg[:2])

    if id == ID_TEXT:
        return decode_text(msg, length)

    if id == ID_U16:
        return str(decode_data(msg, length))
