const PHYSICAL_MEMORY_LIMIT: u32 = 0xF0000000;

struct Computer {
    physical_memory : Vec<u8>,
    offset: u32,
}


impl Computer {

    pub fn new(mem_size: u32) -> Computer {
        Computer {
            physical_memory: vec![0u8; mem_size as usize],
            offset: 0x0,
        }
    }

    pub fn get(&self, pos: u32) -> u8 {
        if pos < PHYSICAL_MEMORY_LIMIT {
            match self.physical_memory.get((pos + self.offset) as usize) {
                Some(data) => *data,
                None => panic!("Getting an invalid memory position.")  // TODO
            }
        } else {
            unimplemented!();
        }
    }

    pub fn set(&mut self, pos: u32, data: u8) {
        if pos < PHYSICAL_MEMORY_LIMIT {
            match self.physical_memory.get_mut((pos + self.offset) as usize) {
                Some(old) => *old = data,
                None => panic!("Getting an invalid memory position.")  // TODO
            }
        } else {
            unimplemented!();
        }
    }

}


#[cfg(test)]
mod tests {

    use super::Computer;

    #[test]
    fn physical_memory() {
        let mut computer = Computer::new(64 * 1024);
        computer.set(0x12, 0xAF);
        assert_eq!(computer.get(0x12), 0xAF);
    }

    #[test]
    fn offset() {
        let mut computer = Computer::new(64 * 1024);
        computer.offset = 0x1000;
        computer.set(0x34, 0x68);
        computer.offset = 0x0;
        assert_eq!(computer.get(0x1034), 0x68);
    }
}
