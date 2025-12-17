import unittest
import struct

import serialpal_utils


class TestSerialpalUtils(unittest.TestCase):

    def test_decode_header(self):
        s = b'\x00\x05'
        expected = (0, 5)

        output = serialpal_utils.decode_header(s)
        self.assertEqual(expected, output)

    def test_decode_text(self):
        s = b'\x00\x05PING\n'
        expected = 'PING\n'

        output = serialpal_utils.decode_message(s)
        self.assertEqual(expected, output)

    def test_decode_data(self):
        expected = (1, 5, 10, 15)

        header = b'\x01\x04'
        payload = struct.pack('<4H', *expected)

        output = serialpal_utils.decode_data(header+payload, payload_len=4)
        self.assertEqual(expected, output)


if __name__ == '__main__':
    unittest.main()
