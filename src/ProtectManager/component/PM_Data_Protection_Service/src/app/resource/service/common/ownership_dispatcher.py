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
import uuid

from app.common.exception.common_error_codes import CommonErrorCodes
from app.protection.object.models.projected_object import ProtectedObject
from app.resource.models.resource_models import ResourceTable
from app.common.dispatcher.dispatcher import StateDispatcher
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exception.user_error_codes import UserErrorCodes
from app.common.logger import get_logger

log = get_logger(__name__)


class OwnershipDispatcher(object):
    query_subset_id_list = StateDispatcher()
    state = None

    def __init__(self, state):
        self.state = state

    @staticmethod
    def check_uuid(check_uuid, version=4):
        try:
            if check_uuid.endswith("_sanclient"):
                return uuid.UUID(check_uuid[0:36]).version == version
            return uuid.UUID(check_uuid).version == version
        except ValueError as exception:
            raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR, message="Invalid UUID.") from exception

    # 检查当前uuid资源是否被保护 和下边的is_current_resource_bounded_sla功能一模一样
    @staticmethod
    def is_sla_bounded(resource_id, session):
        res = session.query(ResourceTable).filter(
            ResourceTable.root_uuid == str(resource_id)
        ).join(
            ProtectedObject,
            ProtectedObject.resource_id == ResourceTable.uuid
        ).first()
        if res:
            raise EmeiStorBizException(UserErrorCodes.ALREADY_BIND_SLA, *[res.name],
                                       message="Resource already bind SLA.")

    @staticmethod
    def is_current_resource_bounded_sla(resource_id, session):
        res = session.query(ProtectedObject).filter(ProtectedObject.resource_id == str(resource_id)).first()
        if res:
            raise EmeiStorBizException(UserErrorCodes.ALREADY_BIND_SLA, *[res.name],
                                       message="Resource already bind SLA.")

    @query_subset_id_list.register(
        ResourceSubTypeEnum.NUTANIX.value,
        ResourceSubTypeEnum.CNWARE.value,
        ResourceSubTypeEnum.APSARA_STACK.value,
        ResourceSubTypeEnum.ABBackupClient.value,
        ResourceSubTypeEnum.DBBackupAgent.value,
        ResourceSubTypeEnum.VMBackupAgent.value,
        ResourceSubTypeEnum.MysqlCluster.value,
        ResourceSubTypeEnum.SQLServer.value,
        ResourceSubTypeEnum.GaussDB.value,
        ResourceSubTypeEnum.OpenGauss.value,
        ResourceSubTypeEnum.DWSCluster.value,
        ResourceSubTypeEnum.UBackupAgent.value,
        ResourceSubTypeEnum.S_BACKUP_AGENT.value,
        ResourceSubTypeEnum.PostgreCluster.value,
        ResourceSubTypeEnum.KingBaseCluster.value,
        ResourceSubTypeEnum.GOLDENDB_CLUSTER.value,
        ResourceSubTypeEnum.TDSQL_CLUSTER.value,
        ResourceSubTypeEnum.OCEANBASE_CLUSTER.value,
        ResourceSubTypeEnum.HCS_GAUSSDB_PROJECT.value,
        ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT.value,
        ResourceSubTypeEnum.KingBaseCluster.value,
        ResourceSubTypeEnum.TiDB_CLUSTER.value,
        ResourceSubTypeEnum.TiDB_DATABASE.value,
        ResourceSubTypeEnum.TiDB_TABLE.value,
        ResourceSubTypeEnum.EXCHANGE_GROUP.value,
        ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE.value,
        ResourceSubTypeEnum.COMMON_SHARE.value,
        ResourceSubTypeEnum.SAPHANA_INSTANCE,
        ResourceSubTypeEnum.InformixService.value,
        ResourceSubTypeEnum.SAP_ON_ORACLE.value,
        ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE.value,
        ResourceSubTypeEnum.EXCHANGE_ONLINE.value,
        ResourceSubTypeEnum.EXCHANGE_ONLINE_BACKUP_SET.value
    )
    def common_resource(self, resource: ResourceTable, session):
        # 检查资源是否绑定SLA，已绑定SLA的情况下，不允许进行资源分配或回收
        self.is_sla_bounded(resource.uuid, session)
        # 检查当前顶层资源是否归属于其他顶层资源的子资源
        envs_counts = session.query(ResourceTable).filter(
            ResourceTable.children_uuids.isnot(None),
            ResourceTable.children_uuids.any(resource.uuid)
        ).count()
        if envs_counts > 0:
            # 如果当前顶层资源归属于其他顶层资源的子资源，不允许单独进行分配或回收
            raise EmeiStorBizException(UserErrorCodes.NOT_ALLOW_AUTHORIZE_OR_REVOKE)
        if resource.children_uuids:
            for uid in resource.children_uuids:
                # 检查子资源是否绑定SLA，已绑定SLA的情况下，不允许进行资源分配或回收
                self.is_sla_bounded(uid, session)
            return resource.children_uuids
        return []

    @query_subset_id_list.register(ResourceSubTypeEnum.Kubernetes.value,
                                   ResourceSubTypeEnum.NUTANIX.value,
                                   ResourceSubTypeEnum.CNWARE.value,
                                   ResourceSubTypeEnum.APSARA_STACK,
                                   ResourceSubTypeEnum.APSARA_STACK.value,
                                   ResourceSubTypeEnum.KUBERNETES_CLUSTER_COMMON.value,
                                   ResourceSubTypeEnum.FusionCompute.value,
                                   ResourceSubTypeEnum.HCSTenant.value,
                                   ResourceSubTypeEnum.HCSProject.value,
                                   ResourceSubTypeEnum.Redis.value,
                                   ResourceSubTypeEnum.Hive.value,
                                   ResourceSubTypeEnum.ElasticSearch.value,
                                   ResourceSubTypeEnum.ClickHouse.value,
                                   ResourceSubTypeEnum.MysqlCluster.value,
                                   ResourceSubTypeEnum.PostgreCluster.value,
                                   ResourceSubTypeEnum.KingBaseCluster.value,
                                   ResourceSubTypeEnum.DB2Cluster.value,
                                   ResourceSubTypeEnum.UBackupAgent.value,
                                   ResourceSubTypeEnum.S_BACKUP_AGENT.value,
                                   ResourceSubTypeEnum.OpenGauss.value,
                                   ResourceSubTypeEnum.DWSCluster.value,
                                   ResourceSubTypeEnum.SQLServerCluster.value,
                                   ResourceSubTypeEnum.OPENSTACK_PROJECT.value,
                                   ResourceSubTypeEnum.GENERAL_DB.value,
                                   ResourceSubTypeEnum.ORACLE_CLUSTER_ENV,
                                   ResourceSubTypeEnum.GOLDENDB_CLUSTER.value,
                                   ResourceSubTypeEnum.TDSQL_CLUSTER.value,
                                   ResourceSubTypeEnum.OCEANBASE_CLUSTER.value,
                                   ResourceSubTypeEnum.HCS_GAUSSDB_PROJECT.value,
                                   ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT.value,
                                   ResourceSubTypeEnum.vCenter.value,
                                   ResourceSubTypeEnum.ESXi.value,
                                   ResourceSubTypeEnum.HYPER_V_CLUSTER,
                                   ResourceSubTypeEnum.HYPER_V_SCVMM,
                                   ResourceSubTypeEnum.HYPER_V_HOST,
                                   ResourceSubTypeEnum.FUSION_ONE_COMPUTE,
                                   ResourceSubTypeEnum.SAP_ON_ORACLE,
                                   ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE.value,
                                   ResourceSubTypeEnum.EXCHANGE_ONLINE.value,
                                   ResourceSubTypeEnum.EXCHANGE_ONLINE_BACKUP_SET.value
                                   )
    def all_sub_resource(self, resource: ResourceTable, session):
        self.check_uuid(resource.uuid)
        sqlstr = "WITH RECURSIVE tree(uuid, parent_uuid) as ( " \
                 "select r1.uuid, r1.parent_uuid from resources r1 " \
                 "where r1.uuid=\'" + resource.uuid + "\'" \
                 "union all select r2.uuid, r2.parent_uuid from resources r2, tree r0" \
                 " where r2.parent_uuid = r0.uuid ) " \
                 " select uuid from tree"
        result = session.execute(sqlstr).fetchall()
        list_uuid = []
        for uuid_obj in result:
            self.is_current_resource_bounded_sla(uuid_obj[0], session)
            list_uuid.append(uuid_obj[0])
        return list_uuid
