use std::any::Any;

use device::*;
use cpu::*;

const PHYSICAL_MEMORY_LIMIT: u32 = 0xF0000000;

struct MemoryLocation {
    location: u32,
    size: u32,
}

struct DeviceDef {
    memory: Option<MemoryLocation>,
    device: Box<Device>,
}

pub struct Computer {
    physical_memory : Vec<u8>,
    offset: u32,
    devices: Vec<DeviceDef>,
}


impl Computer {

    pub fn new(mem_size: u32) -> Computer {
        Computer {
            physical_memory: vec![0u8; mem_size as usize],
            offset: 0x0,
            devices: vec![],
        }
    }

    pub fn get(&self, pos: u32) -> u8 {
        if pos < PHYSICAL_MEMORY_LIMIT {
            match self.physical_memory.get((pos + self.offset) as usize) {
                Some(data) => *data,
                None => panic!("Getting an invalid memory position.")  // TODO
            }
        } else if pos >= PHYSICAL_MEMORY_LIMIT && pos < PHYSICAL_MEMORY_LIMIT + 4 {
            (self.offset >> ((pos - PHYSICAL_MEMORY_LIMIT) * 8)) as u8
        } else {
            for dev in &self.devices {
                match dev.memory {
                    Some(ref loc) => if pos >= loc.location && pos < (loc.location + loc.size) {
                        return dev.device.dev_get(pos - loc.location);
                    },
                    None => (),
                }
            }
            panic!("Getting an invalid memory position.");
        }
    }

    pub fn set(&mut self, pos: u32, data: u8) {
        if pos < PHYSICAL_MEMORY_LIMIT {
            match self.physical_memory.get_mut((pos + self.offset) as usize) {
                Some(old) => *old = data,
                None => panic!("Setting an invalid memory position.")  // TODO
            }
        } else if pos >= PHYSICAL_MEMORY_LIMIT && pos < PHYSICAL_MEMORY_LIMIT + 4 {
            let bytes = 8 * (pos - PHYSICAL_MEMORY_LIMIT);
            let mask : u32 = 0xFF << bytes;
            self.offset = (self.offset & !mask) | (((data as u32) << bytes) & mask);
        } else {
            for dev in &mut self.devices {
                match dev.memory {
                    Some(ref loc) => if pos >= loc.location && pos < (loc.location + loc.size) {
                        dev.device.dev_set(pos - loc.location, data);
                        return;
                    },
                    None => (),
                }
            }
            panic!("Setting an invalid memory position.");
        }
    }

    pub fn add_device(&mut self, dev: Box<Device>, memory_pos: Option<u32>) -> u32 {
        let (next, mloc) = match memory_pos {
            Some(addr) => {
                if addr < PHYSICAL_MEMORY_LIMIT + 0x100 {
                    panic!("Memory position must be above PHYSICAL_MEMORY_LIMIT.");
                }
                let next = addr + dev.dev_size();
                let memloc = MemoryLocation { location: addr, size: dev.dev_size() };
                (next, Some(memloc))
            },
            None => (0, None),
        };
        self.devices.push(DeviceDef { memory: mloc, device: dev });
        next
    }

}


#[cfg(test)]
mod tests { // {{{
    use super::Computer;
    use device::Device;

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

    struct MockDevice;

    impl Device for MockDevice {
        fn dev_get(&self, pos: u32) -> u8 { return pos as u8; }
        fn dev_set(&mut self, _pos: u32, _data: u8) {}
        fn dev_size(&self) -> u32 { return 0x100; }
    }

    #[test]
    fn new_device() {
        let mut computer = Computer::new(64 * 1024);
        let mock = MockDevice {};
        let n = computer.add_device(Box::new(mock), Some(0xF0001000));
        assert_eq!(n, 0xF0001100);
        assert_eq!(computer.get(0xF0001004), 0x4);
        assert_eq!(computer.get(0xF0001007), 0x7);
    }

    #[test]
    fn offset_addr() {
        let mut computer = Computer::new(64 * 1024);
        computer.set(0xF0000000, 0x64);
        assert_eq!(computer.get(0xF0000000), 0x64);
        assert_eq!(computer.offset, 0x64);
        computer.set(0xF0000001, 0xAB);
        assert_eq!(computer.offset, 0xAB64);
        assert_eq!(computer.get(0xF0000001), 0xAB);
    }

} // }}}
