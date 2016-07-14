use std::time::Duration;
use computer::*;

pub trait Device {
    fn get(&self, pos: u32) -> u8;
    fn set(&mut self, pos: u32, data: u8);
    fn size(&self) -> u32;
    fn step(&mut self, computer: &mut Computer, dt: &Duration);
}
