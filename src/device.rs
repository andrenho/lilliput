pub trait Device {
    fn dev_get(&self, pos: u32) -> u8;
    fn dev_set(&mut self, pos: u32, data: u8);
    fn dev_size(&self) -> u32;
}
