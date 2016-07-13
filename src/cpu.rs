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
        // {{{
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
                _    => panic!("Invalid register")
            }
        }

        #[derive(PartialEq)]
        enum Par { None, Reg(u8), IndReg(u8), V8(u8), V16(u16), V32(u32), IndV32(u32) };
        fn get_type(par: &str) -> Par {
            let fst = par.chars().nth(0).unwrap();
            if fst == '[' {
                if par.chars().nth(1).unwrap().is_alphanumeric() {
                    Par::IndReg(register_from_name(par) as u8)
                } else {
                    Par::IndV32(par.parse::<u32>().unwrap())
                }
            } else if fst.is_alphanumeric() {
                Par::Reg(register_from_name(par) as u8)
            } else {
                let v = par.parse::<u32>().unwrap();
                match v {
                    0x0   ...   0xFF =>  Par::V8(v as  u8),
                    0x100 ... 0xFFFF => Par::V16(v as u16),
                    _                => Par::V32(v as u32)
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
        let opcodes: Vec<Opcode> = vec![
            Opcode { opcode: 0x01, name: "mov", pars: vec![ Par::Reg(0), Par::Reg(0) ] },
        ];

        let mut pos = 0u32;
        for opc in &opcodes {
            if opcode == opc.name && par == opc.pars {  // TODO - equality?
                computer.set(pos, opc.opcode);
                pos += 1;
                break;
            }
        }

        // add parameters

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
