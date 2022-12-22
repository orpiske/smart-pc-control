// use crate::power_control

extern crate core;

use std::env;

pub mod client;
pub mod power_control;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 3 {
        panic!("Invalid arguments: use {} [power] [mqtt-broker-address]", &args[0]);
    }

    if args[1].eq("power") {
        return power_control();
    }

    panic!("Invalid command: {}", &args[1])
}

fn power_control() {
    let args: Vec<String> = env::args().collect();
    let address = &args[2];

    let address = String::from(address);

    client::client::client(address,
                           |topic,client| power_control::power_control::set_turned_on_state(topic, client),
                           |msg| power_control::power_control::handle_incoming_message(msg));
}