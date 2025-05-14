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
import time
import uuid
import unittest
from unittest import mock
import sys
from unittest.mock import Mock
from tests.test_cases import common_mocker # noqa
from tests.test_cases.tools import http, env, redis_mock # noqa
from app.common.security.kmc_util import Kmc
mock.patch("app.common.security.kmc_util._build_kmc_handler", Mock(return_value=None)).start()
mock.patch.object(Kmc, "decrypt", Mock(return_value=None)).start()

from tests.test_cases.backup.common.context import mock_context # noqa

from sqlalchemy import create_engine

sys.modules['app.common.config'] = mock.Mock()
from app.resource_lock.db import db_utils


def _mock_get_db_engine(bring_up_db=True):
    engine = create_engine("sqlite:///:memory:")
    if bring_up_db:
        db_utils.setup_db_and_tables_if_needed(engine)
    return engine

mock.patch.object(db_utils, "get_db_engine", mock.Mock(return_value=_mock_get_db_engine())).start()

try:
    from app.resource_lock.service import async_lock_decorator
    from app.resource_lock.common import consts
    from app.resource_lock.kafka import messaging_utils
    from app.common.enums import job_enum
except Exception:
    pass

msg_info = dict()
job_log_level = dict()


def mock_produce(msg, topic):
    if topic in msg_info.keys():
        raise Exception(topic)
    msg_info[topic] = msg


def mock_job_log(request_id, job_id, req):
    job_log_level[request_id] = req


