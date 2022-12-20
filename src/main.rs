// use crate::power_control

use std::env;

pub mod client;
pub mod power_control;

fn main() {
    let args: Vec<String> = env::args().collect();
    let address = &args[1];

    let address = String::from(address);

    client::client::client(address,
                           |topic,client| power_control::power_control::set_turned_on_state(topic, client),
                           |msg| power_control::power_control::handle_incoming_message(msg));
}