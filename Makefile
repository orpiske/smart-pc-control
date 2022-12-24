build:
	cargo build --release

install:
	install -D -m755 target/release/smart-pc-control /usr/bin/smart-pc-control
	install -D -m644 src/power_control/config/smart-pc-control-power.sh /etc/sysconfig/smart-pc-control-power.sh.set-me-up
	install -D -m644 src/power_control/config/smart-pc-control-power.service /usr/lib/systemd/system/smart-pc-control-power.service
	sudo systemctl daemon-reload

uninstall:
	[[ -f /usr/bin/smart-pc-control ]] && rm -f /usr/bin/smart-pc-control
	[[ -f /etc/sysconfig/smart-pc-control-power.sh.set-me-up ]] && rm -f /etc/sysconfig/smart-pc-control-power.sh.set-me-up
	[[ -f /usr/lib/systemd/system/smart-pc-control-power.service ]] && rm -f /usr/lib/systemd/system/smart-pc-control-power.service