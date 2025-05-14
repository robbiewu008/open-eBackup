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
from unittest import TestCase

from pydantic import ValidationError

from app.protection.object.schemas.extends.params.ad_protection_ext_param import ADProtectionExtParam


class TestADProtectionExtParam(TestCase):
	def test_ad_protection_ext_params_correct(self):
		values = ADProtectionExtParam(
			object_backup=True,
			pre_script="/opt/ss.bat",
			post_script="/opt/ss.bat",
			failed_script="/opt/ss.bat")
		self.assertTrue(True, "无异常抛出")

	def test_ad_protection_ext_params_throws_exception_when_script_invalid(self):
		with self.assertRaises(ValidationError):
			values = ADProtectionExtParam(
				object_backup=True,
				pre_script="/opt/ss.sh",
				post_script="/opt/ss.bat",
				failed_script="/opt/ss.bat")
			self.assertRaises(ValidationError, values)


if __name__ == '__main__':
	unittest.main(verbosity=2)
