use std::process;
use std::time::Duration;

use paho_mqtt as mqtt;
use paho_mqtt::{Client, Message};

// const DEFAULT_QOS: i32 = 1;
const QOS_AT_MOST_ONCE: i32 = mqtt::QOS_1;

fn clean_disconnect(cli: &Client) {
    // Disconnect from the broker
    cli.disconnect(None).unwrap();
}

pub fn client<F, K>(address: String, on_connect: F, on_message: K) where
    F: FnOnce(&str, &Client) -> paho_mqtt::Result<()>,
    K: FnOnce(&Option<Message>) + Copy
{
    let default_topic = "pc/nuc/status/on";

    let cli = mqtt::Client::new(address).unwrap_or_else(|err| {
        println!("Error creating the client: {:?}", err);
        process::exit(1);
    });

    let rx = cli.start_consuming();

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

                println!("Subscribing to {default_topic}");
                cli.subscribe(default_topic, QOS_AT_MOST_ONCE)
                    .and_then(|rsp| {
                        rsp.subscribe_response().ok_or(mqtt::Error::General("Unable to subscribe due to Bad MQTT response"))
                    })
                    .unwrap_or_else(|err| {
                        println!("Unable to subscribe to topic: {:?}", err);
                        clean_disconnect(&cli);
                        process::exit(1);
                    });

            }
        }
        Err(e) => {
            println!("Error connecting to the broker: {:?}", e);
            process::exit(1);
        }
    }

    if let Err(e) = on_connect(default_topic, &cli) {
        println!("Error connecting to the broker: {:?}", e);

        clean_disconnect(&cli);
        process::exit(1);
    }

    for msg in rx.iter() {
        on_message(&msg);
        // handle_incoming_message(msg);
    }

    clean_disconnect(&cli);
}



