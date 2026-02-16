import unittest

from serialpal import cli


class TestCli(unittest.TestCase):
    def test_build_rgb_hex(self):
        expected = 255 << 16 | 16 << 8 | 15

        output = cli.parse_rgb(['0xff100f'])
        self.assertEqual(expected, output)

        hexstr = '0x100f0c'
        expected = int(hexstr, 16)
        output = cli.parse_rgb([hexstr])
        self.assertEqual(expected, output)

        with self.assertRaises(ValueError):
            cli.parse_rgb(['abx'])

        with self.assertRaises(ValueError):
            cli.parse_rgb(['0x1000000'])

    def test_build_rgb_base10(self):
        r, g, b = 255, 25, 66
        expected = r << 16 | g << 8 | b

        output = cli.parse_rgb([str(x) for x in (r, g, b)])
        self.assertEqual(expected, output)

        with self.assertRaises(ValueError):
            cli.parse_rgb(['0', '256', '1'])


if __name__ == '__main__':
    unittest.main()
