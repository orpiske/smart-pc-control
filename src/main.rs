extern crate core;

use std::env;

pub mod client;
pub mod power_control;
pub mod temperature_sensor;

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() != 3 {
        panic!("Invalid arguments: use {} [power] [mqtt-broker-address]", &args[0]);
    }

    if args[1].eq("power") {
        return power_control();
    }

    if args[1].eq("temperature-sensor") {
        return temperature_sensor();
    }

    panic!("Invalid command: {}", &args[1])
}

fn power_control() {
    let args: Vec<String> = env::args().collect();
    let address = &args[2];

    let address = String::from(address);
    let topic = "pc/nuc/status/on";

    client::client::run_client_consumer(address, topic,
                                        |topic,client| power_control::power_control::set_turned_on_state(topic, client),
                                        |msg| power_control::power_control::handle_incoming_message(msg));
}


fn temperature_sensor() {
    let args: Vec<String> = env::args().collect();
    let address = &args[2];

    let address = String::from(address);

    let topic = "pc/nuc/temperature/cpu";

    client::client::run_client_producer(address, topic,
                                        |topic,client| temperature_sensor::temperature_sensor::on_init(topic, client),
                                        |topic| temperature_sensor::temperature_sensor::handle_produce_message(topic));
}