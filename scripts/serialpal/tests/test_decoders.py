import unittest
import struct

from serialpal import decoders


class TestDecoders(unittest.TestCase):

    def test_get_response_parts(self):
        raw = b"&TEXT,12,Hello World!*"

        resp_type, len_bytes, raw_payload = decoders._get_response_parts(raw)

        self.assertEqual(resp_type, "TEXT")
        self.assertEqual(len_bytes, 12)

        payload = decoders._decode_payload(resp_type, raw_payload, len_bytes)
        self.assertEqual(payload, "Hello World!")

        expected_payload = (1233, 5, 10, 15, 4095, 3343)
        raw = b"&DATA,6," + struct.pack('<6H', *expected_payload) + b"*"

        resp_type, len_bytes, raw_payload = decoders._get_response_parts(raw)

        self.assertEqual(resp_type, "DATA")
        self.assertEqual(len_bytes, 6)

        payload2 = decoders._decode_payload(resp_type, raw_payload, len_bytes)
        self.assertEqual(payload2, expected_payload)

    # def test_decode_header(self):
    #     s = b'\x00\x05'
    #     expected = (0, 5)

    #     output = decoders.decode_header(s)
    #     self.assertEqual(expected, output)

    # def test_decode_text(self):
    #     s = b'\x00\x05PING\n'
    #     expected = 'PING\n'

    #     output = decoders.decode_text(s, 5)
    #     self.assertEqual(expected, output)

    # def test_decode_data(self):
    #     expected = (1, 5, 10, 15)

    #     header = b'\x01\x04'
    #     payload = struct.pack('<4H', *expected)

    #     output = decoders.decode_data(header+payload, payload_len=4)
    #     self.assertEqual(expected, output)


if __name__ == '__main__':
    unittest.main()
