Smart PC Control: utilities to control a remote PC via HomeKit
============

Build Status
----
Fedora (GRU Testing): [![Fedora COPR (Testing Repo) Status](https://copr.fedorainfracloud.org/coprs/orpiske/orp-tools-testing/package/smart-pc-control/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/orpiske/orp-tools-testing/package/smart-pc-control/)


nstallation:
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
