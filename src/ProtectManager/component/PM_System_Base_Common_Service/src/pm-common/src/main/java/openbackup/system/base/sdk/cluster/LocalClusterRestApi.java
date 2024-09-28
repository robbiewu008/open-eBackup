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
package openbackup.system.base.sdk.cluster;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.alarm.model.ClusterAlarmsInfo;
import openbackup.system.base.sdk.cluster.model.ClusterCapacityInfo;
import openbackup.system.base.sdk.cluster.model.ClusterJobsInfo;
import openbackup.system.base.sdk.cluster.model.ClusterResourcesInfo;
import openbackup.system.base.sdk.cluster.model.ClusterSlaComplianceInfo;
import openbackup.system.base.sdk.cluster.model.TargetClusterRequest;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestParam;

import java.net.URI;

/**
 * Local Cluster Rest Api
 *
 */
@FeignClient(name = "LocalClusterApi", url = "${service.url.pm-system-base}/v1",
    configuration = CommonFeignConfiguration.class)
public interface LocalClusterRestApi {
    /**
     * 获取目标集群资源信息
     *
     * @param uri uri
     * @param token token
     * @return ClusterResourcesInfo
     */
    @ExterAttack
    @GetMapping("/v1/resource/protection/summary")
    ClusterResourcesInfo getClusterResources(URI uri, @RequestHeader(name = "x-auth-token") String token);

    /**
     * 获取目标集群任务信息
     *
     * @param uri uri
     * @param token token
     * @param startTime startTime
     * @param endTime endTime
     * @return ClusterJobsInfo
     */
    @ExterAttack
    @GetMapping("/v1/jobs/summary")
    ClusterJobsInfo getClusterJobs(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @RequestParam("startTime") Long startTime, @RequestParam("endTime") Long endTime);

    /**
     * 获取目标集群告警信息
     *
     * @param uri uri
     * @param token token
     * @return ClusterAlarmsInfo
     */
    @ExterAttack
    @GetMapping("/v2/alarms/node/count")
    ClusterAlarmsInfo getClusterAlarms(URI uri, @RequestHeader(name = "x-auth-token") String token);

    /**
     * 获取目标集群容量信息
     *
     * @param uri uri
     * @param token token
     * @return ClusterCapacityInfo
     */
    @ExterAttack
    @GetMapping("/v1/clusters/capacity")
    ClusterCapacityInfo getClusterCapacity(URI uri, @RequestHeader(name = "x-auth-token") String token);

    /**
     * 获取目标集群Sla信息
     *
     * @param uri uri
     * @param token token
     * @return ClusterSlaComplianceInfo
     */
    @ExterAttack
    @GetMapping("/v1/protected-objects/sla-compliance")
    ClusterSlaComplianceInfo getClusterSlaCompliance(URI uri, @RequestHeader(name = "x-auth-token") String token);

    /**
     * 创建集群
     *
     * @param uri uri
     * @param token token
     * @param targetClusterRequest targetClusterRequest
     * @return cluster id
     */
    @ExterAttack
    @PostMapping ("/v1/clusters")
    int createTargetCluster(URI uri, @RequestHeader(name = "x-auth-token") String token, @RequestBody
    TargetClusterRequest targetClusterRequest);
}
