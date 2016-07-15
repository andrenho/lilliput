use std::time::Duration;

use computer::*;
use device::*;

pub enum Register { A, B, C, D, E, F, G, H, I, J, K, L, FP, SP, PC, FL }

#[allow(dead_code)]
pub enum Flag { Y, V, Z, S, GT, LT, P, T }

// parameters
#[derive(Debug)]
enum Par { Reg(u8), IndReg(u8), V8(u8), V16(u16), V32(u32), IndV32(u32) }
impl PartialEq for Par { // {{{ PartialEq
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
} // }}}

enum Instruction {
    MOV, MOVB, MOVW, MOVD,
    OR, XOR, AND, SHL, SHR, NOT,
}

//
// CPU
//
pub struct CPU {
    register: [u32; 16],
}

macro_rules! reg {
    ($cpu:expr, $reg:ident) => {{ $cpu.register[Register::$reg as usize] }};
    ($cpu:expr, $reg:ident = $value:expr) => {{ $cpu.register[Register::$reg as usize] = $value }};
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

    fn set_flag(&mut self, f: Flag, v: bool) {
        let value = if v { 1i64 } else { 0i64 };
        let mut new_value = self.register[Register::FL as usize] as i64;
        new_value ^= (-value ^ new_value) & (1 << (f as u32));
        self.register[Register::FL as usize] = new_value as u32;
    }

    #[inline]
    fn parse_opcode(&self, computer: &Computer) -> (Instruction, Vec<Par>, u32) {
        let reg = |p| Par::Reg(computer.get(p));
        let v8 =  |p| Par::V8(computer.get(p));
        let v16 = |p| Par::V16(computer.get16(p));
        let v32 = |p| Par::V32(computer.get32(p));
        let indreg =  |p| Par::IndReg(computer.get(p));
        let indv32 =  |p| Par::IndV32(computer.get32(p));
        let regs_rr = |p| vec![Par::Reg(computer.get(p) >> 4), Par::Reg(computer.get(p) & 0xF) ];
        let regs_ri = |p| vec![Par::Reg(computer.get(p) >> 4), Par::IndReg(computer.get(p) & 0xF) ];
        let regs_ir = |p| vec![Par::IndReg(computer.get(p) >> 4), Par::Reg(computer.get(p) & 0xF) ];
        let regs_ii = |p| vec![Par::IndReg(computer.get(p) >> 4), Par::IndReg(computer.get(p) & 0xF) ];

        let pc = reg!(self, PC);
        match computer.get(pc) {
            // {{{ instructions
			// MOV
            0x01 => (Instruction::MOV, regs_rr(pc+1), 2),
            0x02 => (Instruction::MOV, vec![reg(pc+1), v8(pc+2)], 3),
            0x03 => (Instruction::MOV, vec![reg(pc+1), v16(pc+2)], 4),
            0x04 => (Instruction::MOV, vec![reg(pc+1), v32(pc+2)], 6),
			// MOVB
            0x05 => (Instruction::MOVB, regs_ri(pc+1), 2),
            0x06 => (Instruction::MOVB, vec![reg(pc+1), indv32(pc+2)], 6),
            0x0B => (Instruction::MOVB, regs_ir(pc+1), 2),
            0x0C => (Instruction::MOVB, vec![indreg(pc+1), v8(pc+2)], 3),
            0x0D => (Instruction::MOVB, regs_ii(pc+1), 2),
            0x0E => (Instruction::MOVB, vec![indreg(pc+1), indv32(pc+2)], 6),
            0x21 => (Instruction::MOVB, vec![indv32(pc+1), reg(pc+5)], 6),
            0x22 => (Instruction::MOVB, vec![indv32(pc+1), v8(pc+5)], 6),
            0x23 => (Instruction::MOVB, vec![indv32(pc+1), indreg(pc+5)], 6),
            0x24 => (Instruction::MOVB, vec![indv32(pc+1), indv32(pc+5)], 9),
			// MOVW
            0x07 => (Instruction::MOVW, regs_ri(pc+1), 2),
            0x08 => (Instruction::MOVW, vec![reg(pc+1), indv32(pc+2)], 6),
            0x0F => (Instruction::MOVW, regs_ir(pc+1), 2),
            0x1A => (Instruction::MOVW, vec![indreg(pc+1), v16(pc+2)], 4),
            0x1B => (Instruction::MOVW, regs_ii(pc+1), 2),
            0x1C => (Instruction::MOVW, vec![indreg(pc+1), indv32(pc+2)], 6),
            0x25 => (Instruction::MOVW, vec![indv32(pc+1), reg(pc+5)], 6),
            0x26 => (Instruction::MOVW, vec![indv32(pc+1), v16(pc+5)], 7),
            0x27 => (Instruction::MOVW, vec![indv32(pc+1), indreg(pc+5)], 6),
            0x28 => (Instruction::MOVW, vec![indv32(pc+1), indv32(pc+5)], 9),
			// MOVW
            0x09 => (Instruction::MOVD, regs_ri(pc+1), 2),
            0x0A => (Instruction::MOVD, vec![reg(pc+1), indv32(pc+2)], 6),
            0x1D => (Instruction::MOVD, regs_ir(pc+1), 2),
            0x1E => (Instruction::MOVD, vec![indreg(pc+1), v32(pc+2)], 6),
            0x1F => (Instruction::MOVD, regs_ii(pc+1), 2),
            0x20 => (Instruction::MOVD, vec![indreg(pc+1), indv32(pc+2)], 6),
            0x29 => (Instruction::MOVD, vec![indv32(pc+1), reg(pc+5)], 6),
            0x2A => (Instruction::MOVD, vec![indv32(pc+1), v32(pc+5)], 9),
            0x2B => (Instruction::MOVD, vec![indv32(pc+1), indreg(pc+5)], 6),
            0x2C => (Instruction::MOVD, vec![indv32(pc+1), indv32(pc+5)], 9),
            // OR
            0x2D => (Instruction::OR, regs_rr(pc+1), 2),
            0x2E => (Instruction::OR, vec![reg(pc+1), v8(pc+2)], 3),
            0x2F => (Instruction::OR, vec![reg(pc+1), v16(pc+2)], 4),
            0x30 => (Instruction::OR, vec![reg(pc+1), v32(pc+2)], 6),
            // XOR
            0x31 => (Instruction::XOR, regs_rr(pc+1), 2),
            0x32 => (Instruction::XOR, vec![reg(pc+1), v8(pc+2)], 3),
            0x33 => (Instruction::XOR, vec![reg(pc+1), v16(pc+2)], 4),
            0x34 => (Instruction::XOR, vec![reg(pc+1), v32(pc+2)], 6),
            // AND
            0x35 => (Instruction::AND, regs_rr(pc+1), 2),
            0x36 => (Instruction::AND, vec![reg(pc+1), v8(pc+2)], 3),
            0x37 => (Instruction::AND, vec![reg(pc+1), v16(pc+2)], 4),
            0x38 => (Instruction::AND, vec![reg(pc+1), v32(pc+2)], 6),
            // SH*
            0x39 => (Instruction::SHL, regs_rr(pc+1), 2),
            0x3A => (Instruction::SHL, vec![reg(pc+1), v8(pc+2)], 3),
            0x3D => (Instruction::SHR, regs_rr(pc+1), 2),
            0x3E => (Instruction::SHR, vec![reg(pc+1), v8(pc+2)], 3),
            // NOT
            0x41 => (Instruction::NOT, vec![reg(pc+1)], 2),
            // }}}
            // invalid
            _    => panic!(format!("Invalid instruction 0x{:02x}", computer.get(pc)))
        }
    }

    #[inline]
    fn take(&self, par: &Par, computer: &Computer) -> u32 {
        match par {
            &Par::Reg(v)    => self.register[v as usize],
            &Par::IndReg(v) => computer.get32(self.register[v as usize]),
            &Par::V8(v)     => v as u32,
            &Par::V16(v)    => v as u32,
            &Par::V32(v)    => v,
            &Par::IndV32(v) => computer.get32(v),
        }
    }

    #[inline]
    fn apply(&mut self, par: &Par, value: u32, cmds: &mut Vec<Command>, size: u8) {
        self.set_flag(Flag::Z, value == 0);
        self.set_flag(Flag::P, (value % 2) == 0);
        self.set_flag(Flag::S, ((value >> 31) & 1) == 1);
        self.set_flag(Flag::V, false);
        self.set_flag(Flag::Y, false);
        self.set_flag(Flag::GT, false);
        self.set_flag(Flag::LT, false);

        match par {
            &Par::Reg(v)    => self.register[v as usize] = value,
            &Par::IndReg(v) => match size {
                                     8 => cmds.push(Command::Set8(self.register[v as usize], value as u8)),
                                    16 => cmds.push(Command::Set16(self.register[v as usize], value as u16)),
                                    32 => cmds.push(Command::Set32(self.register[v as usize], value as u32)),
                                    _  => panic!("Invalid choice")
                               },
            &Par::IndV32(v) => match size {
                                     8 => cmds.push(Command::Set8(v, value as u8)),
                                    16 => cmds.push(Command::Set16(v, value as u16)),
                                    32 => cmds.push(Command::Set32(v, value as u32)),
                                    _  => panic!("Invalid choice")
                               },
            _               => unimplemented!()
        };
    }

}

