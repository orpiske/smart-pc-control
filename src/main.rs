// use crate::power_control

use std::env;

pub mod client;

fn main() {
    let args: Vec<String> = env::args().collect();
    let address = &args[1];

    client::run_from_mod2();
    // client::client::run_from_mod();
    let address = String::from(address);
    client::client::client(address);

    println!("Hello, world!");
}
