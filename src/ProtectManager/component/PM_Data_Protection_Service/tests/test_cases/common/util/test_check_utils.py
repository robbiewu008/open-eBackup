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
import unittest

from app.common.util.check_utils import is_ip_address, is_port, is_domain


class TestCheckUtils(unittest.TestCase):
    def test_return_true_if_ip_is_valid_when_check_is_ip_address(self):
        valid_ipv4_list = ['10.10.10.10', '0.0.0.0', '255.255.255.255']
        for i in valid_ipv4_list:
            self.assertTrue(is_ip_address(i))
        valid_ipv6_list = [
            '0000:0000:0000:0000:0000:0000:0000:0000',
            'ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff',
            '1050:0000:0000:0000:0005:0600:300c:326b',
            '1050:0:0:0:5:600:300c:326b',
            'ff06:0:0:0:0:0:0:c3',
            'ff06::c3',
            '0:0:0:0:0:ffff:192.1.56.10',
            '::ffff:192.1.56.10',
            '::',
            '1::',
            '::1',
            'FEDC:BA98:7654:3210:FEDC:BA98:7654:3210',
        ]
        for i in valid_ipv6_list:
            self.assertTrue(is_ip_address(i))

    def test_return_false_if_ip_is_invalid_when_check_is_ip_address(self):
        valid_ipv4_list = ['10', '30.30.30.256', '100.100.100.', '192.168.0.1.0']
        for i in valid_ipv4_list:
            self.assertFalse(is_ip_address(i))
        valid_ipv6_list = [
            'ffff:ffff:ffff:ffff:ffff:ffff:ffff:fffg',
            '1111:1111:1111:1111:1111:1111:1111:1111:1111',
            '1111:1111:1111:1111:1111:1111:1111',
            'ff06:::c3',
            '::ffff:192.1.56.256',
        ]
        for i in valid_ipv6_list:
            self.assertFalse(is_ip_address(i))

    def test_return_true_if_port_is_valid_when_check_is_port(self):
        valid_port_list = [0, 22, 80, 8088, 65535, '9000']
        for i in valid_port_list:
            self.assertTrue(is_port(i))

    def test_return_false_if_port_is_invalid_when_check_is_port(self):
        invalid_port_list = [50.0, -22, 65536]
        for i in invalid_port_list:
            self.assertFalse(is_port(i))

    def test_return_true_if_domain_is_valid_when_check_is_domain(self):
        valid_domain_list = ['www.huawei.com', 'www.gov.cn', 'apiserver.cluster.local',
                             'pm-resource-manager', f"www.{'a'*63}.com", f"w{'.a'*126}"]
        for i in valid_domain_list:
            self.assertTrue(is_domain(i))

    def test_return_false_if_domain_is_invalid_when_check_is_domain(self):
        invalid_domain_list = ['www._baidu.com', 'www.baidu_.com', 'www.-tencent.com', 'www.tencent-.com',
                               'www.alibaba.', f"www.{'z'*64}.com", f"w{'.a'*127}", "10.10.10.10"]
        for i in invalid_domain_list:
            self.assertFalse(is_domain(i))
