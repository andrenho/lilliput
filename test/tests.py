import unittest
import re
import socket
import subprocess
from time import sleep

conn = None

# {{{ CPU PARSE OPCODE

opcodes = (
  # movement
  ( 0x01, 'mov', 'reg', 'reg' ),
  ( 0x02, 'mov', 'reg', 'v8' ),
  ( 0x03, 'mov', 'reg', 'v16' ),
  ( 0x04, 'mov', 'reg', 'v32' ),
  ( 0x05, 'movb', 'reg', 'indreg' ),
  ( 0x06, 'movb', 'reg', 'indv32' ),
  ( 0x07, 'movw', 'reg', 'indreg' ),
  ( 0x08, 'movw', 'reg', 'indv32' ),
  ( 0x09, 'movd', 'reg', 'indreg' ),
  ( 0x0A, 'movd', 'reg', 'indv32' ),

  ( 0x0B, 'movb', 'indreg', 'reg' ),
  ( 0x0C, 'movb', 'indreg', 'v8' ),
  ( 0x0D, 'movb', 'indreg', 'indreg' ),
  ( 0x0E, 'movb', 'indreg', 'indv32' ),
  ( 0x0F, 'movw', 'indreg', 'reg' ),
  ( 0x1A, 'movw', 'indreg', 'v16' ),
  ( 0x1B, 'movw', 'indreg', 'indreg' ),
  ( 0x1C, 'movw', 'indreg', 'indv32' ),
  ( 0x1D, 'movd', 'indreg', 'reg' ),
  ( 0x1E, 'movd', 'indreg', 'v32' ),
  ( 0x1F, 'movd', 'indreg', 'indreg' ),
  ( 0x20, 'movd', 'indreg', 'indv32' ),

  ( 0x21, 'movb', 'indv32', 'reg' ),
  ( 0x22, 'movb', 'indv32', 'v8' ),
  ( 0x23, 'movb', 'indv32', 'indreg' ),
  ( 0x24, 'movb', 'indv32', 'indv32' ),
  ( 0x25, 'movw', 'indv32', 'reg' ),
  ( 0x26, 'movw', 'indv32', 'v16' ),
  ( 0x27, 'movw', 'indv32', 'indreg' ),
  ( 0x28, 'movw', 'indv32', 'indv32' ),
  ( 0x29, 'movd', 'indv32', 'reg' ),
  ( 0x2A, 'movd', 'indv32', 'v32' ),
  ( 0x2B, 'movd', 'indv32', 'indreg' ),
  ( 0x2C, 'movd', 'indv32', 'indv32' ),

  ( 0x8A, 'swap', 'reg', 'reg' ),

  # logic
  ( 0x2D, 'or', 'reg', 'reg' ),
  ( 0x2E, 'or', 'reg', 'v8' ),
  ( 0x2F, 'or', 'reg', 'v16' ),
  ( 0x30, 'or', 'reg', 'v32' ),
  ( 0x31, 'xor', 'reg', 'reg' ),
  ( 0x32, 'xor', 'reg', 'v8' ),
  ( 0x33, 'xor', 'reg', 'v16' ),
  ( 0x34, 'xor', 'reg', 'v32' ),
  ( 0x35, 'and', 'reg', 'reg' ),
  ( 0x36, 'and', 'reg', 'v8' ),
  ( 0x37, 'and', 'reg', 'v16' ),
  ( 0x38, 'and', 'reg', 'v32' ),
  ( 0x39, 'shl', 'reg', 'reg' ),
  ( 0x3A, 'shl', 'reg', 'v8' ),
  ( 0x3D, 'shr', 'reg', 'reg' ),
  ( 0x3E, 'shr', 'reg', 'v8' ),
  ( 0x41, 'not', 'reg' ),

  # arithmetic
  ( 0x42, 'add', 'reg', 'reg' ),
  ( 0x43, 'add', 'reg', 'v8' ),
  ( 0x44, 'add', 'reg', 'v16' ),
  ( 0x45, 'add', 'reg', 'v32' ),
  ( 0x46, 'sub', 'reg', 'reg' ),
  ( 0x47, 'sub', 'reg', 'v8' ),
  ( 0x48, 'sub', 'reg', 'v16' ),
  ( 0x49, 'sub', 'reg', 'v32' ),
  ( 0x4A, 'cmp', 'reg', 'reg' ),
  ( 0x4B, 'cmp', 'reg', 'v8' ),
  ( 0x4C, 'cmp', 'reg', 'v16' ),
  ( 0x4D, 'cmp', 'reg', 'v32' ),
  ( 0x8B, 'cmp', 'reg' ),
  ( 0x4E, 'mul', 'reg', 'reg' ),
  ( 0x4F, 'mul', 'reg', 'v8' ),
  ( 0x50, 'mul', 'reg', 'v16' ),
  ( 0x51, 'mul', 'reg', 'v32' ),
  ( 0x52, 'idiv', 'reg', 'reg' ),
  ( 0x53, 'idiv', 'reg', 'v8' ),
  ( 0x54, 'idiv', 'reg', 'v16' ),
  ( 0x55, 'idiv', 'reg', 'v32' ),
  ( 0x56, 'mod', 'reg', 'reg' ),
  ( 0x57, 'mod', 'reg', 'v8' ),
  ( 0x58, 'mod', 'reg', 'v16' ),
  ( 0x59, 'mod', 'reg', 'v32' ),
  ( 0x5A, 'inc', 'reg' ),
  ( 0x5B, 'dec', 'reg' ),

  # jumps
  ( 0x5C, 'bz', 'reg' ),
  ( 0x5D, 'bz', 'v32' ),
  ( 0x5C, 'beq', 'reg' ),
  ( 0x5D, 'beq', 'v32' ),
  ( 0x5E, 'bnz', 'reg' ),
  ( 0x5F, 'bnz', 'v32' ),
  ( 0x60, 'bneg', 'reg' ),
  ( 0x61, 'bneg', 'v32' ),
  ( 0x62, 'bpos', 'reg' ),
  ( 0x63, 'bpos', 'v32' ),
  ( 0x64, 'bgt', 'reg' ),
  ( 0x65, 'bgt', 'v32' ),
  ( 0x66, 'bgte', 'reg' ),
  ( 0x67, 'bgte', 'v32' ),
  ( 0x68, 'blt', 'reg' ),
  ( 0x69, 'blt', 'v32' ),
  ( 0x6A, 'blte', 'reg' ),
  ( 0x6B, 'blte', 'v32' ),
  ( 0x6C, 'bv', 'reg' ),
  ( 0x6D, 'bv', 'v32' ),
  ( 0x6E, 'bnv', 'reg' ),
  ( 0x6F, 'bnv', 'v32' ),
  ( 0x70, 'jmp', 'reg' ),
  ( 0x71, 'jmp', 'v32' ),
  ( 0x72, 'jsr', 'reg' ),
  ( 0x73, 'jsr', 'v32' ),
  ( 0x74, 'ret' ),
  ( 0x75, 'sys', 'reg' ),
  ( 0x76, 'sys', 'v8' ),
  ( 0x77, 'iret' ),
  ( 0x86, 'sret' ),

  # stack
  ( 0x78, 'pushb', 'reg' ),
  ( 0x79, 'pushb', 'v8' ),
  ( 0x7A, 'pushw', 'reg' ),
  ( 0x7B, 'pushw', 'v16' ),
  ( 0x7C, 'pushd', 'reg' ),
  ( 0x7D, 'pushd', 'v32' ),
  ( 0x7E, 'push.a' ),
  ( 0x7F, 'popb', 'reg' ),
  ( 0x80, 'popw', 'reg' ),
  ( 0x81, 'popd', 'reg' ),
  ( 0x82, 'pop.a' ),
  ( 0x83, 'popx', 'reg' ),
  ( 0x84, 'popx', 'v8' ),
  ( 0x85, 'popx', 'v9' ),

  # other
  ( 0x87, 'nop' ),
  ( 0x88, 'halt' ),
  ( 0x89, 'dbg' ),
)

