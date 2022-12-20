use std::process;
use std::time::Duration;

use paho_mqtt as mqtt;
use paho_mqtt::Client;

const DEFAULT_QOS: i32 = 1;
const QOS_AT_MOST_ONCE: i32 = mqtt::QOS_1;

fn clean_disconnect(cli: &Client) {
    // Disconnect from the broker
    cli.disconnect(None).unwrap();
}

fn set_turned_on_state(default_topic: &str, cli: &Client) {
    // Create a message and publish it
    let msg = mqtt::MessageBuilder::new()
        .topic(default_topic)
        .payload("true")
        .qos(DEFAULT_QOS)
        .finalize();

    if let Err(e) = cli.publish(msg) {
        println!("Error sending message: {:?}", e);

        clean_disconnect(&cli);
        process::exit(1);
    }
}

pub fn client(address: String) {
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

    set_turned_on_state(default_topic, &cli);

    for msg in rx.iter() {
        if let Some(msg) = msg {
            match msg.payload_str().trim().parse() {
                Ok(v) => {
                    dbg!("Received a request with data: {v}");
                    let data: String = v;
                    dbg!("Received a parseable response as string: {data}");
                    if data.eq("true") {
                        println!("The computer is already on");
                    } else if data.eq("false") {
                        println!("Shutting down the computer ...");
                    } else {
                        println!("Invalid request data");
                    }
                }
                Err(_) => {
                    println!("Failed to parse the response");
                    continue;
                }
            }
        }
    }

    clean_disconnect(&cli);
}

