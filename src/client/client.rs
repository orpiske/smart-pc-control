use std::{process, time, thread};
use std::time::Duration;

use paho_mqtt as mqtt;
use paho_mqtt::{Client, Message};

const QOS_AT_MOST_ONCE: i32 = mqtt::QOS_1;

pub struct SmartContext<'a> {
    pub state_topic: &'a str,
    pub status_topic: &'a str,
    pub last_will: Message,
}

fn clean_disconnect(cli: &Client) {
    cli.disconnect(None).unwrap();
}

pub fn run_client_consumer<F, K>(address: String, context: &SmartContext, on_connect: F, on_message: K) where
    F: FnOnce(&str, &Client) -> paho_mqtt::Result<()>,
    K: FnOnce(&Message) + Copy
{
    let client_id = std::env::var("SMART_PC_CONTROL_ID").unwrap_or(String::from("smart-pc-control"));

    let create_opts = mqtt::CreateOptionsBuilder::new()
        .server_uri(address)
        .client_id(client_id)
        .finalize();

    let cli = mqtt::Client::new(create_opts).unwrap_or_else(|err| {
        eprintln!("Error creating the client: {:?}", err);
        process::exit(1);
    });

    let rx = cli.start_consuming();

    let conn_opts = mqtt::ConnectOptionsBuilder::new()
        .keep_alive_interval(Duration::from_secs(5))
        .clean_session(true)
        .will_message(context.last_will.clone())
        .finalize();

    match cli.connect(conn_opts) {
        Ok(rsp) => {
            if let Some(conn_rsp) = rsp.connect_response() {
                println!(
                    "Connected to: '{}' with MQTT version {}",
                    conn_rsp.server_uri, conn_rsp.mqtt_version
                );

                if conn_rsp.session_present {
                    println!("Client session is already present on broker");
                } else {
                    println!("Subscribing to {0}", context.state_topic);
                    cli.subscribe(context.state_topic, QOS_AT_MOST_ONCE)
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
        }
        Err(e) => {
            println!("Error connecting to the broker: {:?}", e);
            process::exit(1);
        }
    }

    dbg!("Connecting ...");
    if let Err(e) = on_connect(context.status_topic, &cli) {
        println!("Error connecting to the broker: {:?}", e);

        clean_disconnect(&cli);
        process::exit(1);
    }

    dbg!("Waiting for messages");
    for msg in rx.iter() {
        if let Some(msg) = msg {
            on_message(&msg);
        } else if !cli.is_connected() {
            if !try_reconnect(&cli) {
                dbg!("Disconnected ..");
                break;
            }
        }
    }

    dbg!("Disconnecting ...");
    clean_disconnect(&cli);
}

fn try_reconnect(cli: &mqtt::Client) -> bool {
    println!("Connection lost. Waiting to retry connection");
    for _ in 0..30 {
        thread::sleep(Duration::from_millis(5000));
        if cli.reconnect().is_ok() {
            println!("Successfully reconnected");
            return true;
        }
    }

    println!("Unable to reconnect after several attempts");
    false
}

pub fn run_client_producer<F, K>(address: String, topic: &str, on_connect: F, on_produce: K) where
    F: FnOnce(&str, &Client) -> paho_mqtt::Result<()>,
    K: FnOnce(&str) -> Result<Message, &str> + Copy
{
    let cli = mqtt::Client::new(address).unwrap_or_else(|err| {
        println!("Error creating the client: {:?}", err);
        process::exit(1);
    });


    loop {
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
                break;
            }
            Err(e) => {
                println!("Error connecting to the broker: {:?}", e);
                thread::sleep(Duration::from_secs(5))
            }
        }

        if let Err(e) = on_connect(topic, &cli) {
            println!("Error connecting to the broker: {:?}", e);

            clean_disconnect(&cli);
            process::exit(1);
        }
    }

    loop {
        match on_produce(topic) {
            Ok(message) => {
                match cli.publish(message.clone()) {
                    Ok(_) => {
                        println!("Message sent successfully")
                    }
                    Err(error) => {
                        eprintln!("Failed to publish message: {}", error)
                    }
                }
            }
            Err(error) => {
                eprintln!("Failed to create message: {}", error)
            }
        }

        thread::sleep(Duration::from_secs(30))
    }
}



