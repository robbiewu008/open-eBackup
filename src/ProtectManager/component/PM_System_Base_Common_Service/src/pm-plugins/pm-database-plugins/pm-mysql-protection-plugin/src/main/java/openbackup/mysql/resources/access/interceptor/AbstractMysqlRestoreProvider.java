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

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.database.base.plugin.utils.ProtectionTaskUtils;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.util.BeanTools;

import java.util.Collections;
import java.util.Map;
import java.util.Optional;

/**
 * mysql恢复抽象类
 *
 */
@Slf4j
public abstract class AbstractMysqlRestoreProvider extends AbstractDbRestoreInterceptorProvider {
    private final CopyRestApi copyRestApi;
    private final MysqlBaseService mysqlBaseService;

    /**
     * 构造器
     *
     * @param copyRestApi 副本restApi
     * @param mysqlBaseService mysql应用基本的Service
     */
    public AbstractMysqlRestoreProvider(CopyRestApi copyRestApi, MysqlBaseService mysqlBaseService) {
        this.copyRestApi = copyRestApi;
        this.mysqlBaseService = mysqlBaseService;
    }

    /**
     * 恢复任务前，必须保证数据库已停止，这个由用户手动去停止数据库
     *
     * 1. 在任务下发前，做连通性检查，必须保证无法连接mysql
     *
     * @param task 恢复任务task
     */
    @Override
    protected void checkConnention(RestoreTask task) {
        log.info("mysql restore no need check connection.");
    }

    @Override
    public void restoreTaskCreationPreCheck(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        // 恢复校验
        checkRestore(task, copy);
    }

    /**
     * mysql base supplyRestoreTask
     *
     * @param task 恢复对象
     * @return RestoreTask
     */
    @Override
    public RestoreTask supplyRestoreTask(RestoreTask task) {
        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);

        // 设置恢复模式
        ProtectionTaskUtils.setRestoreMode(task, copyRestApi);

        // 从副本里查询SLA里的并发数，设置到恢复的高级参数里
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        CopyBo copyBo = new CopyBo();
        BeanTools.copy(copy, copyBo);
        task.getAdvanceParams().put(DatabaseConstants.SLA_KEY, copyBo.getSlaProperties());

        log.info("mysql restore check success.");
        return task;
    }

    /**
     * 恢复校验 恢复不支持跨操作系统，跨MySQL部署模式，跨版本，跨MySQL以及MariaDB。
     * 并且不能重命名为系统数据库且不能包含必然失败的特殊字符。后台需要能防住API调用
     *
     * @param task 目标参数
     * @param copy 源
     */
    private void checkRestore(RestoreTask task, Copy copy) {
        // 数据获取
        TaskResource targetResource = task.getTargetObject();
        JSONObject resourceJson = JSONObject.fromObject(copy.getResourceProperties());
        Map<String, String> extendInfo = Optional.ofNullable(
                resourceJson.getJSONObject(DatabaseConstants.EXTEND_INFO).toMap(String.class))
            .orElse(Collections.emptyMap());
        Map<String, String> targetResourceExtendInfo = Optional.ofNullable(targetResource.getExtendInfo())
            .orElse(Collections.emptyMap());

        // 拦截MySQL部署的操作系统
        mysqlBaseService.checkDeployOperatingSystem(extendInfo, targetResourceExtendInfo);

        // 拦截部署模式 subType类型一致
        mysqlBaseService.checkSubType(copy, targetResource);

        // clusterType一致
        mysqlBaseService.checkClusterType(extendInfo, targetResourceExtendInfo);

        // 拦截版本不一致,跨MySQL以及MariaDB
        mysqlBaseService.checkVersion(targetResource, resourceJson);

        // 拦截数据库名字
        mysqlBaseService.checkNewDatabaseName(task);
    }
}
