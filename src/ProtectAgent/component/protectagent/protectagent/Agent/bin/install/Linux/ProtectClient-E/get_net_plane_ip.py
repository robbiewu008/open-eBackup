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
