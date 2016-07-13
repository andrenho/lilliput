use device::*;

pub struct CPU {
    A: u32,
}

impl CPU {
    pub fn new() -> CPU {
        CPU { A: 0 }
    }
}

impl Device for CPU {
    fn dev_get(&self, pos: u32) -> u8 { return 0x0; }
    fn dev_set(&mut self, _pos: u32, _data: u8) {}
    fn dev_size(&self) -> u32 { return 0x0; }
}


/*
#[cfg(test)]
mod tests {
    use super::CPU;
    use computer::*;

    fn add_code(computer: &mut Computer, code: &str) {
        // TODO
    }

    fn prepare_cpu<F>(preparation: F, code: &str) -> Computer
            where F : Fn(&mut Computer) {
        let mut computer = Computer::new(64 * 1024);
        let mut cpu = CPU::new();
        computer.add_device(Box::new(cpu), None);
        preparation(&mut computer);
        add_code(&mut computer, code);
        computer
    }

    #[test]
    fn MOV() {
        let mut computer = prepare_cpu(|comp| comp.set(0, 0), "mov A, B");
        let mut computer2 = prepare_cpu(|comp| comp.set(0, 0), "mov A, B");
    }
}
*/
