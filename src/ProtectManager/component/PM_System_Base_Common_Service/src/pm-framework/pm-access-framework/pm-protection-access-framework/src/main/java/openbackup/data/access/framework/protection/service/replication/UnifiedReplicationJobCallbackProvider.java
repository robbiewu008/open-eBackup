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
package openbackup.data.access.framework.protection.service.replication;

import openbackup.data.protection.access.provider.sdk.job.JobCallbackProvider;
import openbackup.system.base.common.constants.RedisConstants;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Arrays;

/**
 * 强制复制任务中止回调
 *
 */
@Slf4j
@Component
public class UnifiedReplicationJobCallbackProvider implements JobCallbackProvider {
    @Autowired
    private RedissonClient redissonClient;

    @Override
    public boolean applicable(String jobType) {
        return Arrays.asList(JobTypeEnum.COPY_REPLICATION.getValue())
                .contains(jobType);
    }

    @ExterAttack
    @Override
    public void doCallback(JobBo job) {
        RMap<String, String> context = redissonClient.getMap(job.getJobId(), StringCodec.INSTANCE);
        cleanTargetClusterRelatedTaskInfo(context);
    }

    /**
     * clean Target Cluster Related Task Info
     *
     * @param context context
     */
    @ExterAttack
    public void cleanTargetClusterRelatedTaskInfo(RMap<String, String> context) {
        TargetClusterVo targetCluster =
                JSONObject.fromObject(context.get("target_cluster")).toBean(TargetClusterVo.class);
        RMap<String, String> map =
                redissonClient.getMap(
                        RedisConstants.TARGET_CLUSTER_RELATED_TASKS + targetCluster.getClusterId(),
                        StringCodec.INSTANCE);
        String jobId = context.get("job_id");
        map.remove(jobId);
    }
}
