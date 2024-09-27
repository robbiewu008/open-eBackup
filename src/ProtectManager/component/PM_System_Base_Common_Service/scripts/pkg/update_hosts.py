# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# coding: utf-8
import sys

def add_host_entry(ip, domain):
    entry = f"{ip} {domain}\n"
    try:
        with open("/etc/hosts", "a") as file:
            file.write(entry)
        print(f"Added entry: {entry}")
    except PermissionError:
        print("Permission denied. Please run the script with sufficient privileges.")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 add_host_entry.py <IP> <DOMAIN>")
        sys.exit(1)

    ip = sys.argv[1]
    domain = sys.argv[2]
    add_host_entry(ip, domain)
