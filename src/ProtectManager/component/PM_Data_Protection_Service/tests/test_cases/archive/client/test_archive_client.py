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
import unittest
from http import HTTPStatus
from unittest import mock
from unittest.mock import Mock

from urllib3.response import HTTPResponse

from app.copy_catalog.models.req_param import NoArchiveReq
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.common.events import mock_producer  # noqa

sys.modules['app.common.logger'] = mock.Mock()
mock.patch("app.common.security.kmc_util._build_kmc_handler", Mock(return_value=None)).start()
mock.patch("app.common.clients.client_util._build_ssl_context", Mock(return_value=None)).start()

okResponse = HTTPResponse()
okResponse.status = HTTPStatus.OK

errorResponse = HTTPResponse()
errorResponse.status = HTTPStatus.INTERNAL_SERVER_ERROR


class TestArchiveClient(unittest.TestCase):

    # create_task
    @mock.patch("app.common.clients.client_util.SystemBaseHttpsClient.request",
                Mock(return_value=okResponse))
    def test_create_task_OK(self):
        from app.archive.client.archive_client import ArchiveClient
        ArchiveClient.create_task("copy_id", "policy", "resource_sub_type", "resource_type")
        assert True

    @mock.patch("app.common.clients.client_util.SystemBaseHttpsClient.request",
                Mock(return_value=errorResponse))
    def test_create_task_error(self):
        from app.archive.client.archive_client import ArchiveClient
        from app.common.exception.unified_exception import EmeiStorBizException

        with self.assertRaises(EmeiStorBizException) as ex:
            ArchiveClient.create_task("copy_id", "policy", "resource_sub_type", "resource_type")

    # update_copy_status
    @mock.patch("app.common.clients.client_util.ProtectionServiceHttpsClient.request",
                Mock(return_value=okResponse))
    def test_update_copy_status_OK(self):
        from app.archive.client.archive_client import ArchiveClient
        update_res = ArchiveClient.update_copy_status({"uuid": "uuid", "status": 1})
        self.assertEqual(update_res, {})

    @mock.patch("app.common.clients.client_util.ProtectionServiceHttpsClient.request",
                Mock(return_value=errorResponse))
    def test_update_copy_status_error(self):
        from app.archive.client.archive_client import ArchiveClient

        update_res = ArchiveClient.update_copy_status({"uuid": "uuid", "status": 1})
        self.assertIsNone(update_res)

    # get_no_archive_copy_list
    @mock.patch("app.common.clients.client_util.ProtectionServiceHttpsClient.request",
                Mock(return_value=okResponse))
    def test_get_no_archive_copy_list_OK(self):
        from app.archive.client.archive_client import ArchiveClient
        req = NoArchiveReq(resource_id="123")
        get_res = ArchiveClient.get_no_archive_copy_list(req)
        self.assertEqual(get_res, {})

    @mock.patch("app.common.clients.client_util.ProtectionServiceHttpsClient.request",
                Mock(return_value=errorResponse))
    def test_get_no_archive_copy_list_error(self):
        from app.archive.client.archive_client import ArchiveClient
        req = NoArchiveReq(resource_id="123")
        get_res = ArchiveClient.get_no_archive_copy_list(req)
        self.assertIsNone(get_res)


    # create_copy_archive_map
    @mock.patch("app.common.clients.client_util.ProtectionServiceHttpsClient.request",
                Mock(return_value=okResponse))
    def test_create_copy_archive_map_OK(self):
        from app.archive.client.archive_client import ArchiveClient
        ArchiveClient.create_copy_archive_map("resource_id","storage_id", "copy_id")
        assert True

    @mock.patch("app.common.clients.client_util.ProtectionServiceHttpsClient.request",
                Mock(return_value=errorResponse))
    def test_create_copy_archive_map_error(self):
        from app.archive.client.archive_client import ArchiveClient

        ArchiveClient.create_copy_archive_map("resource_id","storage_id", "copy_id")
        assert True


    # query_resource
    @mock.patch("app.common.clients.client_util.ProtectionServiceHttpsClient.request",
                Mock(return_value=okResponse))
    def test_query_resource_OK(self):
        from app.archive.client.archive_client import ArchiveClient
        ArchiveClient.query_resource("resource_id")
        assert True

    @mock.patch("app.common.clients.client_util.ProtectionServiceHttpsClient.request",
                Mock(return_value=errorResponse))
    def test_query_resource_error(self):
        from app.archive.client.archive_client import ArchiveClient
        from app.common.exception.unified_exception import EmeiStorBizException

        with self.assertRaises(EmeiStorBizException):
            ArchiveClient.query_resource("resource_id")


    # dispatch_archive
    @mock.patch("app.common.clients.client_util.SystemBaseHttpsClient.request",
                Mock(return_value=okResponse))
    def test_dispatch_archive_OK(self):
        from app.archive.client.archive_client import ArchiveClient
        ArchiveClient.dispatch_archive("resource_id")
        assert True

    @mock.patch("app.common.clients.client_util.SystemBaseHttpsClient.request",
                Mock(return_value=errorResponse))
    def test_dispatch_archive_error(self):
        from app.archive.client.archive_client import ArchiveClient
        from app.common.exception.unified_exception import EmeiStorBizException

        with self.assertRaises(EmeiStorBizException):
            ArchiveClient.dispatch_archive("resource_id")

    # query_media_set_info
    @mock.patch("app.common.clients.client_util.SystemBaseHttpsClient.request",
                Mock(return_value=okResponse))
    def test_query_media_set_info_OK(self):
        from app.archive.client.archive_client import ArchiveClient
        media_set_info = ArchiveClient.query_media_set_info("resource_id")
        self.assertEqual(media_set_info, {})

    @mock.patch("app.common.clients.client_util.SystemBaseHttpsClient.request",
                Mock(return_value=errorResponse))
    def test_query_media_set_info_error(self):
        from app.archive.client.archive_client import ArchiveClient

        media_set_info = ArchiveClient.query_media_set_info("resource_id")
        self.assertIsNone(media_set_info)

