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

import json
import ssl
from urllib import request, error
from public_cbb.config.global_config import get_settings
from public_cbb.log.logger import get_logger
from public_cbb.security.pwd_manage.pwd_manage import clear
from public_cbb.security.pwd_manage.kmc_util import Kmc, read_file_by_utf_8
from public_cbb.security.anonym_utils.anonymity import Anonymity
from public_cbb.utils.decorator import retry

logger = get_logger()
PM_HTTP_TIME_OUT = 30


class KmcManage(object):
    @classmethod
    def kmc_encrypt_by_pm(cls, passwd):
        settings = get_settings()
        url_prefix = 'https' if settings.OPEN_CERT_VERIFY else 'http'
        url = f'{url_prefix}://pm-system-base:30081/v1/kms/encrypt'
        data = json.dumps({
            'plaintext': passwd
        })
        header = {"content-type": "application/json"}
        req = request.Request(url=url, data=bytes(data, 'utf8'), method='POST', headers=header)
        key_pass = ''
        if settings.OPEN_CERT_VERIFY:
            key_pass = cls.kmc_decrypt_by_infra(settings.INTERNAL_CNF_DIR)
            context = cls.get_ssl_context(settings.INTERNAL_CA_DIR, settings.INTERNAL_CERT_DIR,
                                          settings.INTERNAL_KEY_DIR, key_pass)
        else:
            context = cls.get_unverified_ssl_context()
        if context is None:
            logger.error(f'Get ssl context failed, url:{url}')
            return ''
        try:
            data = cls.send_request_to_pm(req, context)
            if data.get('ciphertext') is None:
                logger.error(f'Kmc encrypt response value incorrect')
                return ''
            return data['ciphertext']
        except error.HTTPError as e:
            logger.error(f'Kmc encrypt by pm failed, code:{e.code}, e:{Anonymity.process(str(e.reason))}')
            return ''
        except error.URLError as e:
            logger.error(f'Kmc encrypt by pm failed, e:{Anonymity.process(str(e.reason))}')
            return ''
        finally:
            clear(key_pass)

    @classmethod
    def kmc_decrypt_by_pm(cls, passwd):
        settings = get_settings()
        url_prefix = 'https' if settings.OPEN_CERT_VERIFY else 'http'
        url = f'{url_prefix}://pm-system-base:30081/v1/kms/decrypt'
        data = json.dumps({
            'ciphertext': passwd
        })
        header = {"content-type": "application/json"}
        req = request.Request(url=url, data=bytes(data, 'utf8'), method='POST', headers=header)
        key_pass = ''
        if settings.OPEN_CERT_VERIFY:
            key_pass = cls.kmc_decrypt_by_infra(settings.INTERNAL_CNF_DIR)
            context = cls.get_ssl_context(settings.INTERNAL_CA_DIR, settings.INTERNAL_CERT_DIR,
                                          settings.INTERNAL_KEY_DIR, key_pass)
        else:
            context = cls.get_unverified_ssl_context()
        if context is None:
            logger.error(f'Get ssl context failed, url:{url}')
            return ''
        try:
            data = cls.send_request_to_pm(req, context)
            if data.get('plaintext') is None:
                logger.error(f'Kmc decrypt response value incorrect')
                return ''
            return data['plaintext']
        except error.HTTPError as e:
            logger.error(f'Kmc decrypt by pm failed, code:{e.code}, e:{Anonymity.process(str(e.reason))}')
            return ''
        except error.URLError as e:
            logger.error(f'Kmc decrypt by pm failed, e:{Anonymity.process(str(e.reason))}')
            return ''
        finally:
            clear(key_pass)

    @classmethod
    @retry((error.HTTPError, error.URLError), get_settings().PM_HTTP_RETRY_TIMES, get_settings().PM_HTTP_RETRY_INTERVAL)
    def send_request_to_pm(cls, req, context):
        response = request.urlopen(req, timeout=PM_HTTP_TIME_OUT, context=context)
        data = json.loads(response.read().decode('utf-8'))
        return data

    @classmethod
    def kmc_decrypt_by_infra(cls, cnf_dir):
        settings = get_settings()
        try:
            res = Kmc().decrypt(read_file_by_utf_8(settings.INTERNAL_CNF_DIR))
            return res
        except Exception as e:
            logger.error(f'Kmc decrypt by infra failed, e:{Anonymity.process(str(e))}')
            return ''
        finally:
            pass

    @classmethod
    def kmc_decrypt_text_by_infra(cls, cipher_text):
        try:
            res = Kmc().decrypt(cipher_text)
            return res
        except Exception as exp:
            logger.error(f'Kmc decrypt text by infra failed, e:{Anonymity.process(str(exp))}')
            return ''

    @classmethod
    def kmc_encrypt_text_by_infra(cls, plain_text):
        try:
            res = Kmc().encrypt(plain_text)
            return res
        except Exception as exp:
            logger.error(f'Kmc encrypt text by infra failed, e:{Anonymity.process(str(exp))}')
            return ''

    @classmethod
    def get_ssl_context(cls, ca_dir, cert_dir, key_dir, key_passwd):
        try:
            context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
            context.verify_mode = ssl.CERT_REQUIRED
            context.load_cert_chain(cert_dir, key_dir, key_passwd)
            context.load_verify_locations(ca_dir)
            return context
        except Exception as e:
            logger.error(f'Get ssl context failed, e:{Anonymity.process(str(e))}')
            return ''
        finally:
            pass

    @classmethod
    def get_unverified_ssl_context(cls):
        context = ssl.create_default_context()
        context.check_hostname = False
        context.verify_mode = ssl.CERT_NONE
        return context
