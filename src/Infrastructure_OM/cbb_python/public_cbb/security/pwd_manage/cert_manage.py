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
#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import os
import ssl

from public_cbb.config.global_config import get_settings
from public_cbb.log.logger import get_logger
from public_cbb.security.pwd_manage.kmc_manage import KmcManage

logger = get_logger()


class CertManage(object):
    valid_ssl_protocol = ('TLSv1.2', 'TLSv1.3')
    not_allowed_ciphers = (
        'AES256-SHA256', 'AES128-SHA256', 'ECDHE-RSA-AES128-SHA256',
        'ECDHE-RSA-AES256-SHA384', 'AES128-GCM-SHA256', 'AES256-GCM-SHA384'
    )

    @classmethod
    def get_cert_pwd(cls, cnf_dir):
        settings = get_settings()
        if cnf_dir == settings.INTERNAL_CNF_DIR:
            return cls.get_internal_cert_pwd(cnf_dir)
        else:
            return cls.get_external_cert_pwd(cnf_dir)

    @classmethod
    def get_internal_cert_pwd(cls, cnf_dir):
        return KmcManage.kmc_decrypt_by_infra(cnf_dir)

    @classmethod
    def get_external_cert_pwd(cls, cnf_dir):
        passwd = cls._get_pwd_when_encrypted(cnf_dir)
        return KmcManage.kmc_decrypt_text_by_infra(passwd)

    @classmethod
    def get_ssl_ciphers(cls):
        ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
        ciphers = ctx.get_ciphers()
        temp = []
        for cipher in ciphers:
            if cipher['protocol'] in cls.valid_ssl_protocol and cipher['name'] not in cls.not_allowed_ciphers:
                temp.append(cipher['name'])
        return ':'.join(temp)

    @classmethod
    def _get_pwd_when_encrypted(cls, cnf_dir):
        if not os.path.isfile(cnf_dir):
            logger.error(f'Not found cnf file in the environment, input path is {cnf_dir}')
            return ''
        with open(cnf_dir, 'r') as f:
            passwd = f.read().strip('\n\"')
        return passwd
