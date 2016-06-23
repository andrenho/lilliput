import unittest
import socket
import subprocess
from time import sleep


class Connection:

    def connect(self):
        self.process = subprocess.Popen(["./lilliput", "-D"])
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        while True:
            try:
                self.socket.connect(("localhost", 5999))
                sleep(0.2)
            except ConnectionRefusedError:
                continue
            break
        self.socket.recv(1024)  # welcome message

    def send(self, data):
        self.socket.sendall(bytes(data + '\n', 'latin1'))
        self.socket.recv(100)  # ignore prompt

    def get_i(self, data):
        self.socket.sendall(bytes(data + '\n', 'latin1'))
        r = self.socket.recv(1024).decode('latin1')
        if r[-2:] == '\n+':
            r = r[:-2]
        else:
            self.socket.recv(2)
        return int(r, 16)

    def disconnect(self):
        self.socket.sendall(bytes('q\n', 'latin1'))
        self.socket.recv(100)
        self.process.wait(10)

#-----------------------------------------------------------------------------------------------

conn = None

class MemoryTest(unittest.TestCase):

    def testByte(self):
        conn.send('m w 0 0xAF')
        self.assertEqual(conn.get_i('m r 0'), 0xAF)

    def testMultibyte(self):
        conn.send('m w32 0x0 0x12345678')
        self.assertEqual(conn.get_i('m r 0x0'), 0x78)
        self.assertEqual(conn.get_i('m r 0x1'), 0x56)
        self.assertEqual(conn.get_i('m r 0x2'), 0x34)
        self.assertEqual(conn.get_i('m r 0x3'), 0x12)

    def testOffset(self):
        conn.send('m offset 0x1234')
        self.assertEqual(conn.get_i('m offset'), 0x1234)

#-----------------------------------------------------------------------------------------------

if __name__ == '__main__':
    conn = Connection()
    conn.connect()
    unittest.TextTestRunner(verbosity=2).run(unittest.TestLoader().loadTestsFromTestCase(MemoryTest))
    conn.disconnect()
