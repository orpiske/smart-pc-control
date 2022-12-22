build:
	cargo build --release

install:
	install -D -m755 target/release/smart-pc-control /usr/bin/smart-pc-control
	install -D -m644 src/power_control/config/smart-pc-control-power.sh /etc/sysconfig/smart-pc-control-power.sh
	install -D -m644 src/power_control/config/smart-pc-control-power.service /usr/lib/systemd/system/smart-pc-control-power.service

uninstall:
	[[ -f /usr/bin/smart-pc-control ]] && rm -f /usr/bin/smart-pc-control
	[[ -f /etc/sysconfig/smart-pc-control-power.sh ]] && rm -f /etc/sysconfig/smart-pc-control-power.sh
	[[ -f /usr/lib/systemd/system/smart-pc-control-power.service ]] && rm -f /usr/lib/systemd/system/smart-pc-control-power.service