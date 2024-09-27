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
package openbackup.data.access.framework.protection.listener.v1.archive;

import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.archive.ArchiveTaskManager;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveObject;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveProvider;
import openbackup.data.protection.access.provider.sdk.sla.Policy;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.security.exterattack.ExterAttack;

import com.alibaba.fastjson.JSONObject;
import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;

/**
 * 副本删除任务完成消息监听器
 *
 * @author y00490893
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-07
 */
@Slf4j
@Component
public class ArchiveListener {
    private static final String JOB_STATUS_LOG = "job_status_{payload?.job_status|status}_label";

    private static final String CLOUD_ARCHIVE_PROTOCOL = "2";

    private final ProviderManager providerManager;

    private final RedissonClient redissonClient;

    private final ArchiveTaskManager archiveTaskManager;

    private final FunctionSwitchService functionSwitchService;

    private final UserQuotaManager userQuotaManager;

    private CopyRestApi copyRestApi;

    // 老框架已经支持资源类型列表
    @Value("${supported.resources.archive}")
    private List<String> supportedResources;

    public ArchiveListener(ProviderManager providerManager, RedissonClient redissonClient,
        ArchiveTaskManager archiveTaskManager, FunctionSwitchService functionSwitchService,
        UserQuotaManager userQuotaManager) {
        this.providerManager = providerManager;
        this.redissonClient = redissonClient;
        this.archiveTaskManager = archiveTaskManager;
        this.functionSwitchService = functionSwitchService;
        this.userQuotaManager = userQuotaManager;
    }

    @Autowired
    public void setCopyRestApi(final CopyRestApi copyRestApi) {
        this.copyRestApi = copyRestApi;
    }

    /**
     * Consume restore topic message
     *
     * @param msg archive msg
     * @param acknowledgment Acknowledgment
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.PROTECTION_ARCHIVE, containerFactory = "retryFactory", log = {
            "job_log_protection_archive_execute_label", JOB_STATUS_LOG})
    public void archiving(String msg, Acknowledgment acknowledgment) {
        log.info("ArchiveListener receive topic:{}", TopicConstants.PROTECTION_ARCHIVE);
        if (msg == null) {
            log.info("Archiving msg is null");
            return;
        }
        Map map = JSONObject.parseObject(msg, Map.class);
        String requireId = String.valueOf(map.get("request_id"));
        RMap<String, String> rMap = redissonClient.getMap(requireId, StringCodec.INSTANCE);
        ArchiveObject archiveObject = convertToArchiveObject(rMap);

        // 新老应用下发归档任务前校验是否能够备份及额度
        cloudArchivePreCheck(rMap, archiveObject.getCopyId());
        if (StringUtils.isBlank(archiveObject.getRequestId())) {
            return;
        }
        if (supportedResources.contains(archiveObject.getObjectType())) {
            ArchiveProvider provider = providerManager.findProvider(ArchiveProvider.class,
                    archiveObject.getObjectType());
            provider.archive(archiveObject);
        } else {
            archiveTaskManager.start(archiveObject);
        }
    }

    private void cloudArchivePreCheck(RMap<String, String> rMap, String copyId) {
        Copy copy = copyRestApi.queryCopyByID(copyId);
        if (VerifyUtil.isEmpty(copy)) {
            log.error("Copy is not present when cloudArchivePreCheck!CopyId:{}", copyId);
            return;
        }

        String userId = copy.getUserId();

        // 校验归档额度
        checkArchiveQuota(rMap, userId);
    }

    private void checkArchiveQuota(RMap<String, String> rMap, String userId) {
        // 目前只支持对象存储额度管理
        // 校验对象存储归档额度开关
        Policy policy = openbackup.system.base.common.utils.JSONObject.fromObject(rMap.get("policy"))
            .toBean(Policy.class);
        if (VerifyUtil.isEmpty(policy)) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Policy is Empty!");
        }
        JsonNode protocol = policy.getExtParameters().get("protocol");
        if (!VerifyUtil.isEmpty(protocol) && CLOUD_ARCHIVE_PROTOCOL.equals(protocol.asText())) {
            log.info("Start to checkArchiveQuota!userId:{}", userId);
            userQuotaManager.checkCloudArchiveQuota(userId, null);
        }
    }

    private ArchiveObject convertToArchiveObject(RMap<String, String> rMap) {
        ArchiveObject archiveObject = new ArchiveObject();
        archiveObject.setRequestId(rMap.get("request_id"));
        archiveObject.setCopyId(rMap.get("copy_id"));
        archiveObject.setPolicy(rMap.get("policy"));
        archiveObject.setJobId(rMap.get("job_id"));
        archiveObject.setObjectType(rMap.get("resource_sub_type"));
        return archiveObject;
    }
}