registers = ('a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'fp', 'sp', 'pc', 'fl')

regex = re.compile('^([a-z\.]+?)(?:\s+([\w\[\]]+?))?(?:,\s*([\w\[\]]+?))?$', re.IGNORECASE)

def parse_cpu_code(code):

    def type(par):
        if par == '':
            return None
        elif par[0] == '[':
            if str.isalpha(par[1]):
                return 'indreg'
            else:
                return 'indv32'
        elif str.isalpha(par[0]):
            return 'reg'
        else:
            v = int(par, 0)
            if v <= 0xFF:
                return 'v8'
            elif v < 0xFFFF:
                return 'v16'
            else:
                return 'v32'

    def value(par, tp):
        if tp == None:
            return ()
        elif tp == 'reg':
            return (registers.index(par),)
        elif tp == 'v8':
            return (int(par, 0),)
        elif tp == 'v16':
            v = int(par, 0)
            return (v & 0xFF, (v >> 8) & 0xFF)
        elif tp == 'v32':
            v = int(par, 0)
            return (v & 0xFF, (v >> 8) & 0xFF, (v >> 16) & 0xFF, (v >> 24) & 0xFF)
        elif tp == 'indreg':
            return (registers.index(re.sub('\[|\]', '', par)),)
        elif tp == 'indv32':
            v = int(re.sub('\[|\]', '', par), 0)
            return (v & 0xFF, (v >> 8) & 0xFF, (v >> 16) & 0xFF, (v >> 24) & 0xFF)

    cmd, par1, par2 = regex.findall(code)[0]

    par1type = type(par1)
    par2type = type(par2)
    par1value = value(par1, par1type)
    par2value = value(par2, par2type)

    if par1type in ('reg', 'indreg') and par2type in ('reg', 'indreg'):
        par1value = (par1value[0] | (par2value[0] << 4),)
        par2value = ()

    opcode_type = tuple([t for t in (cmd, par1type, par2type) if t])
    for opcode in opcodes:
        if opcode_type == opcode[1:]:
            a = [opcode[0],]
            a.extend(par1value)
            a.extend(par2value)
            return a

    raise Exception('Opcode was not found')

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
            i += 1
        self.send('c s')

    def disconnect(self):
        self.socket.sendall(bytes('q\n', 'latin1'))
        self.socket.recv(100)
        self.process.wait(10)

# }}}

#-----------------------------------------------------------------------------------------------

# {{{ MEMORY TESTS

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

# }}}

#-----------------------------------------------------------------------------------------------

# {{{ CPU TESTS

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
        self.assertEqual(conn.get_i('c r pc'), 2)

        conn.send('reset')
        conn.exec('mov a, 0x34')
        self.assertEqual(conn.get_i('c r a'), 0x34)

        conn.send('reset')
        conn.exec('mov a, 0x1234')
        self.assertEqual(conn.get_i('m r 0'), 3)
        self.assertEqual(conn.get_i('c r a'), 0x1234)

        conn.send('reset')
        conn.exec('mov a, 0xFABC1234')
        self.assertEqual(conn.get_i('c r a'), 0xFABC1234)

# }}}

#-----------------------------------------------------------------------------------------------

if __name__ == '__main__':
    conn = Connection()
    conn.connect()
    unittest.TextTestRunner(verbosity=2).run(unittest.TestLoader().loadTestsFromTestCase(MemoryTest))
    unittest.TextTestRunner(verbosity=2).run(unittest.TestLoader().loadTestsFromTestCase(CPUTest))
    conn.disconnect()
    #print(parse_cpu_code('mov a, 0x34'))
