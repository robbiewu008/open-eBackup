#
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
#
#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import sys
import json


def get_net_plane_ip(param, default):
    data = json.loads(json.loads(param))
    net_ip = ''
    for item in data:
        if item.get('name') == f'default/{default}':
            ips = item.get('ips')
            net_ip = ips[0]
            break
    return net_ip


if __name__ == '__main__':
    data = sys.argv[1]
    default = sys.argv[2]
    net_ip = get_net_plane_ip(data, default)
    print(net_ip)
