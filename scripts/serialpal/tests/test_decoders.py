import unittest
import struct

from serialpal import decoders


class TestDecoders(unittest.TestCase):

    def test_decode_header(self):
        s = b'\x00\x05'
        expected = (0, 5)

        output = decoders.decode_header(s)
        self.assertEqual(expected, output)

    def test_decode_text(self):
        s = b'\x00\x05PING\n'
        expected = 'PING\n'

        output = decoders.decode_text(s, 5)
        self.assertEqual(expected, output)

    def test_decode_data(self):
        expected = (1, 5, 10, 15)

        header = b'\x01\x04'
        payload = struct.pack('<4H', *expected)

        output = decoders.decode_data(header+payload, payload_len=4)
        self.assertEqual(expected, output)


if __name__ == '__main__':
    unittest.main()
