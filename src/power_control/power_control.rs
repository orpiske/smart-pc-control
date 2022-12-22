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
            if let Err(error) = convert(&mut mac_address, address) {
                eprintln!("{}", error)
            }
        }
        Err(_) => {
            eprintln!("The target mac address is unset. Set SMART_PC_CONTROL_TARGET_MAC_ADDRESS to configure");
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

fn convert(mac_address: &mut [u8; 6], address: String) -> Result<String, &str> {
    let mut i = 0;
    for octet in address.split(':') {
        match u8::from_str_radix(octet, 16) {
            Ok(octet) => {
                mac_address[i] = octet;
                i += 1;
            }
            Err(_) => {
                return Err("Unable to convert value to octet {octet}: {}")
            }
        };
    }

    Ok(String::from(""))
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
                dbg!(std::format!("Received a request with data: {v}"));
                let data: String = v;
                dbg!(std::format!("Received a parseable response as string: {data}"));
                if data.eq("true") {
                    println!("Turning on the remote computer");
                    turn_on();
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

#[test]
fn test_conversion() {
    let expected_mac_address: [u8; 6] = [0x54, 0xb2, 0x03, 0x09, 0x10, 0xd7];
    let mut mac_address: [u8; 6] = [0; 6];

    let address = String::from("54:b2:03:09:10:d7");
    match convert(&mut mac_address, address) {
        Ok(_) => { assert_eq!(mac_address, expected_mac_address) }
        Err(error) => { panic!("{}", error) }
    };


}

#[test]
fn test_conversion_invalid() {
    let mut mac_address: [u8; 6] = [0; 6];

    let address = String::from("54:zz:03:09:10");
    match convert(&mut mac_address, address) {
        Ok(_) => { panic!("This conversion should have failed") }
        Err(error) => {
            assert_eq!(error, "Unable to convert value to octet {octet}: {}")
        }
    };
}