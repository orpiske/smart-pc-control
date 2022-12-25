Smart PC Control: utilities to control a remote PC via HomeKit
============

Build Status
----
Fedora (GRU Testing): [![Fedora COPR (Testing Repo) Status](https://copr.fedorainfracloud.org/coprs/orpiske/orp-tools-testing/package/smart-pc-control/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/orpiske/orp-tools-testing/package/smart-pc-control/)


----
You can find the old version written in C in the [c-version](https://github.com/orpiske/smart-pc-control/tree/c-version) branch.

Installation:
----

The code can be installed via Fedora COPR. Packages are available for CentOS 6, 7, Fedora 33
or greater and RHEL 6 and 7. For CentOS 6 and RHEL 6, please use the legacy repos (check the notes below).

**Testing**

1. Enable my testing COPR.

```dnf copr enable orpiske/orp-tools-testing```

2. Install the runtime only:
```dnf install -y smart-pc-control```

Packaging
----
Steps from: https://docs.fedoraproject.org/en-US/quick-docs/publish-rpm-on-copr/


```
tito build --test
tito tag && git push --follow-tags origin
```

Upgrading
---

If you are using Fedora, you can skip these steps. If you are using Raspbian, then you have to uninstall the old version in C.

On the directory you had the version in C, you can use the `uninstall` target to remove the C binaries:

```shell
sudo systemctl stop smart-pc-control-power@pi.service
sudo make uninstall
```

Then, you can compile and install manually

```shell
make build && sudo make install
```


Configuration:
----

First set up the infrastructure required to integrate with HomeKit:

* A MQTT broker
* Homekit2mqtt

I wrote about these steps in [my blog](https://www.orpiske.net/2019/11/controlling-a-pc-via-apple-homekit/).

Then, configure the daemon on the host you want to shut down (the one that will be turned off when you flip the switch on the iPhone/iPad). To do so edit the file `/etc/sysconfig/smart-pc-control-power.sh`.

*Note*: you can use the template file installed at `/etc/sysconfig/smart-pc-control-power.sh.set-me-up` and then save it as `/etc/sysconfig/smart-pc-control-power.sh`.

For this host you only need to set the address of the MQTT broker:

```shell
# Set the
MQTT_BROKER_URL=tcp://thyone:1883

SMART_PC_CONTROL_ENVIRONMENT="production"

# Use a unique ID for each client connecting
SMART_PC_CONTROL_ID="smart-pc-control-stateful"

# Uncomment for clients that don't store any state
# SMART_PC_CONTROL_STATELESS="false"

# Ensure to export library dir if not using a standard location
# SMART_PC_CONTROL_TARGET_MAC_ADDRESS="54:b2:03:09:10:d7"
```

Then on the host that will send the magic packet for the wake-on-lan, configure the file like this:

```shell
# Set the
MQTT_BROKER_URL=tcp://thyone:1883

SMART_PC_CONTROL_ENVIRONMENT="production"

# Use a unique ID for each client connecting
SMART_PC_CONTROL_ID="smart-pc-control-stateless"

# Uncomment for clients that don't store any state
SMART_PC_CONTROL_STATELESS="true"

# Ensure to export library dir if not using a standard location
SMART_PC_CONTROL_TARGET_MAC_ADDRESS="00:00:00:00:00:00"
```


