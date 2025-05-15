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
from logging import LogRecord
from unittest import TestCase

from app.common.sensitive.log_filter import LogFilter


class TestLogFilter(TestCase):

    def test_filter_given_log_not_contains_sensitive_word(self):
        msg = "send schedule message, params={\"uuid\":\"8be715c4-e15c-4a6d-b642-256cbb0e3eb1\"," \
              "\"timestamp\":1647401522034,\"msg_id\":\"08a8fba6-d7b4-4eee-9cfe-782acc08b203\"," \
              "\"request_id\":\"0c00cddb-53fb-42a3-af26-b044ac9bba07\"," \
              "\"default_publish_topic\":\"Sanning_environment_v2\"} "
        self.assertTrue(LogFilter().filter(self.build_record(msg)))

    def test_filter_given_log_contains_sensitive_word(self):
        log_filter = LogFilter()
        self.assertTrue(log_filter.filter(self.build_record("get aka testak fdfak1 ussss test")))
        self.assertTrue(log_filter.filter(self.build_record("get aka testaiv fdivk1 ussss test")))
        self.assertTrue(log_filter.filter(self.build_record("get aka mkak fdfamk sdsmksss test")))
        self.assertFalse(log_filter.filter(self.build_record("i don't have password")))
        self.assertFalse(log_filter.filter(self.build_record("i don't have pass!")))
        self.assertFalse(log_filter.filter(self.build_record("i don't have,pwd")))
        self.assertFalse(log_filter.filter(self.build_record("i don't have!key")))
        self.assertFalse(log_filter.filter(self.build_record("i don't have # crypto.")))
        self.assertFalse(log_filter.filter(self.build_record("i don't have. have [session] test again")))
        self.assertFalse(log_filter.filter(self.build_record("token: 12324343 is create")))
        self.assertFalse(log_filter.filter(self.build_record("fingerprint[0e99r8ejr82fds] is success")))
        self.assertFalse(log_filter.filter(self.build_record("generate auth info [huawei@123]")))
        self.assertFalse(log_filter.filter(self.build_record("enc[aaaaa] and dec is useraaaa")))
        self.assertFalse(log_filter.filter(self.build_record("tgt is userabca")))
        self.assertFalse(log_filter.filter(self.build_record("get iqn from xxx, value is abc1")))
        self.assertFalse(log_filter.filter(self.build_record("initiator")))
        self.assertFalse(log_filter.filter(self.build_record("init secret xxxxx success")))
        self.assertFalse(log_filter.filter(self.build_record("check cert failed, cert name is [dsdsdsds]")))
        self.assertFalse(log_filter.filter(self.build_record("use salt, value is xxxxxx")))
        self.assertFalse(log_filter.filter(self.build_record("this is a private info, value is not support")))
        self.assertFalse(log_filter.filter(self.build_record("user login success, verfiycode is abcdsds")))
        self.assertFalse(log_filter.filter(self.build_record("user email is abc@aacfuf.com")))
        self.assertFalse(log_filter.filter(self.build_record("my phone number is 136404038543")))
        self.assertFalse(log_filter.filter(self.build_record("have a rand value")))
        self.assertFalse(log_filter.filter(self.build_record("check safe failed")))
        self.assertFalse(log_filter.filter(self.build_record("user_info is user info")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [PKCS1]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [base64]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [AES128]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [AES256]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [RSA]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [SHA1]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [SHA256]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [SHA384]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [SHA512]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [algorithm]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [AccountNumber]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [bank]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [cvv]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [checkno]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [mima]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [CardPinNumber]")))
        self.assertFalse(log_filter.filter(self.build_record("check text failed, value is [IDNumber]")))
        self.assertFalse(log_filter.filter(self.build_record("get ak from db, is 9d8f9d822hjh")))
        self.assertFalse(log_filter.filter(self.build_record("get !ak from db, is 9d8f9d822hjh")))
        self.assertFalse(log_filter.filter(self.build_record("get %ak@ from db, is 9d8f9d822hjh")))
        self.assertFalse(log_filter.filter(self.build_record("get iv from db, is 9d8f9d822hjh")))
        self.assertFalse(log_filter.filter(self.build_record("get !iv from db, is 9d8f9d822hjh")))
        self.assertFalse(log_filter.filter(self.build_record("get %iv@ from db, is 9d8f9d822hjh")))
        self.assertFalse(log_filter.filter(self.build_record("get mk from db, is 9d8f9d822hjh")))
        self.assertFalse(log_filter.filter(self.build_record("get !mk from db, is 9d8f9d822hjh")))
        self.assertFalse(log_filter.filter(self.build_record("get %mk@ from db, is 9d8f9d822hjh")))
        self.assertFalse(log_filter.filter(self.build_record("get sk from db, is 9d8f9d822hjh")))
        self.assertFalse(log_filter.filter(self.build_record("get !sk from db, is 9d8f9d822hjh")))
        self.assertFalse(log_filter.filter(self.build_record("get %sk@ from db, is 9d8f9d822hjh")))

    @staticmethod
    def build_record(msg):
        return LogRecord("", 1, "", 0, msg, (), None)