impl Device for CPU {
    fn get(&self, _pos: u32) -> u8 { 0x0 }
    fn set(&mut self, _pos: u32, _data: u8) {}
    fn size(&self) -> u32 { 0x0 }

    fn step(&mut self, computer: &Computer, _dt: &Duration, cmds: &mut Vec<Command>) {

        let (instruction, pars, sz) = self.parse_opcode(computer);
        match instruction {
            Instruction::MOV => { 
                let value = self.take(&pars[1], computer);
                self.apply(&pars[0], value, cmds, 0);
            },
            Instruction::MOVB => { 
                let value = self.take(&pars[1], computer) as u8;
                self.apply(&pars[0], value as u32, cmds, 8);
            },
            Instruction::MOVW => { 
                let value = self.take(&pars[1], computer) as u16;
                self.apply(&pars[0], value as u32, cmds, 16);
            },
            Instruction::MOVD => { 
                let value = self.take(&pars[1], computer) as u32;
                self.apply(&pars[0], value as u32, cmds, 32);
            },
            Instruction::OR => {
                let value_a = self.take(&pars[0], computer) as u32;
                let value_b = self.take(&pars[1], computer) as u32;
                self.apply(&pars[0], (value_a | value_b) as u32, cmds, 0);
            },
            Instruction::XOR => {
                let value_a = self.take(&pars[0], computer) as u32;
                let value_b = self.take(&pars[1], computer) as u32;
                self.apply(&pars[0], (value_a ^ value_b) as u32, cmds, 0);
            },
            Instruction::AND => {
                let value_a = self.take(&pars[0], computer) as u32;
                let value_b = self.take(&pars[1], computer) as u32;
                self.apply(&pars[0], (value_a & value_b) as u32, cmds, 0);
            },
            Instruction::SHL => {
                let value_a = self.take(&pars[0], computer) as u32;
                let value_b = self.take(&pars[1], computer) as u32;
                self.apply(&pars[0], (value_a << value_b) as u32, cmds, 0);
            },
            Instruction::SHR => {
                let value_a = self.take(&pars[0], computer) as u32;
                let value_b = self.take(&pars[1], computer) as u32;
                self.apply(&pars[0], (value_a >> value_b) as u32, cmds, 0);
            },
            Instruction::NOT => {
                let value = self.take(&pars[0], computer) as u32;
                self.apply(&pars[0], !value as u32, cmds, 0);
            },
        }

        let pc = reg!(self, PC);
        reg!(self, PC = pc + sz);
    }
}


