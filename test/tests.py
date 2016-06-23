import unittest
import socket
import subprocess
from time import sleep

conn = None

# {{{ CPU PARSE OPCODE

def parse_cpu_code(code):
    return ()

# }}}

#-----------------------------------------------------------------------------------------------

# {{{ CONNECTION

class Connection:

    def connect(self):
        self.process = subprocess.Popen(["./lilliput", "-D", "-q"])
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        while True:
            try:
                self.socket.connect(("localhost", 5999))
                sleep(0.2)
            except ConnectionRefusedError:
                continue
            break
        self.socket.recv(1024)  # welcome message

    def send(self, *data):
        for d in data:
            self.socket.sendall(bytes(d + '\n', 'latin1'))
            self.socket.recv(100)  # ignore prompt

    def get_i(self, data):
        self.socket.sendall(bytes(data + '\n', 'latin1'))
        r = self.socket.recv(1024).decode('latin1')
        if r[-2:] == '\n+':
            r = r[:-2]
        else:
            self.socket.recv(2)
        return int(r, 16)

    def exec(self, code):
        b = parse_cpu_code(code)
        i = 0
        for bt in b:
            self.send('m w ' + str(i) + ' ' + str(bt))
        self.send('c s')

    def disconnect(self):
        self.socket.sendall(bytes('q\n', 'latin1'))
        self.socket.recv(100)
        self.process.wait(10)

# }}}

#-----------------------------------------------------------------------------------------------

class MemoryTest(unittest.TestCase):

    def testByte(self):
        conn.send('reset', 'm w 0 0xAF')
        self.assertEqual(conn.get_i('m r 0'), 0xAF)

    def testMultibyte(self):
        conn.send('reset', 'm w32 0x0 0x12345678')
        self.assertEqual(conn.get_i('m r 0x0'), 0x78)
        self.assertEqual(conn.get_i('m r 0x1'), 0x56)
        self.assertEqual(conn.get_i('m r 0x2'), 0x34)
        self.assertEqual(conn.get_i('m r 0x3'), 0x12)

    def testOffset(self):
        conn.send('reset', 'm offset 0x1234')
        self.assertEqual(conn.get_i('m offset'), 0x1234)

#-----------------------------------------------------------------------------------------------

class CPUTest(unittest.TestCase):

    def testRegisters(self):
        conn.send('reset', 'c r b 0xAF')
        self.assertEqual(conn.get_i('c r a'), 0x00)
        self.assertEqual(conn.get_i('c r b'), 0xAF)

    def testFlags(self):
        conn.send('reset', 'c f z 1');
        self.assertEqual(conn.get_i('c f y'), 0)
        self.assertEqual(conn.get_i('c f z'), 1)
        self.assertEqual(conn.get_i('c r fl'), 0b100)

    def testMOV(self):
        conn.send('reset', 'c r b 0x42')
        conn.exec('mov a, b')
        self.assertEqual(conn.get_i('c r a'), 0x42)
        self.assertEqual(conn.get_i('c r pc'), 3)

#-----------------------------------------------------------------------------------------------

if __name__ == '__main__':
    conn = Connection()
    conn.connect()
    unittest.TextTestRunner(verbosity=2).run(unittest.TestLoader().loadTestsFromTestCase(MemoryTest))
    unittest.TextTestRunner(verbosity=2).run(unittest.TestLoader().loadTestsFromTestCase(CPUTest))
    conn.disconnect()
