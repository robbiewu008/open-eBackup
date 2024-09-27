#!/bin/bash

set -e

if ! timeout 10s kubectl cluster-info >/dev/null; then
    # cluster not exist
    exit 0
fi

if helm status dataprotect >/dev/null; then
    helm uninstall dataprotect
fi

sleep 10