#!/bin/bash
set -euo pipefail
set -x
cd $(dirname $0)
curl -fLO https://download.opensuse.org/repositories/Cloud:/Images:/Leap_15.3/images/openSUSE-Leap-15.3.x86_64-NoCloud.qcow2.sha256
curl -fLO https://download.opensuse.org/repositories/Cloud:/Images:/Leap_15.3/images/openSUSE-Leap-15.3.x86_64-NoCloud.qcow2
exit 0
