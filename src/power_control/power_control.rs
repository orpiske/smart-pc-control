use std::env::VarError;
use paho_mqtt as mqtt;
use paho_mqtt::{Client, Message};
use wake_on_lan as wol;

const QOS_AT_MOST_ONCE: i32 = mqtt::QOS_1;

use system_shutdown::shutdown;

fn turn_off() -> () {
    let default_environment = String::from("development");
    let environment= std::env::var("SMART_PC_CONTROL_ENVIRONMENT").unwrap_or(default_environment);

    if environment.eq("production") {
        match shutdown() {
            Ok(_) => println!("Shutting down, bye!"),
            Err(error) => eprintln!("Failed to shut down: {}", error),
        }
    } else {
        println!("The application is running on {environment} environment where shutdown is not allowed. Set SMART_PC_CONTROL_ENVIRONMENT to 'production' to override")
    }
}

fn turn_on() {
    let mut mac_address: [u8; 6] = [0; 6];

    match std::env::var("SMART_PC_CONTROL_TARGET_MAC_ADDRESS") {
        Ok(address) => {
            let mut i: i8 = 0;
            for octet in address.unwrap().split(':') {
                match octet.parse() {
                    Ok(octet) => {
                        mac_address[i] = octet;
                        i += 1;
                    }
                    Err(error) => {
                        println("Unable to convert value to octet: {}", error);
                        break;
                    }
                };
            }
        }
        Err(_) => {
            eprintln!("The target mac address is unset. Set SMART_PC_CONTROL_TARGET_MAC_ADDRESS to configure")
            return;
        }
    };

    let magic_packet = wol::MagicPacket::new(&mac_address);

    match magic_packet.send() {
        Ok(_) => { println!("Magic packet sent successfully"); }
        Err(error) => {
            println!("Unable to send magic packet: {}", error);
        }
    }
}

pub fn set_turned_on_state(default_topic: &str, cli: &Client) -> paho_mqtt::Result<()> {
    // Create a message and publish it
    let msg = mqtt::MessageBuilder::new()
        .topic(default_topic)
        .payload("true")
        .qos(QOS_AT_MOST_ONCE)
        .finalize();

    cli.publish(msg)
}

pub fn handle_incoming_message(msg: &Option<Message>) {
    if let Some(msg) = msg {
        match msg.payload_str().trim().parse() {
            Ok(v) => {
                dbg!("Received a request with data: {v}");
                let data: String = v;
                dbg!("Received a parseable response as string: {data}");
                if data.eq("true") {
                    println!("The computer is already on");
                } else if data.eq("false") {
                    turn_off();
                    println!("Shutting down the computer ...");
                } else {
                    eprintln!("Invalid request data");
                }
            }
            Err(_) => {
                println!("Failed to parse the response");
            }
        }
    }
}