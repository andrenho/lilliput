use std::cell::Ref;
use std::cell::RefCell;
use std::cell::RefMut;
use std::rc::Rc;
use std::time::{Duration, SystemTime};

use device::*;
use cpu::*;

const PHYSICAL_MEMORY_LIMIT: u32 = 0xF0000000;

#[derive(Debug)]
pub enum Command {
    Set8(u32, u8),
    Set16(u32, u16),
    Set32(u32, u32),
}

struct MemoryLocation {
    location: u32,
    size: u32,
}

pub struct DeviceDef {
    memory: Option<MemoryLocation>,
    device: Rc<RefCell<Device>>,
}

pub struct Computer {
    physical_memory : Vec<u8>,
    offset:           u32,
    devices:          Vec<DeviceDef>,
    cpu:              Option<Rc<RefCell<CPU>>>,
    last_time:        SystemTime,
}


impl Computer {

    pub fn new(mem_size: u32) -> Computer {
        Computer {
            physical_memory: vec![0u8; mem_size as usize],
            offset: 0x0,
            devices: vec![],
            cpu: None,
            last_time: SystemTime::now(),
        }
    }

    #[inline]
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
                        return dev.device.borrow().get(pos - loc.location);
                    },
                    None => (),
                }
            }
            panic!("Getting an invalid memory position.");
        }
    }

    #[inline]
    pub fn get16(&self, pos: u32) -> u16 {
        let b1 = self.get(pos) as u16;
        let b2 = self.get(pos+1) as u16;
        b1 | (b2 << 8)
    }

    #[inline]
    pub fn get32(&self, pos: u32) -> u32 {
        let b1 = self.get(pos) as u32;
        let b2 = self.get(pos+1) as u32;
        let b3 = self.get(pos+2) as u32;
        let b4 = self.get(pos+3) as u32;
        b1 | (b2 << 8) | (b3 << 16) | (b4 << 24)
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
                        dev.device.borrow_mut().set(pos - loc.location, data);
                        return;
                    },
                    None => (),
                }
            }
            panic!("Setting an invalid memory position.");
        }
    }

    pub fn set16(&mut self, pos: u32, data: u16) {
        self.set(pos, data as u8);
        self.set(pos+1, (data >> 8) as u8);
    }

    pub fn set32(&mut self, pos: u32, data: u32) {
        self.set(pos, data as u8);
        self.set(pos+1, (data >> 8) as u8);
        self.set(pos+2, (data >> 16) as u8);
        self.set(pos+3, (data >> 24) as u8);
    }

    pub fn cpu(&self) -> Ref<CPU> {
        self.cpu.as_ref().unwrap().borrow()
    }

    pub fn cpu_mut(&mut self) -> RefMut<CPU> {
        self.cpu.as_mut().unwrap().borrow_mut()
    }

    pub fn set_cpu(&mut self, cpu: CPU, pos: u32) -> u32 {
        let rcpu = Rc::new(RefCell::new(cpu));
        self.cpu = Some(rcpu.clone());
        self.add_device(rcpu, Some(pos))
    }

    fn add_device(&mut self, dev: Rc<RefCell<Device>>, memory_pos: Option<u32>) -> u32 {
        let (next, mloc) = match memory_pos {
            Some(addr) => {
                if addr < PHYSICAL_MEMORY_LIMIT + 0x100 {
                    panic!("Memory position must be above PHYSICAL_MEMORY_LIMIT.");
                }
                let next = addr + dev.borrow().size();
                let memloc = MemoryLocation { location: addr, size: dev.borrow().size() };
                (next, Some(memloc))
            },
            None => (0, None),
        };
        self.devices.push(DeviceDef { memory: mloc, device: dev });
        next
    }

    pub fn step(&mut self) {
        let now = SystemTime::now();
        let frame_time = match now.duration_since(self.last_time) {
            Ok(v)  => v,
            Err(_) => Duration::new(1, 0)
        };
        self.last_time = now;

        let mut cmds : Vec<Command> = vec![];
        for dev in &self.devices {
            dev.device.borrow_mut().step(&self, &frame_time, &mut cmds);
        }

        for cmd in cmds {
            // println!("{:?}", cmd);
            match cmd {
                Command::Set8(pos, data)  => self.set(pos, data),
                Command::Set16(pos, data) => self.set16(pos, data),
                Command::Set32(pos, data) => self.set32(pos, data),
            }
        }
    }
}


#[cfg(test)]
mod tests { // {{{
    use std::rc::Rc;
    use std::cell::RefCell;
    use std::time::Duration;

    use super::*;
    use device::Device;

    #[test]
    fn physical_memory() {
        let mut computer = Computer::new(64 * 1024);
        computer.set(0x12, 0xAF);
        assert_eq!(computer.get(0x12), 0xAF);
    }

    #[test]
    fn bytes() {
        let mut computer = Computer::new(64 * 1024);
        computer.set32(0x0, 0x12345678);
        assert_eq!(computer.get(0x0), 0x78);
        assert_eq!(computer.get(0x1), 0x56);
        assert_eq!(computer.get(0x2), 0x34);
        assert_eq!(computer.get(0x3), 0x12);
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
        fn get(&self, pos: u32) -> u8 { return pos as u8; }
        fn set(&mut self, _pos: u32, _data: u8) {}
        fn size(&self) -> u32 { return 0x100; }
        fn step(&mut self, _computer: &Computer, _dt: &Duration, _cmds: &mut Vec<Command>) {}
    }

    #[test]
    fn new_device() {
        let mut computer = Computer::new(64 * 1024);
        let mock = MockDevice {};
        let n = computer.add_device(Rc::new(RefCell::new(mock)), Some(0xF0001000));
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
