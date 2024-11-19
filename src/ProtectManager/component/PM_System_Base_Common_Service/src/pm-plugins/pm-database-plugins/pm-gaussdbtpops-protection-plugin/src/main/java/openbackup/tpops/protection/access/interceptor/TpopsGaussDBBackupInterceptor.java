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
package openbackup.tpops.protection.access.interceptor;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.backup.v2.PostBackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.system.base.common.constants.FaultEnum;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoInternalAlarm;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.alarm.CommonAlarmService;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;
import openbackup.tpops.protection.access.constant.TpopsGaussDBErrorCode;
import openbackup.tpops.protection.access.provider.TpopsGaussDBAgentProvider;
import openbackup.tpops.protection.access.service.TpopsGaussDBService;
import openbackup.tpops.protection.access.util.TpopsGaussDBClusterUtils;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;

/**
 * TPOPS 备份
 *
 */

@Component
@Slf4j
public class TpopsGaussDBBackupInterceptor extends AbstractDbBackupInterceptor {
    private final TpopsGaussDBService tpopsGaussDbService;

    private final TpopsGaussDBAgentProvider tpopsGaussDBAgentProvider;

    private final ClusterBasicService clusterBasicService;

    private final DeployTypeService deployTypeService;

    private final CommonAlarmService commonAlarmService;

    /**
     * 构造器
     *
     * @param tpopsGaussDbService gaussDBService
     * @param tpopsGaussDBAgentProvider tpopsGaussDBAgentProvider
     * @param clusterBasicService clusterBasicService
     * @param deployTypeService deployTypeService
     * @param commonAlarmService commonAlarmService
     */
    public TpopsGaussDBBackupInterceptor(TpopsGaussDBService tpopsGaussDbService,
        TpopsGaussDBAgentProvider tpopsGaussDBAgentProvider, ClusterBasicService clusterBasicService,
        DeployTypeService deployTypeService, CommonAlarmService commonAlarmService) {
        this.tpopsGaussDbService = tpopsGaussDbService;
        this.tpopsGaussDBAgentProvider = tpopsGaussDBAgentProvider;
        this.clusterBasicService = clusterBasicService;
        this.deployTypeService = deployTypeService;
        this.commonAlarmService = commonAlarmService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType().equals(object);
    }

    /**
     * 设置注册的agent信息到备份任务
     *
     * @param backupTask backupTask
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        // 获取环境ID
        String envId = backupTask.getProtectEnv().getUuid();
        ProtectedEnvironment protectedEnvironment = tpopsGaussDbService.getEnvironmentById(envId);

        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(protectedEnvironment)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .build();
        backupTask.setAgents(tpopsGaussDBAgentProvider.getSelectedAgents(agentSelectParam));
    }

    /**
     * 填充node信息
     *
     * @param backupTask backupTask
     */
    @Override
    protected void supplyNodes(BackupTask backupTask) {
        List<TaskEnvironment> taskEnvironments = tpopsGaussDbService.supplyNodes(backupTask.getProtectEnv().getUuid());
        backupTask.getProtectEnv().setNodes(taskEnvironments);

        // 日志备份加上检查任务类型校验
        checkIsLogBackup(backupTask);
    }

    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        if (!deployTypeService.isE1000()) {
            TpopsGaussDBClusterUtils.addRepositoryEsnAndRole(backupTask.getRepositories().get(0),
                clusterBasicService.getCurrentClusterEsn());
        }

        // 更新环境中的信息
        tpopsGaussDbService.modifyBackupTaskParam(backupTask);
        return backupTask;
    }

    @Override
    public void finalize(PostBackupTask postBackupTask) {
        tpopsGaussDbService.setNextBackupTypeWhenLogBackFail(postBackupTask);

        // 获取副本扩展信息canRestore，日志备份且返回false上报告警
        String copyPropertiesStr = postBackupTask.getCopyInfo().getProperties();
        JSONObject copyProperties = JSONObject.fromObject(copyPropertiesStr);
        String canRestore = copyProperties.getString(TpopsGaussDBConstant.CAN_RESTORE);
        if (BackupTypeEnum.LOG.getAbbreviation() == postBackupTask.getBackupType().getAbbreviation()
            && TpopsGaussDBConstant.FALSE.equals(canRestore)) {
            log.warn("The canRestore is false.");
            // 资源（[实例名]）运行[0-备份]任务[1-部分成功]
            String[] alarmParams = new String[] {postBackupTask.getProtectedObject().getName(), "0", "1"};
            commonAlarmService.generateAlarm(genLogBackupFailedAlarmParam(alarmParams));
        }
    }

    private LegoInternalAlarm genLogBackupFailedAlarmParam(String[] alarmParams) {
        LegoInternalAlarm legoInternalAlarm = new LegoInternalAlarm();
        legoInternalAlarm.setAlarmId(TpopsGaussDBErrorCode.GAUSSDB_LOG_BACKUP_FAILED_ID);
        legoInternalAlarm.setMoName(TpopsGaussDBErrorCode.GAUSSDB_LOG_BACKUP_FAILED_ID);
        legoInternalAlarm.setAlarmParam(alarmParams);
        legoInternalAlarm.setAlarmSequence(IsmNumberConstant.ONE);
        legoInternalAlarm.setAlarmLevel(FaultEnum.AlarmSeverity.MAJOR);
        legoInternalAlarm.setSourceType(FaultEnum.AlarmResourceType.PROTECTION.getValue());
        return legoInternalAlarm;
    }

    /**
     * PM侧校验日志备份，并将校验结果设置到任务的额外参数中
     *
     * @param backupTask 通用备份框架备份参数对象
     */
    private void checkIsLogBackup(BackupTask backupTask) {
        log.info("Check Is LogBackup, TaskType: {}", backupTask.getBackupType());
        if (TpopsGaussDBConstant.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            Map<String, String> advanceParams = backupTask.getAdvanceParams();
            advanceParams.put(TpopsGaussDBConstant.IS_CHECK_BACKUP_JOB_TYPE, "true");
        }
    }
}
