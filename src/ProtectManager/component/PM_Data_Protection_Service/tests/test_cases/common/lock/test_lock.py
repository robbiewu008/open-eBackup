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
from unittest.mock import patch, Mock

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

from app.common.clients.client_util import InfrastructureHttpsClient
from app.common.security.kmc_util import Kmc


class TestLock(unittest.TestCase):

    def setUp(self) -> None:
        super(TestLock, self).setUp()

        sys.modules['app.common.config'] = Mock()
        sys.modules['app.common.database'] = Mock()

        self.engine = create_engine('sqlite:///:memory:')
        self.memory_session = sessionmaker(bind=self.engine)
        session = self.memory_session()

        from app.common.lock.lock_object import LockObject
        LockObject.metadata.create_all(self.engine)
        from app.common.lock.lock import Lock
        self.lock: Lock = Lock(lock_db_session=session, key='key')

    def tearDown(self):
        from app.common.lock.lock_object import LockObject
        LockObject.metadata.drop_all(self.engine)

    @unittest.skip
    @patch.object(InfrastructureHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_generate_lock_object(self, mock_request):
        lock_object = self.lock._generate_lock_object(desc='desc', timeout=1)
        self.assertEqual(lock_object.key, 'key')
        self.assertEqual(lock_object.description, 'desc')

    @unittest.skip
    @patch.object(InfrastructureHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_do_acquire(self, mock_request):
        # 一次加锁， 返回True
        lock_status = self.lock._do_acquire(timeout=1)
        self.assertEqual(lock_status, True)
        # 二次加锁，为False
        self.assertEqual(self.lock._do_acquire(timeout=1), False)
        # 释放锁
        self.lock.unlock()
        # 释放后，重新加锁，返回True
        self.assertEqual(self.lock._do_acquire(timeout=1), True)

    @unittest.skip
    @patch.object(InfrastructureHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_is_locked(self, mock_request):
        # 加锁
        self.lock._do_acquire(timeout=1)
        self.assertEqual(self.lock.is_locked(), True)
        # 释放锁
        self.lock.unlock()
        # 释放后
        self.assertEqual(self.lock.is_locked(), False)

    @unittest.skip
    @patch.object(InfrastructureHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_lock(self, mock_request):
        from app.common.lock.lock import Lock
        lock_one: Lock = Lock(lock_db_session=self.memory_session(), key='one')
        # 加锁
        self.assertEqual(lock_one.lock(), True)
        # 释放锁
        lock_one.unlock()
        # 释放后
        self.assertEqual(lock_one.is_locked(), False)

        # 阻塞加锁，超时时间小于阻塞时间
        blocking_lock: Lock = Lock(lock_db_session=self.memory_session(),
                                   key='blocking')
        # 一次加锁
        self.assertEqual(blocking_lock.lock(timeout=1, blocking_timeout=3), True)
        # 二次加锁, 阻塞获取, 一次加锁释放后获取到锁
        self.assertEqual(blocking_lock.lock(timeout=1, blocking_timeout=3), True)

        # 阻塞加锁，超时时间大于阻塞时间
        blocking_lock_timeout: Lock = Lock(lock_db_session=self.memory_session(), key='blocking_lock_timeout')
        # 一次加锁
        self.assertEqual(blocking_lock_timeout.lock(timeout=3, blocking_timeout=1), True)
        # 二次加锁, 阻塞获取, 阻塞时间窗内上一次加锁不会释放
        self.assertEqual(blocking_lock_timeout.lock(timeout=3, blocking_timeout=1), False)

        # 非阻塞加锁
        not_blocking_lock: Lock = Lock(lock_db_session=self.memory_session(), key='not_blocking')
        # 一次加锁
        self.assertEqual(not_blocking_lock.lock(), True)
        # 二次加锁, 非阻塞获取, 一次加锁没有释放，所以失败
        self.assertEqual(not_blocking_lock.lock(), False)

    @unittest.skip
    @patch.object(InfrastructureHttpsClient, "request", autospec=True)
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @patch("app.common.util.file_utils.read_file_by_utf_8", Mock(return_value="encrypt_text"))
    def test_lock_manager(self, mock_request):
        from app.common.lock.lock_manager import lock_manager
        lock = lock_manager.get_lock('key')
        self.assertIsNotNone(lock)
