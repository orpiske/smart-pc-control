use paho_mqtt as mqtt;
use paho_mqtt::{Client, Message};

use systemstat::{System, Platform};
const QOS_AT_MOST_ONCE: i32 = mqtt::QOS_1;

pub fn on_init(_: &str, _: &Client) -> paho_mqtt::Result<()> {
    return Ok(());
}


pub fn handle_produce_message(topic: &str) -> Result<Message, &str> {
    let sys = System::new();

    match sys.cpu_temp() {
        Ok(cpu_temp) => {
            println!("CPU temp: {}", cpu_temp);

            Ok(mqtt::MessageBuilder::new()
                .topic(topic)
                .payload(std::format!("{:.2}", cpu_temp))
                .qos(QOS_AT_MOST_ONCE)
                .finalize())
        }
        Err(x) => {
            eprintln!("Unable to collect CPU temperature: {}", x);
            Err("Unable to collect CPU temperature")
        }
    }
}