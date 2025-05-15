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

from app.common.constants.resource_type_mapping import SubTypeMapper
from app.common.enums.resource_enum import ResourceSubTypeEnum


class TestSubTypeMapper(unittest.TestCase):

    def test_user_info_from_token(self):
        assert ResourceSubTypeEnum.MysqlInstance in SubTypeMapper.copy_archive_types
        assert ResourceSubTypeEnum.MysqlDatabase in SubTypeMapper.copy_archive_types
        assert ResourceSubTypeEnum.MysqlClusterInstance in SubTypeMapper.copy_archive_types
        assert ResourceSubTypeEnum.PostgreInstance in SubTypeMapper.copy_archive_types
        assert ResourceSubTypeEnum.PostgreClusterInstance in SubTypeMapper.copy_archive_types
        assert ResourceSubTypeEnum.KingBaseInstance in SubTypeMapper.copy_archive_types
        assert ResourceSubTypeEnum.KingBaseClusterInstance in SubTypeMapper.copy_archive_types
        assert ResourceSubTypeEnum.DB2Database in SubTypeMapper.copy_archive_types
        assert ResourceSubTypeEnum.GOLDENDB_CLUSTER_INSTANCE in SubTypeMapper.copy_archive_types
        assert ResourceSubTypeEnum.GAUSSDBT_SINGLE in SubTypeMapper.copy_archive_types
