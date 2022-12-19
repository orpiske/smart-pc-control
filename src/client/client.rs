use std::process;
use std::time::Duration;

use paho_mqtt as mqtt;

const DEFAULT_QOS: i32 = 1;

pub fn client(address: String) {
    let default_topic = "pc/nuc/status/on";

    let cli = mqtt::Client::new(address).unwrap_or_else(|err| {
        println!("Error creating the client: {:?}", err);
        process::exit(1);
    });

    let conn_opts = mqtt::ConnectOptionsBuilder::new()
        .keep_alive_interval(Duration::from_secs(20))
        .clean_session(true)
        .finalize();

    match cli.connect(conn_opts) {
        Ok(rsp) => {
            if let Some(conn_rsp) = rsp.connect_response() {
                println!(
                    "Connected to: '{}' with MQTT version {}",
                    conn_rsp.server_uri, conn_rsp.mqtt_version
                );
            }
        }
        Err(e) => {
            println!("Error connecting to the broker: {:?}", e);
            process::exit(1);
        }
    }

    // Create a message and publish it
    let msg = mqtt::MessageBuilder::new()
        .topic(default_topic)
        .payload("true")
        .qos(DEFAULT_QOS)
        .finalize();


    if let Err(e) = cli.publish(msg) {
        println!("Error sending message: {:?}", e);
    }

    // Disconnect from the broker
    cli.disconnect(None).unwrap();
}