@mock.patch('app.common.events.producer.produce', mock_produce)
@mock.patch('app.common.toolkit.modify_task_log', mock_job_log)
class TestResourceLock(unittest.TestCase):
    @unittest.skip
    def test_resource_lock_success(self):
        """
        添加资源读锁/写锁成功，成功后同一锁资源再次加锁成功

        期望：
        1.对资源1，资源2加锁1，加锁成功
        2.加锁成功后，再次对资源1，资源2加锁1，加锁成功
        :return:
        """
        resource_1 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_2 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.READ_LOCK}
        resource = [resource_1, resource_2]
        lock_id = str(uuid.uuid4())
        request = messaging_utils.Request(
            str(uuid.uuid4()), "test_resource_lock_success1")
        async_lock_decorator.async_lock(request, resource, 0, lock_id, 0)
        self.assertEqual(msg_info.get(request.response_topic).status,
                         consts.MESSAGE_OK)
        job_req = job_log_level.get(request.request_id)
        job_level = job_req['jobLogs'][0]['level']
        self.assertEqual(job_level, job_enum.JobLogLevel.INFO.value)
        request = messaging_utils.Request(
            str(uuid.uuid4()), "test_resource_lock_success2")
        async_lock_decorator.async_lock(request, resource, 0, lock_id, 0)
        self.assertEqual(msg_info.get(request.response_topic).status,
                         consts.MESSAGE_OK)

    @unittest.skip
    def test_write_when_write_locked_failure(self):
        """
        写锁锁定时，再次添加写锁失败

        期望：
        1.对资源1写锁，资源2写锁加锁1，加锁成功
        2.加锁成功后，再次对资源2写锁，资源3写锁加锁2，加锁失败
        :return:
        """
        request = messaging_utils.Request(
            str(uuid.uuid4()), "test_write_when_write_locked_failure1")
        resource_1 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_2 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_3 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource = [resource_1, resource_2]
        async_lock_decorator.async_lock(request, resource, 0, str(uuid.uuid4()), 0)
        self.assertEqual(msg_info.get(request.response_topic).status,
                         consts.MESSAGE_OK)
        resource = [resource_2, resource_3]
        request = messaging_utils.Request(
            str(uuid.uuid4()), "test_write_when_write_locked_failure2")
        async_lock_decorator.async_lock(request, resource, 0, str(uuid.uuid4()), 0)
        self.assertEqual(msg_info.get(request.response_topic).status,
                         consts.MESSAGE_FAIL)

    @unittest.skip
    def test_read_when_write_locked_failure(self):
        """
        写锁锁定时，再次添加读锁失败

       期望：
        1.对资源1写锁，资源2写锁加锁1，加锁成功
        2.加锁成功后，再次对资源2读锁，资源3写锁加锁2，加锁失败
        :return:
        """
        request = messaging_utils.Request(
            str(uuid.uuid4()), "test_read_when_write_locked_failure1")
        resource_1 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_2 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_3 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource = [resource_1, resource_2]
        async_lock_decorator.async_lock(request, resource, 0, str(uuid.uuid4()), 0)
        self.assertEqual(msg_info.get(request.response_topic).status,
                         consts.MESSAGE_OK)
        resource_2[consts.LOCK_TYPE] = consts.READ_LOCK
        resource = [resource_2, resource_3]
        request = messaging_utils.Request(
            str(uuid.uuid4()), "test_read_when_write_locked_failure2")
        async_lock_decorator.async_lock(request, resource, 0, str(uuid.uuid4()), 0)
        self.assertEqual(msg_info.get(request.response_topic).status,
                         consts.MESSAGE_FAIL)

    @unittest.skip
    def test_write_when_read_locked_failure(self):
        """
        读锁锁定时，再次添加写锁失败

       期望：
        1.对资源1写锁，资源2读锁加锁1，加锁成功
        2.加锁成功后，再次对资源2写锁，资源3写锁加锁2，加锁失败
        :return:
        """
        request = messaging_utils.Request(
            str(uuid.uuid4()), "test_write_when_read_locked_failure1")
        resource_1 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_2 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.READ_LOCK}
        resource_3 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource = [resource_1, resource_2]
        async_lock_decorator.async_lock(request, resource, 0, str(uuid.uuid4()), 0)
        self.assertEqual(msg_info.get(request.response_topic).status,
                         consts.MESSAGE_OK)
        resource_2[consts.LOCK_TYPE] = consts.WRITE_LOCK
        resource = [resource_2, resource_3]
        request = messaging_utils.Request(
            str(uuid.uuid4()), "test_write_when_read_locked_failure2")
        async_lock_decorator.async_lock(request, resource, 0, str(uuid.uuid4()), 0)
        self.assertEqual(msg_info.get(request.response_topic).status,
                         consts.MESSAGE_FAIL)

    @unittest.skip
    def test_read_when_read_locked_success(self):
        """
        读锁锁定时，再次添加读锁成功

       期望：
        1.对资源1写锁，资源2读锁加锁1，加锁成功
        2.加锁成功后，再次对资源2读锁，资源3写锁加锁2，加锁成功
        :return:
        """
        request = messaging_utils.Request(
            str(uuid.uuid4()), "test_read_when_read_locked_success1")
        resource_1 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_2 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.READ_LOCK}
        resource_3 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource = [resource_1, resource_2]
        async_lock_decorator.async_lock(request, resource, 0, str(uuid.uuid4()), 0)
        self.assertEqual(msg_info.get(request.response_topic).status,
                         consts.MESSAGE_OK)
        resource_2[consts.LOCK_TYPE] = consts.READ_LOCK
        resource = [resource_2, resource_3]
        request = messaging_utils.Request(
            str(uuid.uuid4()), "test_read_when_read_locked_success2")
        async_lock_decorator.async_lock(request, resource, 0, str(uuid.uuid4()), 0)
        self.assertEqual(msg_info.get(request.response_topic).status,
                         consts.MESSAGE_OK)

    @unittest.skip
    @mock.patch('app.resource_lock.kafka.messaging_utils.handle_pending',
                mock.Mock())
    def test_writing_locked_success(self):
        """
        已锁定时，再次添加锁, 超时前锁定成功

       期望：
        1.对资源1写锁，资源2写锁加锁1，加锁成功
        2.加锁成功后，再次对资源2写锁，资源3写锁加锁2，加锁进入等待
        3.加锁进入等待，等待超时前，锁1解锁成功，锁2加锁成功
        4.超时任务回调时，发现锁2已加锁
        :return:
        """
        request1 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_locked_success1")
        resource_1 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_2 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_3 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource = [resource_1, resource_2]
        lock_id1 = str(uuid.uuid4())
        async_lock_decorator.async_lock(request1, resource, 0, lock_id1, 0)
        self.assertEqual(msg_info.get(request1.response_topic).status,
                         consts.MESSAGE_OK)
        resource = [resource_2, resource_3]
        lock_id2 = str(uuid.uuid4())
        request2 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_locked_success2")
        async_lock_decorator.async_lock(request2, resource, 100, lock_id2, 0)
        self.assertFalse(request2.response_topic in msg_info.keys())
        request3 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_locked_success3")
        async_lock_decorator.async_unlock(request3, lock_id1)
        self.assertEqual(msg_info.get(request3.response_topic).status,
                         consts.MESSAGE_OK)
        self.assertEqual(msg_info.get(request2.response_topic).status,
                         consts.MESSAGE_OK)
        request4 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_locked_success4")
        async_lock_decorator.has_lock(request4, lock_id2, realse_pending=True)
        self.assertEqual(msg_info.get(request4.response_topic).status,
                         consts.STATE_LOCKED)

    @unittest.skip
    @mock.patch('app.resource_lock.kafka.messaging_utils.handle_pending',
                mock.Mock())
    def test_writing_locked_failure(self):
        """
        已锁定时，再次添加锁, 超时锁定失败

       期望：
        1.对资源1写锁，资源2写锁加锁1，加锁成功
        2.加锁成功后，再次对资源2写锁，资源3写锁加锁2，加锁进入等待
        3.超时任务回调时强制解锁，发现锁2已解锁，加锁失败
        :return:
        """
        request1 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_locked_failure1")
        resource_1 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_2 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_3 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource = [resource_1, resource_2]
        lock_id1 = str(uuid.uuid4())
        async_lock_decorator.async_lock(request1, resource, 0, lock_id1, 0)
        self.assertEqual(msg_info.get(request1.response_topic).status,
                         consts.MESSAGE_OK)
        resource = [resource_2, resource_3]
        lock_id2 = str(uuid.uuid4())
        request2 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_locked_failure2")
        async_lock_decorator.async_lock(request2, resource, 100, lock_id2, 0)
        self.assertFalse(request2.response_topic in msg_info.keys())
        request3 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_locked_failure3")
        # 大于0 pending  没有加锁消息
        async_lock_decorator.has_lock(request3, lock_id2, realse_pending=100)
        self.assertIsNone(msg_info.get(request3.response_topic))
        # 小于0 加锁失败 
        request4 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_locked_failure4")
        async_lock_decorator.has_lock(request4, lock_id2, realse_pending=-10)
        self.assertEqual(msg_info.get(request4.response_topic).status,
                         consts.STATE_UNLOCK)
        self.assertEqual(msg_info.get(request2.response_topic).status,
                         consts.MESSAGE_FAIL)

    @unittest.skip
    @mock.patch('app.resource_lock.kafka.messaging_utils.handle_pending',
                mock.Mock())
    def test_writing_again_lock_failure(self):
        """
        未锁定时，再次添加锁, 锁定失败

       期望：
        1.对资源1写锁，资源2写锁加锁1，加锁成功
        2.加锁成功后，再次对资源2写锁，资源3写锁加锁2，加锁进入等待
        3.加锁进入等待，再次对资源2写锁，资源3写锁加锁2，消息库数据库已存在该失败信息
        4.超时任务回调时，发现锁2已解锁，加锁失败
        :return:
        """
        request1 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_again_lock_failure1")
        resource_1 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_2 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_3 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource = [resource_1, resource_2]
        lock_id1 = str(uuid.uuid4())
        async_lock_decorator.async_lock(request1, resource, 0, lock_id1, 0)
        self.assertEqual(msg_info.get(request1.response_topic).status,
                         consts.MESSAGE_OK)
        resource = [resource_2, resource_3]
        lock_id2 = str(uuid.uuid4())
        request2 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_again_lock_failure2")
        async_lock_decorator.async_lock(request2, resource, 100, lock_id2, 0)
        self.assertFalse(request2.response_topic in msg_info.keys())
        request3 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_again_lock_failure3")
        async_lock_decorator.async_lock(request3, resource, 100, lock_id2, 0)
        self.assertIsNone(msg_info.get(request2.response_topic))
        self.assertIsNone(msg_info.get(request3.response_topic))

    @unittest.skip
    @mock.patch('app.resource_lock.kafka.messaging_utils.handle_pending',
                mock.Mock())
    def test_unlock_writing_lock_failure(self):
        """
        未锁定时，直接解锁, 锁定失败

       期望：
        1.对资源1写锁，资源2写锁加锁1，加锁成功
        2.加锁成功后，再次对资源2写锁，资源3写锁加锁2，加锁进入等待
        3.加锁进入等待，直接解锁锁2，加锁失败，解锁成功
        4.超时任务回调时，发现锁2已解锁，加锁失败
        :return:
        """
        request1 = messaging_utils.Request(
            str(uuid.uuid4()), "test_unlock_writing_lock_failure1")
        resource_1 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_2 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_3 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource = [resource_1, resource_2]
        lock_id1 = str(uuid.uuid4())
        async_lock_decorator.async_lock(request1, resource, 0, lock_id1, 0)
        self.assertEqual(msg_info.get(request1.response_topic).status,
                         consts.MESSAGE_OK)
        resource = [resource_2, resource_3]
        lock_id2 = str(uuid.uuid4())
        request2 = messaging_utils.Request(
            str(uuid.uuid4()), "test_unlock_writing_lock_failure2")
        async_lock_decorator.async_lock(request2, resource, 100, lock_id2, 0)
        self.assertFalse(request2.response_topic in msg_info.keys())
        request3 = messaging_utils.Request(
            str(uuid.uuid4()), "test_unlock_writing_lock_failure3")
        async_lock_decorator.async_unlock(request3, lock_id2)
        self.assertEqual(msg_info.get(request2.response_topic).status,
                         consts.MESSAGE_FAIL)
        self.assertEqual(msg_info.get(request3.response_topic).status,
                         consts.MESSAGE_OK)
        request4 = messaging_utils.Request(
            str(uuid.uuid4()), "test_unlock_writing_lock_failure4")
        async_lock_decorator.has_lock(request4, lock_id2, realse_pending=True)
        self.assertEqual(msg_info.get(request4.response_topic).status,
                         consts.STATE_UNLOCK)

    @unittest.skip
    @mock.patch('app.resource_lock.kafka.messaging_utils.handle_pending',
                mock.Mock())
    def test_writing_priority_success(self):
        """
        等锁时，优先级越高时间越早的锁越先调度

       期望：
        1.对资源1写锁，资源2写锁加锁1，加锁成功
        2.加锁成功后，再次对资源2写锁，资源3写锁加锁2，优先级3，加锁进入等待
        3.加锁进入等待，再次对资源2写锁加锁3，优先级1，加锁进入等待
        4.加锁进入等待，再次对资源2写锁加锁4，优先级3，加锁进入等待
        5.解锁锁1，锁1解锁成功，锁3加锁成功，锁2和锁4继续等待
        6.解锁锁3，锁3解锁成功，锁2加锁成功，锁4继续等待
        :return:
        """
        request1 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_priority_success1")
        resource_1 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_2 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_3 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource = [resource_1, resource_2]
        lock_id1 = str(uuid.uuid4())
        async_lock_decorator.async_lock(request1, resource, 0, lock_id1, 0)
        self.assertEqual(msg_info.get(request1.response_topic).status,
                         consts.MESSAGE_OK)
        resource = [resource_2, resource_3]
        lock_id2 = str(uuid.uuid4())
        request2 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_priority_success2")
        async_lock_decorator.async_lock(request2, resource, 100, lock_id2, 3)
        self.assertFalse(request2.response_topic in msg_info.keys())
        resource = [resource_2]
        lock_id3 = str(uuid.uuid4())
        request3 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_priority_success3")
        async_lock_decorator.async_lock(request3, resource, 100, lock_id3, 1)
        self.assertFalse(request3.response_topic in msg_info.keys())
        # sleep 1秒 避免后面的锁和前面的锁是同一秒添加
        time.sleep(1)
        resource = [resource_2]
        lock_id4 = str(uuid.uuid4())
        request4 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_priority_success4")
        async_lock_decorator.async_lock(request4, resource, 100, lock_id4, 3)
        self.assertFalse(request4.response_topic in msg_info.keys())
        request5 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_priority_success5")
        async_lock_decorator.async_unlock(request5, lock_id1)
        self.assertEqual(msg_info.get(request5.response_topic).status,
                         consts.MESSAGE_OK)
        self.assertFalse(request2.response_topic in msg_info.keys())
        self.assertFalse(request4.response_topic in msg_info.keys())
        self.assertEqual(msg_info.get(request3.response_topic).status,
                         consts.MESSAGE_OK)
        request6 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_priority_success6")
        async_lock_decorator.async_unlock(request6, lock_id3)
        self.assertEqual(msg_info.get(request6.response_topic).status,
                         consts.MESSAGE_OK)
        self.assertFalse(request4.response_topic in msg_info.keys())
        self.assertEqual(msg_info.get(request2.response_topic).status,
                         consts.MESSAGE_OK)

    @unittest.skip
    def test_writing_job_failure(self):
        """
        等锁调度任务创建失败，导致加锁失败

       期望：
        1.对资源1写锁，资源2写锁加锁1，加锁成功
        2.加锁成功后，再次对资源2写锁，资源3写锁加锁2，加锁进入等待
        3.创建调度等待任务失败，加锁2直接失败
        :return:
        """
        request1 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_job_failure1")
        resource_1 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_2 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource_3 = {consts.RESOURCE_ID: str(uuid.uuid4()),
                      consts.LOCK_TYPE: consts.WRITE_LOCK}
        resource = [resource_1, resource_2]
        lock_id1 = str(uuid.uuid4())
        async_lock_decorator.async_lock(request1, resource, 0, lock_id1, 0)
        self.assertEqual(msg_info.get(request1.response_topic).status,
                         consts.MESSAGE_OK)
        resource = [resource_2, resource_3]
        lock_id2 = str(uuid.uuid4())
        request2 = messaging_utils.Request(
            str(uuid.uuid4()), "test_writing_job_failure2")
        async_lock_decorator.async_lock(request2, resource, 100, lock_id2, 0)
        self.assertEqual(msg_info.get(request2.response_topic).status,
                         consts.MESSAGE_FAIL)

    @unittest.skip
    def test_process_restart(self):
        """
        消息队列消息未消费，进程重启场景

       期望：
        1.入库加锁消息1，入库解锁消息2
        2.调度进程恢复消息
        3.不存在消息1的锁-->消息1加锁失败
        4.不存在消息2的锁-->消息2解锁成功
        :return:
        """
        request1 = messaging_utils.Request(
            str(uuid.uuid4()), "test_process_restart1")
        messaging_utils.push_to_msg_registry(
            str(uuid.uuid4()), consts.LOCK, request1)
        request2 = messaging_utils.Request(
            str(uuid.uuid4()), "test_process_restart2")
        messaging_utils.push_to_msg_registry(
            str(uuid.uuid4()), consts.UNLOCK, request2)
        messaging_utils.recover_unsent_messages(async_lock_decorator)
        self.assertIsNone(msg_info.get(request1.response_topic))
        self.assertIsNone(msg_info.get(request2.response_topic))


if __name__ == '__main__':
    unittest.main(verbosity=2)
