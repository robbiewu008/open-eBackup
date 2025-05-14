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
import datetime
import unittest
import uuid
from unittest import mock
from unittest.mock import MagicMock

from dateutil import parser

from app.common.deploy_type import DeployType
from app.common.enums.anti_ransomware_enum import AntiRansomwareEnum
from app.common.enums.copy_enum import GenerationType
from app.common.enums.job_enum import JobStatus
from app.common.enums.sla_enum import WormValidityTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.copy_catalog.common.copy_status import CopyStatus
from app.copy_catalog.copy_error_code import CopyErrorCode
from app.copy_catalog.schemas import CopyRetentionPolicySchema, CopySchema
from tests.test_cases import common_mocker  # noqa
from tests.test_cases.copy_catalog.util.mock_util import common_mock
from tests.test_cases.tools import functiontools, timezone

common_mock()

mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
           timezone.dmc.query_time_zone).start()
mock.patch("app.common.database.Database.initialize", mock.Mock).start()
mock.patch("confluent_kafka.Producer", mock.Mock).start()
mock.patch("app.common.clients.copy_manager_client.CopyManagerClient.query_associated_copies",
           mock.Mock(return_value=[])).start()

from app.copy_catalog.models.tables_and_sessions import CopyTable, CopyAntiRansomwareTable
from app.copy_catalog.util.copy_util import check_copy_can_be_deleted, get_copy_default_name, \
    check_associated_copies_can_be_deleted, check_is_snapshot_copy_in_ocean_protect, check_should_be_stop_by_job_status, \
    raise_update_copy_or_snapshot_expire_time_exception, get_valid_retention_info, compare_copy_expiration_time, pick, \
    raise_copy_or_snapshot_is_detecting_exception, check_copy_has_supported_stopping, create_delete_copy_job
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.copy_catalog.common.common import IndexStatus

copy_anti_ransomware_table = CopyAntiRansomwareTable(
    copy_id=str(uuid.uuid4()),
    status=AntiRansomwareEnum.UNDETECTED)


