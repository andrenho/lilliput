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
    MOV, MOVB,
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
            _    => panic!(format!("Invalid instruction 0x{:02x}", computer.get(pc)))
        }
    }

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
        let mut computer = Computer::new(64 * 1024);
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
}
// }}}
