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
package openbackup.data.access.framework.servitization.util;

import openbackup.data.access.framework.servitization.entity.VpcInfoEntity;
import openbackup.data.access.framework.servitization.service.IVpcService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCancelTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCreateTask;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.model.AgentTypeEnum;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.OpServiceUtil;

import com.alibaba.fastjson.JSON;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * op服务化场景判断
 *
 */
@Slf4j
@Component
@AllArgsConstructor
public class OpServiceHelper {
    private static final String VPC_INFO = "vpc_info";

    private final ResourceService resourceService;

    private final IVpcService vpcService;

    /**
     * isNeedVpcInfo，外置agent才需要vpc信息
     *
     * @param advanceParams advanceParams
     * @param agentEndpoints agentEndpoints
     * @return string vpcId
     */
    private Set<String> getVpcInfo(Map advanceParams, List<Endpoint> agentEndpoints) {
        if (advanceParams != null && advanceParams.containsKey(VPC_INFO)) {
            // 插件已经插入过vpc
            return new HashSet<>();
        }
        if (OpServiceUtil.isHcsService() && CollectionUtils.isNotEmpty(agentEndpoints)) {
            return agentEndpoints.stream()
                .map(Endpoint::getId)
                .map(resUuid -> resourceService.getBasicResourceById(false, resUuid))
                .filter(protectedResource -> ResourceTypeEnum.HOST.getType().equals(protectedResource.get().getType())
                    && AgentTypeEnum.EXTERNAL_AGENT.getValue()
                    .equals(protectedResource.get().getExtendInfoByKey(ResourceConstants.AGENT_TYPE_KEY)))
                .map(protectedResource -> protectedResource.get().getExtendInfoByKey(ResourceConstants.VPC_ID))
                .filter(vpcId -> StringUtils.isNotBlank(vpcId))
                .collect(Collectors.toSet());
        }
        return new HashSet<>();
    }

    /**
     * injectVpcInfo
     *
     * @param backupTask backupTask
     */
    public void injectVpcInfo(BackupTask backupTask) {
        log.info("BackUp check is need vpcInfo:uuid={},subType={},jobId={}", backupTask.getProtectObject().getUuid(),
            backupTask.getProtectObject().getSubType(), backupTask.getTaskId());
        Set<String> vpcIds = getVpcInfo(backupTask.getAdvanceParams(), backupTask.getAgents());
        if (CollectionUtils.isNotEmpty(vpcIds)) {
            log.info("vpcIds={}", StringUtils.join(vpcIds, ","));
            // hcs场景，userId就是projectId
            List<VpcInfoEntity> vpcInfoEntities = vpcService.getVpcByVpcIds(vpcIds);
            Map<String, String> advanceParams = Optional.ofNullable(backupTask.getAdvanceParams())
                .orElse(new HashMap<>());
            advanceParams.put(VPC_INFO, JSON.toJSONString(vpcInfoEntities));
            backupTask.setAdvanceParams(advanceParams);
        }
    }

    /**
     * injectVpcInfoForRestore
     *
     * @param restoreTask restoreTask
     */
    public void injectVpcInfoForRestore(RestoreTask restoreTask) {
        log.info("Restore check is need vpcInfo:jobId={}", restoreTask.getTaskId());
        Set<String> vpcIds = getVpcInfo(restoreTask.getAdvanceParams(), restoreTask.getAgents());
        if (CollectionUtils.isNotEmpty(vpcIds)) {
            log.info("vpcIds={}", StringUtils.join(vpcIds, ","));
            List<VpcInfoEntity> vpcInfoEntities = vpcService.getVpcByVpcIds(vpcIds);
            Map<String, String> advanceParams = Optional.ofNullable(restoreTask.getAdvanceParams())
                .orElse(new HashMap<>());
            advanceParams.put(VPC_INFO, JSON.toJSONString(vpcInfoEntities));
            restoreTask.setAdvanceParams(advanceParams);
        }
    }

    /**
     * 即时挂载，增加vpc信息
     *
     * @param liveMountCreateTask 任务参数
     */
    public void injectVpcInfoForLiveMount(LiveMountCreateTask liveMountCreateTask) {
        log.info("LiveMount check is need vpcInfo:jobId={}", liveMountCreateTask.getTaskId());
        Set<String> vpcIds = getVpcInfo(liveMountCreateTask.getAdvanceParams(), liveMountCreateTask.getAgents());
        if (CollectionUtils.isNotEmpty(vpcIds)) {
            log.info("vpcIds={}", StringUtils.join(vpcIds, ","));
            List<VpcInfoEntity> vpcInfoEntities = vpcService.getVpcByVpcIds(vpcIds);
            Map<String, Object> advanceParams = Optional.ofNullable(liveMountCreateTask.getAdvanceParams())
                .orElse(new HashMap<>());
            advanceParams.put(VPC_INFO, JSON.toJSONString(vpcInfoEntities));
            liveMountCreateTask.setAdvanceParams(advanceParams);
        }
    }

    /**
     * 即时挂载，增加vpc信息
     *
     * @param liveMountCancelTask 任务参数
     */
    public void injectVpcInfoForUnLiveMount(LiveMountCancelTask liveMountCancelTask) {
        log.info("UnLiveMount check is need vpcInfo:jobId={}", liveMountCancelTask.getTaskId());
        Set<String> vpcIds = getVpcInfo(liveMountCancelTask.getAdvanceParams(), liveMountCancelTask.getAgents());
        if (CollectionUtils.isNotEmpty(vpcIds)) {
            log.info("vpcIds={}", StringUtils.join(vpcIds, ","));
            List<VpcInfoEntity> vpcInfoEntities = vpcService.getVpcByVpcIds(vpcIds);
            Map<String, Object> advanceParams = Optional.ofNullable(liveMountCancelTask.getAdvanceParams())
                .orElse(new HashMap<>());
            advanceParams.put(VPC_INFO, JSON.toJSONString(vpcInfoEntities));
            liveMountCancelTask.setAdvanceParams(advanceParams);
        }
    }

    /**
     * 副本校验，添加vpc白名单
     *
     * @param task 任务
     */
    public void injectVpcInfoForCopyVerify(CopyVerifyTask task) {
        log.info("copyVerify check need vpcInfo:taskId={}", task.getTaskId());
        Set<String> vpcIds = getVpcInfo(task.getAdvanceParams(), task.getAgents());
        if (CollectionUtils.isNotEmpty(vpcIds)) {
            log.info("vpcIds={}", StringUtils.join(vpcIds, ","));
            List<VpcInfoEntity> vpcInfoEntities = vpcService.getVpcByVpcIds(vpcIds);
            Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());
            advanceParams.put(VPC_INFO, JSON.toJSONString(vpcInfoEntities));
            task.setAdvanceParams(advanceParams);
        }
    }
}
