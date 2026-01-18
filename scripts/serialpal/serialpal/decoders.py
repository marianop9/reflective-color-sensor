import struct
from typing import Literal

"""
/** Expected formats:
 *
 * COMMAND
 * (1) ACP_START_CHAR           (&)
 * (2) CMD_STR                  req. max 10 chars
 * (3) ACP_SEPARATOR_CHAR       (,)
 * (4) PAYLOAD_1                opt. max 10 chars
 * (5) ACP_SEPARATOR_CHAR       (,)
 * (6) PAYLOAD_2                opt. max 10 chars
 * (7) ACP_END_CHAR             (*)
 * (8) CHECKSUM                 opt. 8-bit checksum --- IGNORE!
 *
 * RESPONSE
 * Actually not ASCII-encoded, but easier to work with.
 * (1) ACP_START_CHAR           (&)
 * (2) RESP_STR                 req. ASCII text (TEXT, DATA)
 * (3) ACP_SEPARATOR_CHAR       (,)
 * (4) PAYLOAD_LENGTH_BYTES     req. ASCII nums (length specified in BYTES)
 * (5) ACP_PAYLOAD_START_CHAR   (;)
 * (6) PAYLOAD                  req. raw data. length is PAYLOAD_LENGTH bytes.
 * (7) ACP_END_CHAR             (*)
 */
"""

ACP_START_CHAR = b"&"
ACP_END_CHAR = b"*"
ACP_PAYLOAD_START_CHAR = b";"

type ACPRespType = Literal["TEXT", "DATA"]


class ResponseMessage():
    def __init__(self, type: ACPRespType,
                 payload_len_bytes: int,
                 payload: str | tuple[int]):
        self.type: ACPRespType = type
        self.payload_len_bytes = payload_len_bytes
        self.payload = payload

    @classmethod
    def from_bytes(cls, raw: bytes):
        if not cls.is_valid_message(raw):
            raise ValueError("invalid message")

        resp_type, payload_len_bytes, raw_payload = _get_response_parts(raw)
        print("Got: {} ({}). Payload len is ({})".format(resp_type, payload_len_bytes, len(raw_payload)))

        payload = _decode_payload(resp_type, raw_payload, payload_len_bytes)

        return cls(resp_type, payload_len_bytes, payload)

    @staticmethod
    def is_valid_message(raw: bytes):
        return raw.startswith(ACP_START_CHAR) and raw.endswith(ACP_END_CHAR)

    def is_error(self):
        return self.type == "TEXT" and self.payload.startswith("ERR")

    def print(self):
        print("Response: {} ({})".format(self.type, self.payload_len_bytes))
        print("\t"+str(self.payload))


def _get_response_parts(raw: bytes) -> tuple[ACPRespType, int, bytes]:
    # skip ACP_START_CHAR, ACP_END_CHAR
    (raw_header,
     raw_payload_len_bytes,
     raw_payload) = raw[1:len(raw)-1].split(b",", maxsplit=2)

    header = raw_header.decode("ascii")
    payload_len_bytes = int(raw_payload_len_bytes)

    return header, payload_len_bytes, raw_payload


def _decode_header(raw: bytes) -> tuple[ACPRespType, int]:
    # skip ACP_START_CHAR
    (raw_header,
     raw_payload_len_bytes) = raw[1:len(raw)-1].split(b",", maxsplit=1)

    header = raw_header.decode("ascii")
    payload_len_bytes = int(raw_payload_len_bytes)

    return header, payload_len_bytes


def _decode_payload(resp_type: ACPRespType,
                    payload: bytes,
                    len_bytes: int) -> tuple[int] | str:
    if resp_type == "TEXT":
        return payload.decode('ascii')

    if resp_type == "DATA":
        # unpack* always returns a tupple!
        return struct.unpack('<{}H'.format(int(len_bytes/2)), payload)


def decode_text(msg: bytes, payload_len: int) -> str:
    text: bytes
    # unpack* always returns a tupple!
    text, = struct.unpack_from('<{}s'.format(payload_len), msg, offset=2)
    return text.decode('ascii')


def decode_data(msg: bytes, payload_len: int) -> tuple[int]:
    # unpack* always returns a tupple!
    data = struct.unpack_from('<{}H'.format(payload_len), msg, offset=2)

    return data
