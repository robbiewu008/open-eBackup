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
package openbackup.data.access.framework.protection.listener.v1.job;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.util.EngineUtil;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.sdk.job.model.JobBo;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.annotation.KafkaListener;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * 停止任务的代理类
 *
 */
@Slf4j
@Component
public class JobListener {
    private static final String KAFKA_GROUP_JOB_STOP = "job.stop";

    @Autowired
    private ProviderManager providerManager;

    /**
     * Consume topic message of stopping task from kafka
     *
     * @param jobListStr msg
     * @param acknowledgment acknowledgment
     */
    @ExterAttack
    @KafkaListener(groupId = KAFKA_GROUP_JOB_STOP, topics = TopicConstants.JOB_STOP_TOPIC, autoStartup = "false")
    public void stopJob(String jobListStr, Acknowledgment acknowledgment) {
        // 转换成任务列表
        List<JobBo> jobBosList = JSONArray.toCollection(JSONArray.fromObject(jobListStr), JobBo.class);

        // 根据资源类型调用不同的provider处理
        for (JobBo jobBo : jobBosList) {
            String resourceSubType = jobBo.getSourceSubType().getType();
            String engineTaskTypeKey = EngineUtil.getEngineTaskTypeKey(resourceSubType, jobBo.getType());
            JobProvider jobProvider = providerManager.findProvider(JobProvider.class, engineTaskTypeKey);
            jobProvider.stopJob(jobBo.getAssociativeId());
        }
        acknowledgment.acknowledge();
    }
}
