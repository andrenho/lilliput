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

    pub fn get(pos: u32) -> u8 {
    }

    pub fn set(pos: u32, data: u8) {
    
    }

}

#[test]
fn works() {
}
