#!/bin/bash
set -euo pipefail
set -x
cd $(dirname $0)
URL=https://download.opensuse.org/repositories/Cloud:/Images:/Leap_15.3/images/openSUSE-Leap-15.3.x86_64-NoCloud.qcow2
FNAME="${URL//*\//}"
curl -fLO $URL.sha256
curl -fLO $URL
sha256sum -c $FNAME
exit 0
