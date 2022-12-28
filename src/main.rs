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

    let context = client::client::SmartContext {
        status_topic: power_control::power_control::STATUS_TOPIC,
        state_topic: power_control::power_control::STATE_TOPIC,
        last_will: power_control::power_control::last_will_message(),
    };

    client::client::run_client_consumer(address, &context,
                                        |state_topic,client| power_control::power_control::set_turned_on_state(state_topic, client),
                                        |msg| power_control::power_control::handle_incoming_message(msg));
}