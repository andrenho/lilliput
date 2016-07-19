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
// {{{
        // TODO
// }}}
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
