/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.mysql.resources.access.interceptor;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.sdk.copy.CopyRestApi;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * mysql单实例恢复任务下发provider
 *
 */
@Slf4j
@Component
public class MysqlSingleInstanceRestoreProvider extends AbstractMysqlRestoreProvider {
    private final MysqlBaseService mysqlBaseService;

    private final EncryptorService encryptorService;

    private final ResourceService resourceService;

    /**
     * 构造方法
     *
     * @param mysqlBaseService mysqlBaseService
     * @param copyRestApi 副本rest api
     * @param encryptorService encryptorService
     * @param resourceService resourceService
     */
    public MysqlSingleInstanceRestoreProvider(MysqlBaseService mysqlBaseService, CopyRestApi copyRestApi,
        EncryptorService encryptorService, ResourceService resourceService) {
        super(copyRestApi, mysqlBaseService);
        this.mysqlBaseService = mysqlBaseService;
        this.encryptorService = encryptorService;
        this.resourceService = resourceService;
    }

    /**
     * 数据库各自应用信息
     *
     * @param task RestoreTask
     * @return RestoreTask
     */
    @Override
    public RestoreTask supplyRestoreTask(RestoreTask task) {
        RestoreTask mysqlRestoreTask = super.supplyRestoreTask(task);

        // 获取单实例的version，设置到保护对象的extendInfo里
        ProtectedResource instanceRes =
                mysqlBaseService.getResource(mysqlRestoreTask.getTargetObject().getUuid());
        mysqlRestoreTask.getTargetObject().setExtendInfo(
                mysqlBaseService.supplyExtendInfo(instanceRes.getVersion(),
                mysqlRestoreTask.getTargetObject().getExtendInfo()));

        mysqlRestoreTask.getTargetEnv().getExtendInfo()
                .put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        ProtectedResource resource = mysqlBaseService.getResource(mysqlRestoreTask.getTargetObject().getUuid());
        mysqlRestoreTask.getTargetObject().setAuth(resource.getAuth());

        // 设置agents
        ProtectedEnvironment agentEnv =
                mysqlBaseService.getEnvironmentById(mysqlRestoreTask.getTargetObject().getParentUuid());
        List<Endpoint> agents = Collections.singletonList(mysqlBaseService.getAgentEndpoint(agentEnv));
        mysqlRestoreTask.setAgents(agents);

        // 设置nodes
        mysqlBaseService.supplyNodes(mysqlRestoreTask);

        // 更新node auth
        mysqlBaseService.setNodesAuth(mysqlRestoreTask.getTargetEnv().getNodes(),
                Collections.singletonList(instanceRes));

        // 解密dataDir
        mysqlRestoreTask.getTargetObject().getExtendInfo().put(DatabaseConstants.DATA_DIR,
                encryptorService.decrypt(mysqlRestoreTask.getTargetObject()
                        .getExtendInfo().get(DatabaseConstants.DATA_DIR)));

        // 解密log bin日志文件路径
        mysqlRestoreTask.getTargetObject().getExtendInfo().put(MysqlConstants.LOG_BIN_INDEX_PATH,
                encryptorService.decrypt(mysqlRestoreTask.getTargetObject()
                        .getExtendInfo().get(MysqlConstants.LOG_BIN_INDEX_PATH)));
        return mysqlRestoreTask;
    }

    /**
     * 判断目标资源是否可以应用此拦截器
     *
     * @param subType 目标资源subType
     * @return 是否可以应用此拦截器
     */
    @Override
    public boolean applicable(String subType) {
        return MysqlResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType().equals(subType);
    }

    /**
     * 查询单实例下面的所有数据库资源，包含它本身，一起设置它们下一次备份必须是全量备份
     *
     * @param task 恢复对象
     * @return 这个对象恢复后，要设置哪些关联的对象下一次备份必须是全量备份
     */
    @Override
    protected List<String> findAssociatedResourcesToSetNextFull(RestoreTask task) {
        List<String> associatedResources = new ArrayList<>(resourceService.queryRelatedResourceUuids(
                task.getTargetObject().getUuid(), new String[]{}));
        associatedResources.add(task.getTargetObject().getUuid());
        log.info("set mysql associated resources: {} next backup is full.", String.join(",", associatedResources));
        return associatedResources;
    }
}