// {{{ tests
#[cfg(test)]
#[allow(non_snake_case)]
mod tests {

    use super::*;
    use super::Par;
    use computer::*;

    fn add_code(computer: &mut Computer, code: &str) { // {{{
        // split string
        let _parts: Vec<&str> = code.split_whitespace().collect();
        let opcode = _parts[0];
        let pars: Vec<String> = _parts[1..].into_iter().map(|s| (*s).replace(",", "")).collect();

        // find byte values
        // {{{ functions to understand the parameters
        fn register_from_name(reg: &str) -> Register {
            match &*reg.replace("[", "").replace("]", "") {
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

        fn parse_u32(par: &str) -> u32 {
            let p = &*par.replace("[", "").replace("]", "");
            if p.len() >= 2 && &p[0..2] == "0x" {
                u32::from_str_radix(&p[2..], 16).unwrap()
            } else {
                p.parse::<u32>().unwrap()
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
        let par: Vec<Par> = pars.into_iter().map(|p| get_type(&p)).collect();

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
        match (par.get(0), par.get(1)) {
            // registers in both parameters
            (Some(&Par::Reg(v1)), Some(&Par::Reg(v2)))       => computer.set(pos, (v1 << 4) | v2),
            (Some(&Par::Reg(v1)), Some(&Par::IndReg(v2)))    => computer.set(pos, (v1 << 4) | v2),
            (Some(&Par::IndReg(v1)), Some(&Par::Reg(v2)))    => computer.set(pos, (v1 << 4) | v2),
            (Some(&Par::IndReg(v1)), Some(&Par::IndReg(v2))) => computer.set(pos, (v1 << 4) | v2),
            // other
            _ => {
                for p in &par {
                    match p {
                        &Par::Reg(v) | &Par::IndReg(v) | &Par::V8(v) => { computer.set(pos, v); pos += 1; },
                        &Par::V16(v)                                 => { computer.set16(pos, v); pos += 2; }, 
                        &Par::V32(v) | &Par::IndV32(v)               => { computer.set32(pos, v); pos += 4; },
                    }
                }
            }
        }
    } // }}}

    fn prepare_cpu<F>(preparation: F, code: &str) -> Computer
            where F : Fn(&mut Computer) {
        let mut computer = Computer::new(1024 * 1024);
        computer.set_cpu(CPU::new(), 0xF0001000);
        preparation(&mut computer);
        add_code(&mut computer, code);
        computer.step();
        computer
    }

    #[test]
    fn flags() {
        let mut cpu = CPU::new();
        assert_eq!(cpu.flag(Flag::S), false);
        cpu.set_flag(Flag::S, true);
        assert_eq!(cpu.flag(Flag::S), true);
        cpu.set_flag(Flag::S, false);
        assert_eq!(cpu.flag(Flag::S), false);
    }

    #[test]
    fn parser() {
        /* TODO - reinsert
        let computer = prepare_cpu(|_|(), "add B, 0xBF");
        assert_eq!(computer.get(0x0), 0x43);
        assert_eq!(computer.get(0x1), 0x01);
        assert_eq!(computer.get(0x2), 0xBF);
        */
        let computer2 = prepare_cpu(|_|(), "mov B, C");
        assert_eq!(computer2.get(0x0), 0x01);
        assert_eq!(computer2.get(0x1), 0x12);
        assert_eq!(computer2.get(0x2), 0x00);
    }

    #[test]
    fn MOV() {
        let computer = prepare_cpu(|comp| reg!(comp.cpu_mut(), B = 0x42), "mov A, B");
        assert_eq!(reg!(computer.cpu(), A), 0x42);

        let computer2 = prepare_cpu(|_|(), "mov A, 0x34");
        assert_eq!(reg!(computer2.cpu(), A), 0x34);

        let computer3 = prepare_cpu(|_|(), "mov A, 0x1234");
        assert_eq!(reg!(computer3.cpu(), A), 0x1234);

        let computer4 = prepare_cpu(|_|(), "mov A, 0xFABC1234");
        assert_eq!(reg!(computer4.cpu(), A), 0xFABC1234);

        let computer5 = prepare_cpu(|_|(), "mov A, 0");
        assert_eq!(computer5.cpu().flag(Flag::Z), true);
        assert_eq!(computer5.cpu().flag(Flag::P), true);
        assert_eq!(computer5.cpu().flag(Flag::S), false);

        let computer6 = prepare_cpu(|_|(), "mov A, 0xF0000001");
        assert_eq!(computer6.cpu().flag(Flag::Z), false);
        assert_eq!(computer6.cpu().flag(Flag::P), false);
        assert_eq!(computer6.cpu().flag(Flag::S), true);
    }

    #[test]
    fn MOVB() {
        let computer = prepare_cpu(|c| { reg!(c.cpu_mut(), B = 0x100); c.set(0x100, 0xAB); }, 
            "movb A, [B]");
        assert_eq!(reg!(computer.cpu(), A), 0xAB);

        let computer2 = prepare_cpu(|c| c.set(0x1000, 0xAB), "movb A, [0x1000]");
        assert_eq!(reg!(computer2.cpu(), A), 0xAB);

        let computer3 = prepare_cpu(|c| { reg!(c.cpu_mut(), A = 0x64); reg!(c.cpu_mut(), C = 0x32); },
            "movb [C], 0xFA");
        assert_eq!(computer3.get(0x32), 0xFA);

        let computer4 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0x64), "movb [A], 0xFA");
        assert_eq!(computer4.get(0x64), 0xFA);

        let computer5 = prepare_cpu(|c| { 
            reg!(c.cpu_mut(), A = 0x32); 
            reg!(c.cpu_mut(), B = 0x64); 
            c.set(0x64, 0xFF);
        }, "movb [A], [B]");
        assert_eq!(computer5.get(0x32), 0xFF);

        let computer6 = prepare_cpu(|c| { reg!(c.cpu_mut(), A = 0x32); c.set(0x6420, 0xFF); }, 
            "movb [A], [0x6420]");
        assert_eq!(computer6.get(0x32), 0xFF);

        let computer7 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0xAC32), "movb [0x64], A");
        assert_eq!(computer7.get(0x64), 0x32);

        let computer8 = prepare_cpu(|_|(), "movb [0x64], 0xF0");
        assert_eq!(computer8.get(0x64), 0xF0);

        let computer9 = prepare_cpu(|c| { reg!(c.cpu_mut(), A = 0xF000); c.set(0xF000, 0x42); }, 
            "movb [0xCC64], [A]");
        assert_eq!(computer9.get(0xCC64), 0x42);

        let computer10 = prepare_cpu(|c| c.set(0xABF0, 0x3F), "movb [0x64], [0xABF0]");
        assert_eq!(computer10.get(0x64), 0x3F);
    }

