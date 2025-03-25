# coding: utf-8
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

import sys
import re

def delete_host_entry(ip):
    try:
        pattern = re.compile(ip)
        with open("/etc/hosts", "r") as f:
            lines_origin = f.readlines()
        with open("/etc/hosts", "w") as f:
            lines_new = []
            for line in lines_origin:
                if not re.search(pattern, line):
                    lines_new.append(line)
            f.writelines(lines_new)
        print(f"Delete ip: {ip}")
    except PermissionError:
        print("Permission denied. Please run the script with sufficient privileges.")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 delete_host_entry.py <IP> <DOMAIN>")
        sys.exit(1)

    ip = sys.argv[1]
    delete_host_entry(ip)

