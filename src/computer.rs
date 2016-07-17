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
        match(self.physical_meory
    }

    pub fn set(&mut self, pos: u32, data: u8) {
        if pos < 0xF0000000 {
        }
    }

}


#[test]
fn physical_memory() {
    let mut computer = Computer::new(64 * 1024);
    computer.set(0x12, 0xAF);
    assert_eq!(computer.get(0x12), 0xAF);
}