    #[test]
    fn MOVW() {
        let computer = prepare_cpu(|c| { reg!(c.cpu_mut(), B = 0x1000); c.set16(0x1000, 0xABCD); }, 
            "movw A, [B]");
        assert_eq!(reg!(computer.cpu(), A), 0xABCD);

        let computer2 = prepare_cpu(|c| c.set16(0x1000, 0xABCD), "movw A, [0x1000]");
        assert_eq!(reg!(computer2.cpu(), A), 0xABCD);

        let computer3 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0x6402), "movw [A], A");
        assert_eq!(computer3.get16(0x6402), 0x6402);

        let computer4 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0x64), "movw [A], 0xFABA");
        assert_eq!(computer4.get16(0x64), 0xFABA);

        let computer5 = prepare_cpu(|c| { 
            reg!(c.cpu_mut(), A = 0x32CC); 
            reg!(c.cpu_mut(), B = 0x64); 
            c.set16(0x64, 0xFFAB);
        }, "movw [A], [B]");
        assert_eq!(computer5.get16(0x32CC), 0xFFAB);

        let computer6 = prepare_cpu(|c| { reg!(c.cpu_mut(), A = 0x32); c.set16(0x6420, 0xFFAB); }, 
            "movw [A], [0x6420]");
        assert_eq!(computer6.get16(0x32), 0xFFAB);

        let computer7 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0xAB32AC), "movw [0x64], A");
        assert_eq!(computer7.get16(0x64), 0x32AC);

        let computer8 = prepare_cpu(|_|(), "movw [0x64], 0xF0FA");
        assert_eq!(computer8.get16(0x64), 0xF0FA);

        let computer9 = prepare_cpu(|c| { reg!(c.cpu_mut(), A = 0xF000); c.set16(0xF000, 0x4245); }, 
            "movw [0xCC64], [A]");
        assert_eq!(computer9.get16(0xCC64), 0x4245);

        let computer10 = prepare_cpu(|c| c.set16(0xABF0, 0x3FAB), "movw [0x64], [0xABF0]");
        assert_eq!(computer10.get16(0x64), 0x3FAB);
    }

    #[test]
    fn MOVD() {
        let computer = prepare_cpu(|c| { reg!(c.cpu_mut(), B = 0x1000); c.set32(0x1000, 0xABCDEF01); }, 
            "movd A, [B]");
        assert_eq!(reg!(computer.cpu(), A), 0xABCDEF01);

        let computer2 = prepare_cpu(|c| c.set32(0x1000, 0xABCDEF01), "movd A, [0x1000]");
        assert_eq!(reg!(computer2.cpu(), A), 0xABCDEF01);

        let computer3 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0x16402), "movd [A], A");
        assert_eq!(computer3.get32(0x16402), 0x16402);

        let computer4 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0x64), "movd [A], 0xFABA1112");
        assert_eq!(computer4.get32(0x64), 0xFABA1112);

        let computer5 = prepare_cpu(|c| { 
            reg!(c.cpu_mut(), A = 0x32CC); 
            reg!(c.cpu_mut(), B = 0x64); 
            c.set32(0x64, 0xFFAB5678);
        }, "movd [A], [B]");
        assert_eq!(computer5.get32(0x32CC), 0xFFAB5678);

        let computer6 = prepare_cpu(|c| { reg!(c.cpu_mut(), A = 0x32); c.set32(0x6420, 0xFFAB9876); }, 
            "movd [A], [0x6420]");
        assert_eq!(computer6.get32(0x32), 0xFFAB9876);

        let computer7 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0xAB32AC44), "movd [0x64], A");
        assert_eq!(computer7.get32(0x64), 0xAB32AC44);

        let computer8 = prepare_cpu(|_|(), "movd [0x64], 0xF0FA1234");
        assert_eq!(computer8.get32(0x64), 0xF0FA1234);

        let computer9 = prepare_cpu(|c| { reg!(c.cpu_mut(), A = 0xF000); c.set32(0xF000, 0x4245AABB); }, 
            "movd [0xCC64], [A]");
        assert_eq!(computer9.get32(0xCC64), 0x4245AABB);

        let computer10 = prepare_cpu(|c| c.set32(0xABF0, 0x3FABC312), "movd [0x64], [0xABF0]");
        assert_eq!(computer10.get32(0x64), 0x3FABC312);
    }

    #[test]
    fn OR()
    {
        let computer = prepare_cpu(|c| { reg!(c.cpu_mut(), A = 0b1010); reg!(c.cpu_mut(), B = 0b1100); },
            "or A, B");
        assert_eq!(reg!(computer.cpu(), A), 0b1110);
        assert_eq!(computer.cpu().flag(Flag::S), false);
        assert_eq!(computer.cpu().flag(Flag::P), true);
        assert_eq!(computer.cpu().flag(Flag::Z), false);
        assert_eq!(computer.cpu().flag(Flag::Y), false);
        assert_eq!(computer.cpu().flag(Flag::V), false);

        let computer2 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0b11), "or A, 0x4");
        assert_eq!(reg!(computer2.cpu(), A), 0b111);

        let computer3 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0b111), "or A, 0x4000");
        assert_eq!(reg!(computer3.cpu(), A), 0x4007);

        let computer4 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0x10800000), "or A, 0x2A426653");
        assert_eq!(reg!(computer4.cpu(), A), 0x3AC26653);
    }

    #[test]
    fn XOR()
    {
        let computer = prepare_cpu(|c| { reg!(c.cpu_mut(), A = 0b1010); reg!(c.cpu_mut(), B = 0b1100); },
            "xor A, B");
        assert_eq!(reg!(computer.cpu(), A), 0b110);

        let computer2 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0b11), "xor A, 0x4");
        assert_eq!(reg!(computer2.cpu(), A), 0b111);

        let computer3 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0xFF0), "xor A, 0xFF00");
        assert_eq!(reg!(computer3.cpu(), A), 0xF0F0);

        let computer4 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0x148ABD12), "xor A, 0x2A426653");
        assert_eq!(reg!(computer4.cpu(), A), 0x3EC8DB41);
    }

    #[test]
    fn AND()
    {
        let computer = prepare_cpu(|c| { reg!(c.cpu_mut(), A = 0b11); reg!(c.cpu_mut(), B = 0b1100); },
            "and A, B");
        assert_eq!(reg!(computer.cpu(), A), 0);
        assert_eq!(computer.cpu().flag(Flag::Z), true);

        let computer2 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0b11), "and A, 0x7");
        assert_eq!(reg!(computer2.cpu(), A), 0b11);

        let computer3 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0xFF00), "and A, 0xFF0");
        assert_eq!(reg!(computer3.cpu(), A), 0xF00);

        let computer4 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0x148ABD12), "and A, 0x2A426653");
        assert_eq!(reg!(computer4.cpu(), A), 0x22412);
    }

    #[test]
    fn SHx()
    {
        let computer = prepare_cpu(|c| { reg!(c.cpu_mut(), A = 0b10101010); reg!(c.cpu_mut(), B = 4); },
            "shl A, B");
        assert_eq!(reg!(computer.cpu(), A), 0b101010100000);

        let computer2 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0b10101010), "shl A, 4");
        assert_eq!(reg!(computer2.cpu(), A), 0b101010100000);

        let computer3 = prepare_cpu(|c| { reg!(c.cpu_mut(), A = 0b10101010); reg!(c.cpu_mut(), B = 4); }, "shr A, B");
        assert_eq!(reg!(computer3.cpu(), A), 0b1010);

        let computer4 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0b10101010), "shr A, 4");
        assert_eq!(reg!(computer4.cpu(), A), 0b1010);
    }

    #[test]
    fn NOT()
    {
        let computer = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0b11001010), "not A");
        assert_eq!(reg!(computer.cpu(), A), 0b11111111111111111111111100110101);
    }

    #[test]
    fn ADD()
    {
        let computer = prepare_cpu(|c| { reg!(c.cpu_mut(), A = 0x12); reg!(c.cpu_mut(), B = 0x20); },
            "add A, B");
        assert_eq!(reg!(computer.cpu(), A), 0x32);

        let computer2 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0x12), "and A, 0x20");
        assert_eq!(reg!(computer2.cpu(), A), 0x32);

        // with carry
        let computer2c = prepare_cpu(|c| { reg!(c.cpu_mut(), A = 0x12), c.cpu_mut().set_flag(Flag::Y, true); }
            "and A, 0x20");
        assert_eq!(reg!(computer2c.cpu(), A), 0x32);

        let computer3 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0x12), "and A, 0x2000");
        assert_eq!(reg!(computer3.cpu(), A), 0x2012);

        let computer4 = prepare_cpu(|c| reg!(c.cpu_mut(), A = 0x10000012), "and A, 0xF0000000");
        assert_eq!(reg!(computer4.cpu(), A), 0x12);
        assert_eq!(computer.cpu().flag(Flag::Y), true);
    }