class CopyUtilTest(unittest.TestCase):

    @mock.patch("sqlalchemy.orm.query.Query.one_or_none")
    @mock.patch("app.copy_catalog.util.copy_util.check_copy_browse_status", mock.Mock(return_value=False))
    def test_check_copy_delete_condition_is_available_when_copy_exist_clone_file_system_success(self,
                                                                                                mock_one_or_none):
        """
        用例场景：删除副本
        前置条件：删除存在克隆文件系统的副本
        检查点：存在克隆文件系统的副本，不能删除
        """
        sub_copy = CopyTable(
            uuid=str(uuid.uuid4())
        )
        cur_copy = CopyTable(
            uuid=str(uuid.uuid4()),
            status=CopyStatus.NORMAL,
            timestamp='1632410481532988',
            resource_name='gtest'
        )
        mock_one_or_none.return_value = copy_anti_ransomware_table
        DeployType.is_ocean_protect_type = MagicMock(return_value=False)
        DeployType.is_cloud_backup_type = MagicMock(return_value=False)
        DeployType.is_not_support_dee_restful_deploy_type = MagicMock(return_value=False)
        error_code, _ = check_copy_can_be_deleted(sub_copy, cur_copy)
        self.assertEqual(error_code, CopyErrorCode.ERROR_DELETE_COPY_EXIST_CLONE_FILE_SYSTEM)

    @mock.patch("sqlalchemy.orm.query.Query.one_or_none")
    @mock.patch("app.copy_catalog.util.copy_util.check_copy_browse_status", mock.Mock(return_value=False))
    def test_check_copy_delete_condition_is_available_when_copy_status_in_restore_mounting_unmount_delete_success(self,
                                                                                                                  mock_one_or_none):
        """
        用例场景：删除副本
        前置条件：待删除副本状态为恢复中、挂载中、卸载中、删除中
        检查点：处于恢复中、挂载中、卸载中、删除中的副本不能被删除
        """
        sub_copy = CopyTable(
            uuid=str(uuid.uuid4())
        )
        cur_copy = CopyTable(
            uuid=str(uuid.uuid4()),
            status=CopyStatus.RESTORING,
            timestamp='1632410481532988',
            resource_name='gtest',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.Fileset
        )
        mock_one_or_none.return_value = copy_anti_ransomware_table
        DeployType.is_cloud_backup_type = MagicMock(return_value=False)
        DeployType.is_not_support_dee_restful_deploy_type = MagicMock(return_value=False)
        error_code, _ = check_copy_can_be_deleted(sub_copy, cur_copy)
        self.assertEqual(error_code, CommonErrorCodes.STATUS_ERROR)
        cur_copy.status = CopyStatus.MOUNTING.value
        error_code, _ = check_copy_can_be_deleted(sub_copy, cur_copy)
        self.assertEqual(error_code, CommonErrorCodes.STATUS_ERROR)
        cur_copy.status = CopyStatus.UNMOUNTING.value
        error_code, _ = check_copy_can_be_deleted(sub_copy, cur_copy)
        self.assertEqual(error_code, CommonErrorCodes.STATUS_ERROR)
        cur_copy.status = CopyStatus.DELETING.value
        error_code, _ = check_copy_can_be_deleted(sub_copy, cur_copy)
        self.assertEqual(error_code, CommonErrorCodes.STATUS_ERROR)

    @mock.patch("app.common.deploy_type.DeployType.is_ocean_protect_type",
                mock.Mock(return_value=True))
    @mock.patch("app.common.clients.system_base_client.SystemBaseClient.query_local_storage_fssnapshot",
                mock.Mock(return_value={"isSecuritySnap": True, "isInProtectionPeriod": True}))
    @mock.patch("app.common.clients.system_base_client.SystemBaseClient.get_fs_snapshot_by_names",
                mock.Mock(return_value={"isSecuritySnap": True, "isInProtectionPeriod": True}))
    def test_check_is_ocean_protect_snap_copy_when_copy_is_security_snapshot(self):
        """
            用例场景：删除副本
            前置条件：待删除副本是安全快照
            检查点：处于保护期的安全快照的副本不能被删除
        """
        vmware_copy = CopyTable(
            uuid=str(uuid.uuid4()),
            resource_sub_type=ResourceSubTypeEnum.VirtualMachine.value,
            properties='''
             {
                "vmware_metadata": {
                    "disk_info": [{
                        "DISKDEVICENAME": "18105556305334593051",
                        "DISKSNAPSHOTDEVICENAME": "14679207360381467969"
                    }]
                }
            }
            '''
        )
        vmware_copy_check = check_is_snapshot_copy_in_ocean_protect(vmware_copy)
        self.assertTrue(vmware_copy_check)

        common_copy = CopyTable(
            uuid=str(uuid.uuid4()),
            resource_sub_type=ResourceSubTypeEnum.Fileset.value,
            properties='''
                     {
                        "snapshots": [{
                            "id": "22950@39da292d-ebd9-40ab-aa5c-4e42d3c0084c",
                            "parentName": "Storage_8465632d-22e0-37a6-9dc1-573c0cb61b47"
                        }],
                        "tenantId": "0",
                        "format": 0
                    }
                    '''
        )
        common_copy_check = check_is_snapshot_copy_in_ocean_protect(common_copy)
        self.assertTrue(common_copy_check)

    @mock.patch("app.common.deploy_type.DeployType.is_ocean_protect_type",
                mock.Mock(return_value=True))
    @mock.patch("app.copy_catalog.util.copy_util.AntiRansomwareClient.get_copy_expire_status",
                mock.Mock(return_value=False))
    def test_check_is_ocean_protect_snap_copy_when_copy_is_directory_worm_1(self):
        """
            用例场景：删除副本
            前置条件：待删除副本是非原生格式，即目录格式
            检查点：目录格式副本的WORM策略未到期时，不允许删除
        """
        common_copy = CopyTable(
            uuid=str(uuid.uuid4()),
            resource_sub_type=ResourceSubTypeEnum.Fileset.value,
            properties="{\"tenantId\": \"0\", \"format\": 1, \"worm_status\": 1}"
        )
        common_copy_check = check_is_snapshot_copy_in_ocean_protect(common_copy)
        self.assertTrue(common_copy_check)

    @mock.patch("app.common.deploy_type.DeployType.is_ocean_protect_type",
                mock.Mock(return_value=True))
    @mock.patch("app.copy_catalog.util.copy_util.AntiRansomwareClient.get_copy_expire_status",
                mock.Mock(return_value=False))
    def test_check_is_ocean_protect_snap_copy_when_copy_is_directory_worm_2(self):
        """
            用例场景：删除副本
            前置条件：待删除副本格式不正确
            检查点：返回False
        """
        common_copy = CopyTable(
            uuid=str(uuid.uuid4()),
            resource_sub_type=ResourceSubTypeEnum.Fileset.value,
            properties="{\"tenantId\": \"0\", \"format\": 999, \"worm_status\": 1}"
        )
        common_copy_check = check_is_snapshot_copy_in_ocean_protect(common_copy)
        self.assertFalse(common_copy_check)

    @mock.patch("app.common.deploy_type.DeployType.is_ocean_protect_type",
                mock.Mock(return_value=True))
    def test_check_is_ocean_protect_snap_copy_when_copy_is_directory_worm_3(self):
        """
            用例场景：删除副本时抛出异常
            前置条件：待删除副本是非原生格式，即目录格式
            返回值：抛出异常，返回False
        """
        global copy_check
        common_copy = CopyTable(
            uuid=str(uuid.uuid4()),
            resource_sub_type=ResourceSubTypeEnum.Fileset.value,
            properties="{\"tenantId\": \"0\", \"format\": 1, \"worm_status\": 1}"
        )
        try:
            copy_check = check_is_snapshot_copy_in_ocean_protect(common_copy)
        except:
            self.assertFalse(copy_check)

    @mock.patch("sqlalchemy.orm.query.Query.one_or_none")
    @mock.patch("app.copy_catalog.util.copy_util.check_copy_browse_status", mock.Mock(return_value=False))
    def test_check_copy_delete_condition_is_available_when_copy_indexed_is_indexing_success(self, mock_one_or_none):
        """
        用例场景：删除副本
        前置条件：待删除副本状态索引中
        检查点：正在索引的副本不允许被删除
        """
        sub_copy = CopyTable(
            uuid=str(uuid.uuid4())
        )
        cur_copy = CopyTable(
            uuid=str(uuid.uuid4()),
            status=CopyStatus.NORMAL,
            timestamp='1632410481532988',
            resource_name='gtest',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.Fileset,
            indexed=IndexStatus.INDEXING.value
        )
        mock_one_or_none.return_value = copy_anti_ransomware_table
        DeployType.is_ocean_protect_type = MagicMock(return_value=False)
        DeployType.is_cloud_backup_type = MagicMock(return_value=False)
        DeployType.is_not_support_dee_restful_deploy_type = MagicMock(return_value=False)
        error_code, _ = check_copy_can_be_deleted(sub_copy, cur_copy)
        self.assertEqual(error_code, CopyErrorCode.FORBID_DELETE_INDEXING_COPY)

    def test_get_default_copy_name_success(self):
        resource_name = 'gtest'
        copy_timestamp = '1645517480297692'
        copy_name = resource_name + '_' + '1645517480'
        self.assertEqual(copy_name, get_copy_default_name(resource_name, copy_timestamp))

    @unittest.skip
    @mock.patch("app.common.clients.copy_manager_client.CopyManagerClient.query_associated_copies",
                mock.Mock(return_value=["1", "2"]))
    @mock.patch("app.common.clients.anti_ransomware_client.AntiRansomwareClient.get_copy_expire_status",
                mock.Mock(return_value=False))
    @mock.patch("app.copy_catalog.models.tables_and_sessions.database.session")
    def test_check_associated_copies_can_be_deleted(self, _mock_session):
        """
        用例场景：删除副本
        前置条件：待删除副本存在关联副本，且关联副本为worm
        检查点：关联副本为worm的副本不允许被删除
        """
        copy = CopyTable()
        copy.properties = '{"dataAfterReduction":116,"format":1,"associatedCopies":["5e7a4a8e-9408-43b1-9db9-7bac109da82e","f17b3507-ab30-49d2-833b-c42bf5e79e58"],"isSanClient":"false","size":0,"logDirName":"eed42ba3-6baa-4283-a3f3-fe24736e0c36","verifyStatus":"3","repositories":[{"id":"2102355MFQ10PC100001","type":3,"protocol":5,"role":0,"remotePath":[{"type":1,"path":"/Database_facc9f0bc86c4a15afdcac5526ce1714_LogRepository_su0/eed42ba3-6baa-4283-a3f3-fe24736e0c36","id":"24"}],"extendInfo":{"esn":"2102355MFQ10PC100001","storage_info":{"storage_device":"2102355MFQ10PC100001","storage_pool":"0"},"capacityAvailable":true,"logBackup":{"latestDataCopyId":"5e7a4a8e-9408-43b1-9db9-7bac109da82e","latestLogCopyName":"4de713d3-98b3-4b5a-a7b2-11b0c0192398"}}},{"id":"2102355MFQ10PC100001","type":2,"protocol":5,"role":0,"remotePath":[{"type":1,"path":"/Database_CacheDataRepository_su0/facc9f0bc86c4a15afdcac5526ce1714","id":"23"}],"extendInfo":{"esn":"2102355MFQ10PC100001","storage_info":{"storage_device":"2102355MFQ10PC100001","storage_pool":"0"},"capacityAvailable":true}}],"dataBeforeReduction":572,"beginTime":1732534985,"endTime":1732538313,"multiFileSystem":"false","pre_log_index":5,"labelList":[]}'
        _mock_session().__enter__().query().filter().count.return_value = 1
        _mock_session().__enter__().query().filter().all.return_value = [[copy]]
        with self.assertRaises(EmeiStorBizException) as ex:
            check_associated_copies_can_be_deleted(CopyTable(uuid="1"))
        self.assertEqual(ex.exception.error_code, CopyErrorCode.DELETE_WORM_RELATED_COPY_FAIL.get("code"))

    @mock.patch("app.base.db_base.database.session")
    def test_check_should_be_stop_by_job_status(self, _mock_session):
        class MockJob:
            def __init__(self, status):
                self.status = status

        # 测试当job为None时，应该抛出异常
        _mock_session().__enter__().query().filter().first.return_value = None
        with self.assertRaises(EmeiStorBizException) as _:
            check_should_be_stop_by_job_status('job_none')

        # 测试当job的状态为RUNNING时，应该返回False
        _mock_session().__enter__().query().filter().first.return_value = MockJob(JobStatus.RUNNING.value)
        self.assertFalse(check_should_be_stop_by_job_status('job_running'))

        # 测试当job的状态为RUNNING时，应该返回False
        _mock_session().__enter__().query().filter().first.return_value = MockJob(JobStatus.ABORTING.value)
        self.assertTrue(check_should_be_stop_by_job_status('job_aborting'))

    def test_raise_update_copy_or_snapshot_expire_time_exception_when_deploy_type_is_not_CYBER_ENGINE(self):
        """
        测试引发更新副本或快照过期时间异常，部署类型不为CYBER_ENGINE
        期望：抛出EmeiStorBizException异常
        return:抛出EmeiStorBizException异常
        """
        DeployType.is_cyber_engine_deploy_type = MagicMock(return_value=False)
        try:
            raise_update_copy_or_snapshot_expire_time_exception("error")
        except:
            self.assertTrue(EmeiStorBizException)

    def test_raise_update_copy_or_snapshot_expire_time_exception_when_deploy_type_is_CYBER_ENGINE(self):
        """
        测试引发更新副本或快照过期时间异常，部署类型为CYBER_ENGINE
        期望：抛出EmeiStorBizException异常
        return:抛出EmeiStorBizException异常
        """
        DeployType.is_cyber_engine_deploy_type = MagicMock(return_value=True)
        try:
            raise_update_copy_or_snapshot_expire_time_exception("error")
        except:
            self.assertTrue(EmeiStorBizException)

    def test_get_valid_retention_info_when_retention_type_is_not_temporary(self):
        """
        获取有效的保留信息，保留策略中的保留类型为永久保留
        期望：因为不是按时间保留，所以返回值duration_unit为None, expiration_time为None, retention_duration为0, temporary为False
        return:None, None, 0, False
        """
        retention_policy = dict(resource_id="123", retention_type=1, retention_duration=1, duration_unit="d",
                                worm_validity_type=WormValidityTypeEnum.copy_retention_time_consistent)
        duration_unit, expiration_time, retention_duration, temporary, worm_validity_type = \
            get_valid_retention_info(CopyRetentionPolicySchema(**retention_policy), 10)
        self.assertEqual(None, duration_unit)
        self.assertEqual(None, expiration_time)
        self.assertEqual(0, retention_duration)
        self.assertEqual(False, temporary)

    def test_get_valid_retention_info_when_retention_type_is_temporary_and_duration_unit_is_days(self):
        """
        获取有效的保留信息，保留策略中的保留类型为按时间保留
        期望：retention_duration为大于365，抛出非法参数异常
        return:EmeiStorBizException
        """
        retention_policy = dict(resource_id="123", retention_type=2, retention_duration=366, duration_unit="d")
        with self.assertRaises(EmeiStorBizException) as ex:
            get_valid_retention_info(CopyRetentionPolicySchema(**retention_policy), 10)
        self.assertEqual(ex.exception.error_code, CommonErrorCodes.ILLEGAL_PARAMS.get("code"))
        self.assertEqual(ex.exception._error_message, "retention duration exceeds the maximum value(365)")

    def test_get_valid_retention_info_when_retention_type_is_temporary_and_duration_unit_is_days_2(self):
        """
        获取有效的保留信息，保留策略中的保留类型为按时间保留
        期望：retention_duration为小于1，抛出非法参数异常
        return:EmeiStorBizException
        """
        retention_policy = dict(resource_id="123", retention_type=2, retention_duration=0, duration_unit="d")
        with self.assertRaises(EmeiStorBizException) as ex:
            get_valid_retention_info(CopyRetentionPolicySchema(**retention_policy), 10)
        self.assertEqual(ex.exception.error_code, CommonErrorCodes.ILLEGAL_PARAMS.get("code"))
        self.assertEqual(ex.exception._error_message, "retention duration exceeds the minimum value(1)")

    def test_get_valid_retention_info_when_retention_type_is_temporary_and_duration_unit_is_days_3(self):
        """
        获取有效的保留信息，保留策略中的保留类型为按时间保留
        期望：返回duration_unit, expiration_time, retention_duration, temporary的值
        return:d，参数timestamp的值+retention_duration的值，180，True
        """
        retention_policy = dict(resource_id="123", retention_type=2, retention_duration=180, duration_unit="d",
                                worm_validity_type=WormValidityTypeEnum.copy_retention_time_consistent)
        duration_unit, expiration_time, retention_duration, temporary, worm_validity_type = \
            get_valid_retention_info(CopyRetentionPolicySchema(**retention_policy), datetime.datetime.now())
        self.assertEqual("d", duration_unit)
        self.assertEqual(180, retention_duration)
        self.assertEqual(True, temporary)

    def test_compare_copy_expiration_time_when_copy_and_modify_copy_is_None(self):
        """
        比较副本过期时间
        期望：由于copy和modify_copy都为None，应返回True
        return:True
        """
        retention_policy = dict(resource_id="123", retention_type=2, retention_duration=1, duration_unit="d")
        result = compare_copy_expiration_time(None, None, CopyRetentionPolicySchema(**retention_policy))
        self.assertTrue(result)

    def test_compare_copy_expiration_time_when_retention_type_is_permanent_and_backup_type_is_full(self):
        """
        比较副本过期时间
        期望：由于保留策略是永久保留，副本类型是全量副本，应返回True
        return:True
        """
        retention_policy = dict(resource_id="123", retention_type=1, retention_duration=1, duration_unit="d")
        copy_table = CopyTable(
            uuid=str(uuid.uuid4()),
            status=CopyStatus.RESTORING,
            timestamp='1632410481532988',
            backup_type=1,
            resource_name='gtest',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.Fileset
        )
        copy = CopySchema(
            uuid="123",
            gn=1,
            prev_copy_id="122",
            next_copy_id="124",
            prev_copy_gn=0,
            next_copy_gn=2,
            device_esn="987654321",
            cluster_name="123",
            timestamp="",
            display_timestamp="1657074770",
            deletable=False,
            status="1",
            generated_by="2",
            indexed="0",
            generation=2,
            retention_type="2",
            resource_id="1234",
            resource_name="1234",
            resource_type="Backup",
            resource_location="/",
            resource_status="",
            resource_properties="",
            browse_mounted="Umount"
        )
        result = compare_copy_expiration_time(copy, copy_table, CopyRetentionPolicySchema(**retention_policy))
        self.assertTrue(result)

    def test_compare_copy_expiration_time_when_retention_type_is_permanent_and_backup_type_is_cumulative_increment(
            self):
        """
        比较副本过期时间
        期望：由于保留策略是永久保留，副本类型是增量备份副本，其依赖的全量副本不是永久保留，应返回False
        return:False
        """
        retention_policy = dict(resource_id="123", retention_type=1, retention_duration=1, duration_unit="d")
        copy_table = CopyTable(
            uuid=str(uuid.uuid4()),
            status=CopyStatus.RESTORING,
            timestamp='1632410481532988',
            backup_type=2,
            resource_name='test',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.Fileset
        )
        copy = CopySchema(
            uuid="123456",
            gn=1,
            prev_copy_id="122456",
            next_copy_id="124456",
            prev_copy_gn=0,
            next_copy_gn=2,
            device_esn="987654321123456789",
            cluster_name="123456",
            timestamp="",
            display_timestamp="1657074770",
            deletable=True,
            status="1",
            generated_by="2",
            indexed="0",
            generation=2,
            retention_type="2",
            resource_id="1234",
            resource_name="1234",
            resource_type="Backup",
            resource_location="/",
            resource_status="",
            resource_properties="",
            browse_mounted="Umount"
        )
        result = compare_copy_expiration_time(copy, copy_table, CopyRetentionPolicySchema(**retention_policy))
        self.assertFalse(result)

    def test_compare_copy_expiration_time_when_copy_retention_type_is_permanent_and_copy_backup_type_is_cumulative_increment(
            self):
        """
        比较副本过期时间
        期望：由于副本保留策略不是永久保留，副本是定时过期的，其依赖的全量副本是定点过期的，应返回False
        return:False
        """
        retention_policy = dict(resource_id="123", retention_type=2, retention_duration=1, duration_unit="d")
        copy_table = CopyTable(
            uuid=str(uuid.uuid4()),
            status=CopyStatus.NORMAL,
            timestamp='1632410481532988',
            backup_type=2,
            resource_name='test',
            generated_by=GenerationType.BY_BACKUP.value,
            resource_sub_type=ResourceSubTypeEnum.Fileset
        )
        copy = CopySchema(
            uuid="123",
            gn=1,
            prev_copy_id="122",
            next_copy_id="124",
            prev_copy_gn=0,
            next_copy_gn=2,
            device_esn="987654321",
            cluster_name="123",
            timestamp="",
            display_timestamp="1657074770",
            deletable=True,
            status="1",
            generated_by="2",
            indexed="0",
            generation=2,
            retention_type="1",
            backup_type=2,
            resource_id="1234",
            resource_name="1234",
            resource_type="Backup",
            resource_location="/",
            resource_status="",
            resource_properties="",
            browse_mounted="Umount"
        )
        result = compare_copy_expiration_time(copy, copy_table, CopyRetentionPolicySchema(**retention_policy))
        self.assertFalse(result)

    def test_compare_copy_expiration_time_when_copy_backup_type_is_cumulative_increment_and_retention_type_is_not_permanent(
            self):
        """
        比较副本过期时间
        期望：由于副本保留策略是定时过期的，副本是定点过期的且副本类型为增量副本，修改后的副本过期时间大于副本过期时间，应返回True
        return:True
        """
        retention_policy = dict(resource_id="123", retention_type=2, retention_duration=180, duration_unit="d")
        copy_table = CopyTable(
            uuid=str(uuid.uuid4()),
            status=CopyStatus.RESTORING,
            timestamp='2023-09-01 12:00:00',
            backup_type=2,
            resource_name='test',
            generated_by=GenerationType.BY_BACKUP.value,
            display_timestamp=parser.parse("2023-09-01 12:00:00"),
            resource_sub_type=ResourceSubTypeEnum.Fileset
        )
        copy = CopySchema(
            uuid="123",
            gn=1,
            prev_copy_id="122",
            next_copy_id="124",
            prev_copy_gn=0,
            next_copy_gn=2,
            device_esn="987654321",
            cluster_name="123",
            timestamp="",
            display_timestamp=parser.parse("2023-09-01 12:00:00"),
            expiration_time=parser.parse("2023-12-01 12:00:00"),
            deletable=False,
            status="1",
            generated_by="2",
            indexed="0",
            generation=2,
            retention_type="2",
            backup_type=2,
            resource_id="1234",
            resource_name="1234",
            resource_type="Backup",
            resource_location="/",
            resource_status="",
            resource_properties="",
            browse_mounted="Umount"
        )
        result = compare_copy_expiration_time(copy, copy_table, CopyRetentionPolicySchema(**retention_policy))
        self.assertTrue(result)

    def test_compare_copy_expiration_time_when_copy_backup_type_is_full_and_retention_type_is_not_permanent(self):
        """
        比较副本过期时间
        期望：由于副本保留策略是定时过期的，副本是全量副本，定点过期，修改后的副本过期时间小于副本过期时间，应返回False
        return:False
        """
        retention_policy = dict(resource_id="123", retention_type=2, retention_duration=30, duration_unit="d")
        copy_table = CopyTable(
            uuid=str(uuid.uuid4()),
            status=CopyStatus.RESTORING,
            timestamp='2023-09-01 12:00:00',
            backup_type=2,
            resource_name='test',
            generated_by=GenerationType.BY_BACKUP.value,
            display_timestamp=parser.parse("2023-09-01 12:00:00"),
            resource_sub_type=ResourceSubTypeEnum.Fileset
        )
        copy = CopySchema(
            uuid="123",
            gn=1,
            prev_copy_id="122",
            next_copy_id="124",
            prev_copy_gn=0,
            next_copy_gn=2,
            device_esn="987654321",
            cluster_name="123",
            timestamp="",
            display_timestamp=parser.parse("2023-09-01 12:00:00"),
            expiration_time=parser.parse("2023-09-10 12:00:00"),
            deletable=False,
            status="1",
            generated_by="2",
            indexed="0",
            generation=2,
            retention_type="2",
            backup_type=1,
            resource_id="1234",
            resource_name="1234",
            resource_type="Backup",
            resource_location="/",
            resource_status="",
            resource_properties="",
            browse_mounted="Umount"
        )
        result = compare_copy_expiration_time(copy, copy_table, CopyRetentionPolicySchema(**retention_policy))
        self.assertFalse(result)

    def test_pick(self):
        """
        测试pick方法
        期望：把resource中的字段替换成相应的字段之后输出
        return:替换后的字段及其对应的值
        """
        resource = {
            "uuid": 123,
            "name": "123",
            "type": "Backup",
            "sub_type": "backup",
            "path": "/",
            "environment_name": "000",
            "environment_ip": "000.000.000.000"
        }
        resource_info = pick(resource, {
            "uuid": "resource_id",
            "name": "resource_name",
            "type": "resource_type",
            "sub_type": "resource_sub_type",
            "path": "resource_location",
            "environment_name": "resource_environment_name",
            "environment_ip": "resource_environment_ip"
        }, default="")
        self.assertEqual(
            {'resource_id': 123, 'resource_name': '123', 'resource_type': 'Backup', 'resource_sub_type': 'backup',
             'resource_location': '/', 'resource_environment_name': '000',
             'resource_environment_ip': '000.000.000.000'}, resource_info)

    def test_raise_copy_or_snapshot_is_detecting_exception_when_deploy_type_is_not_CYBER_ENGINE(self):
        """
        测试引发副本或快照正在检测时的异常，部署类型不为CYBER_ENGINE
        期望：抛出EmeiStorBizException异常，错误码1677933328
        return:抛出EmeiStorBizException异常，错误码1677933328
        """
        DeployType.is_cyber_engine_deploy_type = MagicMock(return_value=False)
        with self.assertRaises(EmeiStorBizException) as ex:
            raise_copy_or_snapshot_is_detecting_exception()
        self.assertEqual(ex.exception.error_code, CopyErrorCode.COPY_IS_DETECTING.get("code"))

    def test_raise_copy_or_snapshot_is_detecting_exception_when_deploy_type_is_CYBER_ENGINE(self):
        """
        测试引发副本或快照正在检测时的异常，部署类型不为CYBER_ENGINE
        期望：抛出EmeiStorBizException异常，错误码1677933343
        return:抛出EmeiStorBizException异常，错误码1677933343
        """
        DeployType.is_cyber_engine_deploy_type = MagicMock(return_value=True)
        with self.assertRaises(EmeiStorBizException) as ex:
            raise_copy_or_snapshot_is_detecting_exception()
        self.assertEqual(ex.exception.error_code, CopyErrorCode.SNAPSHOT_IS_DETECTING.get("code"))

    def test_check_copy_has_supported_stopping(self):
        """
        测试副本任务是否支持停止
        期望：CloudBackup的副本删除任务和过期任务不支持停止，应返回False
        return:False
        """
        copy_table = CopyTable(
            uuid=str(uuid.uuid4()),
            status=CopyStatus.RESTORING,
            timestamp='2023-09-01 12:00:00',
            backup_type=2,
            resource_name='test',
            generated_by=GenerationType.BY_BACKUP.value,
            display_timestamp=parser.parse("2023-09-01 12:00:00"),
            resource_sub_type=ResourceSubTypeEnum.CloudBackupFileSystem
        )
        self.assertFalse(check_copy_has_supported_stopping(copy_table))

    mock_client = mock.Mock(name="mock_client")
    mock_response = mock.Mock(name="mock_response")
    mock_response.status = 200
    mock_client.request = mock.Mock(name="mock_request", return_value=mock_response)

    @mock.patch("app.common.toolkit.SystemBaseHttpsClient", mock.Mock(return_value=mock_client))
    @mock.patch("app.copy_catalog.util.copy_util.resource_service.query_resource_by_id", mock.Mock(return_value=None))
    @mock.patch("app.copy_catalog.util.copy_util.get_job_queue_scope", mock.Mock(return_value=None))
    @mock.patch("app.resource.service.common.domain_resource_object_service.get_domain_id_list", mock.Mock(return_value=['test']))
    def test_create_delete_copy_job(self):
        """
        测试创建删除副本任务
        期望：创建成功无报错
        return:创建成功无报错
        """
        copy_table = CopyTable(
            uuid=str(uuid.uuid4()),
            status=CopyStatus.RESTORING,
            timestamp='2023-09-01 12:00:00',
            backup_type=2,
            resource_name='test',
            generated_by=GenerationType.BY_BACKUP.value,
            display_timestamp=parser.parse("2023-09-01 12:00:00"),
            resource_sub_type=ResourceSubTypeEnum.CloudBackupFileSystem
        )
        create_delete_copy_job({"request_is": 1}, copy_table)
