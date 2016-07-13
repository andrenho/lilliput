use device::*;

pub enum Register { A, B, C, D, E, F, G, H, I, J, K, L, FP, SP, PC, FL }
pub enum Flag { Y, V, Z, S, GT, LT, P, T }

pub struct CPU {
    register: [u32; 16],
}

impl CPU {

    pub fn new() -> CPU {
        CPU { 
            register: [0; 16],
        }
    }

    fn flag(&self, f: Flag) -> bool {
        (self.register[Register::FL as usize] >> f as usize) & 1 != 0
    }

    fn set_flag(&mut self, f: Flag, value: bool) {
        let x = if value { 1 } else { 0 };
        self.register[Register::FL as usize] ^= (!x ^ self.register[Register::FL as usize]) & (1 << (f as usize));
    }

    
}

impl Device for CPU {
    fn dev_get(&self, pos: u32) -> u8 { return 0x0; }
    fn dev_set(&mut self, _pos: u32, _data: u8) {}
    fn dev_size(&self) -> u32 { return 0x0; }
}


#[cfg(test)]
mod tests {
    use super::*;
    use computer::*;

    fn add_code(computer: &mut Computer, code: &str) {
        // split string
        let _parts: Vec<&str> = code.split_whitespace().collect();
        let opcode = _parts[0];
        let pars: Vec<String> = _parts[1..].into_iter().map(|s| (*s).replace(",", "")).collect();

        // find byte values
        // {{{ functions to understand the parameters
        fn register_from_name(reg: &str) -> Register {
            match reg {
                "A"  => Register::A,
                "B"  => Register::B,
                "C"  => Register::C,
                "D"  => Register::D,
                "E"  => Register::E,
                "F"  => Register::F,
                "G"  => Register::G,
                "H"  => Register::H,
                "I"  => Register::I,
                "J"  => Register::J,
                "K"  => Register::K,
                "L"  => Register::L,
                "FP" => Register::FP,
                "SP" => Register::SP,
                "PC" => Register::PC,
                "FL" => Register::FL,
                _    => panic!(format!("Invalid register '{}'.", reg))
            }
        }

        enum Par { Reg(u8), IndReg(u8), V8(u8), V16(u16), V32(u32), IndV32(u32) };
        impl PartialEq for Par {
            /* I couldn't find any better way to do that in Rust. This sucks. */
            fn eq(&self, other: &Self) -> bool {
                match (self, other) {
                    (&Par::Reg(_), &Par::Reg(_))       => true,
                    (&Par::IndReg(_), &Par::IndReg(_)) => true,
                    (&Par::V8(_), &Par::V8(_))         => true,
                    (&Par::V16(_), &Par::V16(_))       => true,
                    (&Par::V32(_), &Par::V32(_))       => true,
                    (&Par::IndV32(_), &Par::IndV32(_)) => true,
                    _                                  => false
                }
            }
        }

        fn parse_u32(par: &str) -> u32 {
            if &par[0..2] == "0x" {
                u32::from_str_radix(&par[2..], 16).unwrap()
            } else {
                par.parse::<u32>().unwrap()
            }
        }

        fn get_type(par: &str) -> Par {
            let fst = par.chars().nth(0).unwrap();
            if fst == '[' {
                if par.chars().nth(1).unwrap().is_alphabetic() {
                    Par::IndReg(register_from_name(par) as u8)
                } else {
                    Par::IndV32(parse_u32(par))
                }
            } else if fst.is_alphabetic() {
                Par::Reg(register_from_name(par) as u8)
            } else {
                match parse_u32(par) {
                    v @ 0x0   ...   0xFF =>  Par::V8(v as  u8),
                    v @ 0x100 ... 0xFFFF => Par::V16(v as u16),
                    v @ _                => Par::V32(v as u32)
                }
            }
        }
        // }}}
        let mut par: Vec<Par> = pars.into_iter().map(|p| get_type(&p)).collect();

        // find opcode
        struct Opcode {
            opcode: u8,
            name:   &'static str,
            pars:   Vec<Par>
        }

        // {{{ opcode list
        macro_rules! op {
            ($opcode:expr => $name:expr, $($par:ident),*) => {{ Opcode { opcode: $opcode, name: $name, pars: vec![ $( Par:: $par(0) ),* ] } }};
            ($opcode:expr => $name:expr) => {{ Opcode { opcode: $opcode, name: $name, pars: vec![] } }}
        }

        let opcodes: Vec<Opcode> = vec![
            // movement
            op!(0x01 => "mov",  Reg, Reg    ),
            op!(0x02 => "mov",  Reg, V8     ),
            op!(0x03 => "mov",  Reg, V16    ),
            op!(0x04 => "mov",  Reg, V32    ),
            op!(0x05 => "movb", Reg, IndReg ),
            op!(0x06 => "movb", Reg, IndV32 ),
            op!(0x07 => "movw", Reg, IndReg ),
            op!(0x08 => "movw", Reg, IndV32 ),
            op!(0x09 => "movd", Reg, IndReg ),
            op!(0x0A => "movd", Reg, IndV32 ),

            op!(0x0B => "movb", IndReg, Reg    ),
            op!(0x0C => "movb", IndReg, V8     ),
            op!(0x0D => "movb", IndReg, IndReg ),
            op!(0x0E => "movb", IndReg, IndV32 ),
            op!(0x0F => "movw", IndReg, Reg    ),
            op!(0x1A => "movw", IndReg, V16    ),
            op!(0x1B => "movw", IndReg, IndReg ),
            op!(0x1C => "movw", IndReg, IndV32 ),
            op!(0x1D => "movd", IndReg, Reg    ),
            op!(0x1E => "movd", IndReg, V32    ),
            op!(0x1F => "movd", IndReg, IndReg ),
            op!(0x20 => "movd", IndReg, IndV32 ),

            op!(0x21 => "movb", IndV32, Reg    ),
            op!(0x22 => "movb", IndV32, V8     ),
            op!(0x23 => "movb", IndV32, IndReg ),
            op!(0x24 => "movb", IndV32, IndV32 ),
            op!(0x25 => "movw", IndV32, Reg    ),
            op!(0x26 => "movw", IndV32, V16    ),
            op!(0x27 => "movw", IndV32, IndReg ),
            op!(0x28 => "movw", IndV32, IndV32 ),
            op!(0x29 => "movd", IndV32, Reg    ),
            op!(0x2A => "movd", IndV32, V32    ),
            op!(0x2B => "movd", IndV32, IndReg ),
            op!(0x2C => "movd", IndV32, IndV32 ),

            op!(0x8A => "swap", Reg, Reg ),

            // logic
            op!(0x2D => "or",  Reg, Reg  ),
            op!(0x2E => "or",  Reg, V8   ),
            op!(0x2F => "or",  Reg, V16  ),
            op!(0x30 => "or",  Reg, V32  ),
            op!(0x31 => "xor", Reg, Reg  ),
            op!(0x32 => "xor", Reg, V8   ),
            op!(0x33 => "xor", Reg, V16  ),
            op!(0x34 => "xor", Reg, V32  ),
            op!(0x35 => "and", Reg, Reg  ),
            op!(0x36 => "and", Reg, V8   ),
            op!(0x37 => "and", Reg, V16  ),
            op!(0x38 => "and", Reg, V32  ),
            op!(0x39 => "shl", Reg, Reg  ),
            op!(0x3A => "shl", Reg, V8   ),
            op!(0x3D => "shr", Reg, Reg  ),
            op!(0x3E => "shr", Reg, V8   ),
            op!(0x41 => "not", Reg       ),

            // arithmetic
            op!(0x42 => "add",  Reg, Reg  ),
            op!(0x43 => "add",  Reg, V8   ),
            op!(0x44 => "add",  Reg, V16  ),
            op!(0x45 => "add",  Reg, V32  ),
            op!(0x46 => "sub",  Reg, Reg  ),
            op!(0x47 => "sub",  Reg, V8   ),
            op!(0x48 => "sub",  Reg, V16  ),
            op!(0x49 => "sub",  Reg, V32  ),
            op!(0x4A => "cmp",  Reg, Reg  ),
            op!(0x4B => "cmp",  Reg, V8   ),
            op!(0x4C => "cmp",  Reg, V16  ),
            op!(0x4D => "cmp",  Reg, V32  ),
            op!(0x8B => "cmp",  Reg       ),
            op!(0x4E => "mul",  Reg, Reg  ),
            op!(0x4F => "mul",  Reg, V8   ),
            op!(0x50 => "mul",  Reg, V16  ),
            op!(0x51 => "mul",  Reg, V32  ),
            op!(0x52 => "idiv", Reg, Reg  ),
            op!(0x53 => "idiv", Reg, V8   ),
            op!(0x54 => "idiv", Reg, V16  ),
            op!(0x55 => "idiv", Reg, V32  ),
            op!(0x56 => "mod",  Reg, Reg  ),
            op!(0x57 => "mod",  Reg, V8   ),
            op!(0x58 => "mod",  Reg, V16  ),
            op!(0x59 => "mod",  Reg, V32  ),
            op!(0x5A => "inc",  Reg       ),
            op!(0x5B => "dec",  Reg       ),

            // jumps
            op!(0x5C => "bz",   Reg ),
            op!(0x5D => "bz",   V32 ),
            op!(0x5C => "beq",  Reg ),
            op!(0x5D => "beq",  V32 ),
            op!(0x5E => "bnz",  Reg ),
            op!(0x5F => "bnz",  V32 ),
            op!(0x60 => "bneg", Reg ),
            op!(0x61 => "bneg", V32 ),
            op!(0x62 => "bpos", Reg ),
            op!(0x63 => "bpos", V32 ),
            op!(0x64 => "bgt",  Reg ),
            op!(0x65 => "bgt",  V32 ),
            op!(0x66 => "bgte", Reg ),
            op!(0x67 => "bgte", V32 ),
            op!(0x68 => "blt",  Reg ),
            op!(0x69 => "blt",  V32 ),
            op!(0x6A => "blte", Reg ),
            op!(0x6B => "blte", V32 ),
            op!(0x6C => "bv",   Reg ),
            op!(0x6D => "bv",   V32 ),
            op!(0x6E => "bnv",  Reg ),
            op!(0x6F => "bnv",  V32 ),
            op!(0x70 => "jmp",  Reg ),
            op!(0x71 => "jmp",  V32 ),
            op!(0x72 => "jsr",  Reg ),
            op!(0x73 => "jsr",  V32 ),
            op!(0x74 => "ret"       ),
            op!(0x75 => "sys",  Reg ),
            op!(0x76 => "sys",  V8  ),
            op!(0x77 => "iret"      ),
            op!(0x86 => "sret"      ),

            // stack
            op!(0x78 => "pushb",  Reg ),
            op!(0x79 => "pushb",  V8  ),
            op!(0x7A => "pushw",  Reg ),
            op!(0x7B => "pushw",  V16 ),
            op!(0x7C => "pushd",  Reg ),
            op!(0x7D => "pushd",  V32 ),
            op!(0x7E => "push.a"      ),
            op!(0x7F => "popb",   Reg ),
            op!(0x80 => "popw",   Reg ),
            op!(0x81 => "popd",   Reg ),
            op!(0x82 => "pop.a"       ),
            op!(0x83 => "popx",   Reg ),
            op!(0x84 => "popx",   V8  ),
            op!(0x85 => "popx",   V16 ),

            // other
            op!(0x87 => "nop" ),
            op!(0x88 => "halt"),
            op!(0x89 => "dbg" ),
        ];
        // }}}

        let mut pos = 0u32;
        for opc in &opcodes {
            if opcode == opc.name && par == opc.pars {  // TODO - equality?
                computer.set(pos, opc.opcode);
                pos += 1;
                break;
            }
        }

        // add parameters
        // TODO - registers in both parameters
        for p in &par {
            match p {
                &Par::Reg(v) | &Par::IndReg(v) | &Par::V8(v) => { computer.set(pos, v); pos += 1; },
                &Par::V16(v)                                 => { computer.set16(pos, v); pos += 2; }, 
                &Par::V32(v) | &Par::IndV32(v)               => { computer.set32(pos, v); pos += 4; },
            }
        }
    }

    fn prepare_cpu<F>(preparation: F, code: &str) -> Computer
            where F : Fn(&mut Computer) {
        let mut computer = Computer::new(64 * 1024);
        computer.set_cpu(CPU::new(), 0xF0001000);
        preparation(&mut computer);
        add_code(&mut computer, code);
        computer
    }

    #[test]
    fn flags() {
        let mut cpu = CPU::new();
        assert_eq!(cpu.flag(Flag::GT), false);
        cpu.set_flag(Flag::GT, true);
        assert_eq!(cpu.flag(Flag::GT), true);
    }

    #[test]
    fn parser() {
        let computer = prepare_cpu(|_|(), "add B, 0xBF");
        assert_eq!(computer.get(0x0), 0x43);
        assert_eq!(computer.get(0x1), 0x01);
        assert_eq!(computer.get(0x2), 0xBF);
    }

    /*
    #[test]
    fn MOV() {
        let computer = prepare_cpu(|comp| comp.cpu_mut().register[Register::B as usize] = 0x42, "mov A, B");
        assert_eq!(computer.cpu().register[Register::A as usize], 0x42);
    }
    */
}