/*
  //
  // integer math
  //

  t.comment('Integer arithmetic');
  
  s = opc('add A, B', () => { cpu.A = 0x12; cpu.B = 0x20; });
  t.equal(cpu.A, 0x32, s);
  
  s = opc('add A, 0x20', () => cpu.A = 0x12);
  t.equal(cpu.A, 0x32, s);

  s = opc('add A, 0x20', () => { cpu.A = 0x12, cpu.Y = true; });
  t.equal(cpu.A, 0x33, 'add A, 0x20 (with carry)');

  s = opc('add A, 0x2000', () => cpu.A = 0x12);
  t.equal(cpu.A, 0x2012, s);

  s = opc('add A, 0xF0000000', () => cpu.A = 0x10000012);
  t.equal(cpu.A, 0x12, s);
  t.true(cpu.Y, "cpu.Y == 1");

  s = opc('sub A, B', () => { cpu.A = 0x30; cpu.B = 0x20; });
  t.equal(cpu.A, 0x10, s);
  t.false(cpu.S, 'cpu.S == 0');

  s = opc('sub A, B', () => { cpu.A = 0x20; cpu.B = 0x30; });
  t.equal(cpu.A, 0xFFFFFFF0, 'sub A, B (negative)');
  t.true(cpu.S, 'cpu.S == 1');

  s = opc('sub A, 0x20', () => cpu.A = 0x22);
  t.equal(cpu.A, 0x2, s);

  s = opc('sub A, 0x20', () => { cpu.A = 0x22; cpu.Y = true; });
  t.equal(cpu.A, 0x1, 'sub A, 0x20 (with carry)');

  s = opc('sub A, 0x2000', () => cpu.A = 0x12);
  t.equal(cpu.A, 0xFFFFE012, s);
  t.true(cpu.S, 'cpu.S == 1');
  t.true(cpu.Y, 'cpu.Y == 1');

  s = opc('sub A, 0xF0000000', () => cpu.A = 0x10000012);
  t.equal(cpu.A, 0x20000012, s);
  t.true(cpu.Y, 'cpu.Y == 1');

  s = opc('cmp A, B');
  t.true(cpu.Z, s);

  s = opc('cmp A, 0x12');
  t.true(cpu.LT && !cpu.GT, s);

  s = opc('cmp A, 0x1234', () => cpu.A = 0x6000);
  t.true(!cpu.LT && cpu.GT, s);

  s = opc('cmp A, 0x12345678', () => cpu.A = 0xF0000000);
  t.true(!cpu.LT && cpu.GT, s);  // because of the signal!

  s = opc('cmp A', () => cpu.A = 0x0);
  t.true(cpu.Z, s);

  s = opc('mul A, B', () => { cpu.A = 0xF0; cpu.B = 0xF000; });
  t.equal(cpu.A, 0xE10000, s);

  s = opc('mul A, 0x12', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x147A8, s);

  s = opc('mul A, 0x12AF', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x154198C, s);
  t.false(cpu.V, 'cpu.V == 0');

  s = opc('mul A, 0x12AF87AB', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x233194BC, s);
  t.true(cpu.V, 'cpu.V == 1');

  s = opc('idiv A, B', () => { cpu.A = 0xF000; cpu.B = 0xF0; });
  t.equal(cpu.A, 0x100, s);

  s = opc('idiv A, 0x12', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x102, s);

  s = opc('idiv A, 0x2AF', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x6, s);

  s = opc('idiv A, 0x12AF', () => cpu.A = 0x123487AB);
  t.equal(cpu.A, 0xF971, s);

  s = opc('mod A, B', () => { cpu.A = 0xF000; cpu.B = 0xF0; });
  t.equal(cpu.A, 0x0, s);
  t.true(cpu.Z, 'cpu.Z == 1');

  s = opc('mod A, 0x12', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x10, s);

  s = opc('mod A, 0x2AF', () => cpu.A = 0x1234);
  t.equal(cpu.A, 0x21A, s);

  s = opc('mod A, 0x12AF', () => cpu.A = 0x123487AB);
  t.equal(cpu.A, 0x116C, s);

  s = opc('inc A', () => cpu.A = 0x42);
  t.equal(cpu.A, 0x43, s);

  s = opc('inc A', () => cpu.A = 0xFFFFFFFF);
  t.equal(cpu.A, 0x0, 'inc A (overflow)');
  t.true(cpu.Y, 'cpu.Y == 1');
  t.true(cpu.Z, 'cpu.Z == 1');

  s = opc('dec A', () => cpu.A = 0x42);
  t.equal(cpu.A, 0x41, s);

  s = opc('dec A', () => cpu.A = 0x0);
  t.equal(cpu.A, 0xFFFFFFFF, 'dec A (underflow)');
  t.false(cpu.Z, 'cpu.Z == 0');

  // 
  // branches
  //

  t.comment('Branch operations');

  s = opc('bz A', () => { cpu.Z = true; cpu.A = 0x1000; });
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bz A', () => { cpu.A = 0x1000; });
  t.equal(cpu.PC, 0x2, 'bz A (false)');

  s = opc('bz 0x1000', () => cpu.Z = true);
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bnz A', () => cpu.A = 0x1000);
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bnz 0x1000');
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bneg A', () => { cpu.S = true; cpu.A = 0x1000; });
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bneg A', () => { cpu.A = 0x1000; });
  t.equal(cpu.PC, 0x2, 'bneg A (false)');

  s = opc('bneg 0x1000', () => cpu.S = true);
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bpos A', () => cpu.A = 0x1000);
  t.equal(cpu.PC, 0x1000, s);

  s = opc('bpos 0x1000');
  t.equal(cpu.PC, 0x1000, s);

  s = opc('jmp 0x12345678');
  t.equal(cpu.PC, 0x12345678, s);
  

  // 
  // stack
  //

  t.comment('Stack operations');

  mb.reset();
  cpu.SP = 0xFFF; 
  cpu.A = 0xABCDEF12;

  mb.setArray(0x0, Debugger.encode('pushb A'));
  mb.setArray(0x2, Debugger.encode('pushb 0x12'));
  mb.setArray(0x4, Debugger.encode('pushw A'));
  mb.setArray(0x6, Debugger.encode('pushd A'));

  mb.setArray(0x8, Debugger.encode('popd B'));
  mb.setArray(0xA, Debugger.encode('popw B'));
  mb.setArray(0xC, Debugger.encode('popb B'));

  mb.setArray(0xE, Debugger.encode('popx 1'));

  mb.step();
  t.equal(mb.get(0xFFF), 0x12, 'pushb A');
  t.equal(cpu.SP, 0xFFE, 'SP = 0xFFE');

  mb.step();
  t.equal(mb.get(0xFFE), 0x12, 'pushb 0x12');
  t.equal(cpu.SP, 0xFFD, 'SP = 0xFFD');

  mb.step();
  t.equal(mb.get16(0xFFC), 0xEF12, s);
  t.equal(mb.get(0xFFD), 0xEF, s);
  t.equal(mb.get(0xFFC), 0x12, s);
  t.equal(cpu.SP, 0xFFB, 'SP = 0xFFB');

  mb.step();
  t.equal(mb.get32(0xFF8), 0xABCDEF12);
  t.equal(cpu.SP, 0xFF7, 'SP = 0xFF7');

  mb.step();
  t.equal(cpu.B, 0xABCDEF12, 'popd B');

  mb.step();
  t.equal(cpu.B, 0xEF12, 'popw B');

  mb.step();
  t.equal(cpu.B, 0x12, 'popb B');

  mb.step();
  t.equal(cpu.SP, 0xFFF, 'popx 1');

  // all registers
  s = opc('push.a', () => {
    cpu.SP = 0xFFF;
    cpu.A = 0xA1B2C3E4;
    cpu.B = 0xFFFFFFFF;
  });
  t.equal(cpu.SP, 0xFCF, s);
  t.equal(mb.get32(0xFFC), 0xA1B2C3E4, 'A is saved');
  t.equal(mb.get32(0xFF8), 0xFFFFFFFF, 'B is saved');
  
  s = opc('pop.a', () => {
    cpu.SP = 0xFCF;
    mb.set32(0xFFC, 0xA1B2C3E4);
    mb.set32(0xFF8, 0xFFFFFFFF);
  });
  t.equal(cpu.SP, 0xFFF, s);
  t.equal(cpu.A, 0xA1B2C3E4, 'A is restored');
  t.equal(cpu.B, 0xFFFFFFFF, 'B is restored');

  // others
  t.comment('Others');

  opc('nop');
  
  s = opc('dbg');
  t.true(cpu.activateDebugger, s);

  s = opc('halt');
  t.true(cpu.systemHalted, s);

  s = opc('swap A, B', () => {
    cpu.A = 0xA;
    cpu.B = 0xB;
  });
  t.true(cpu.A == 0xB && cpu.B == 0xA, s);

  t.end();

});


test('CPU: subroutines and system calls', t => {

  let [mb, cpu] = makeCPU();

  // jsr
  mb.reset();
  mb.setArray(0x200, Debugger.encode('jsr 0x1234'));
  mb.setArray(0x1234, Debugger.encode('ret'));
  cpu.PC = 0x200;
  cpu.SP = 0xFFF;
  mb.step();
  t.equal(cpu.PC, 0x1234, 'jsr 0x1234');
  t.equal(mb.get(0xFFC), 0x5, '[FFC] = 0x5');
  t.equal(mb.get(0xFFD), 0x2, '[FFD] = 0x2');
  t.equal(cpu.SP, 0xFFB, 'SP = 0xFFB');
  t.equal(mb.get32(0xFFC), 0x200 + 5, 'address in stack'); 

  mb.step();
  t.equal(cpu.PC, 0x205, 'ret');
  t.equal(cpu.SP, 0xFFF, 'SP = 0xFFF');

  // sys
  mb.reset();
  cpu.SP = 0xFFF;
  mb.setArray(0, Debugger.encode('sys 2'));
  mb.set32(cpu.CPU_SYSCALL_VECT + 8, 0x1000);
  t.equal(cpu._syscallVector[2], 0x1000, 'syscall vector');
  mb.setArray(0x1000, Debugger.encode('sret'));

  mb.step();
  t.equal(cpu.PC, 0x1000, 'sys 2');
  t.equal(cpu.SP, 0xFFB, 'SP = 0xFFD');
  mb.step();
  t.equal(cpu.PC, 0x2, 'sret');
  t.equal(cpu.SP, 0xFFF, 'SP = 0xFFF');

  t.end();

});


test('CPU: interrupts', t => {

  let [mb, cpu] = makeCPU();
  cpu.T = true;
  cpu.SP = 0x800;
  mb.set32(cpu.CPU_INTERRUPT_VECT + 8, 0x1000);
  mb.setArray(0x0, Debugger.encode('movb A, [0xE0000000]'));
  mb.setArray(0x1000, Debugger.encode('iret'));

  mb.step();  // cause the exception
  t.equal(cpu.PC, 0x1000, 'interrupt called');
  t.true(cpu.T, 'interrupts disabled');

  mb.step();  // iret
  t.equal(cpu.PC, 0x6, 'iret');
  t.true(cpu.T, 'interrupts enabled');

  t.end();

});


test('CPU: invalid opcode', t => {

  let [mb, cpu] = makeCPU();
  cpu.T = true;
  mb.set32(cpu.CPU_INTERRUPT_VECT + 12, 0x1000);
  mb.set(0x0, 0xFF);
  mb.step();
  t.equal(cpu.PC, 0x1000, 'interrupt called');

  t.end();

});

*/
}
// }}}
