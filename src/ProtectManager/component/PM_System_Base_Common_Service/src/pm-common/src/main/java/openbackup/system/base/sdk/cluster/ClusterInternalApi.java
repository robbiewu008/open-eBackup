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
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.ClusterStorageNodeVo;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * Target cluster service
 *
 */
@FeignClient(name = "TargetClusterApi", url = "${service.url.pm-system-base}/v1",
    configuration = CommonFeignConfiguration.class)
public interface ClusterInternalApi {
    /**
     * get target cluster details
     *
     * @return list
     */
    @ExterAttack
    @GetMapping("/internal/clusters/target-details")
    @ResponseBody
    List<TargetClusterVo> queryTargetClustersDetails();

    /**
     * get target cluster details by cluster id
     *
     * @param clusterId clusterId
     * @return list
     */
    @ExterAttack
    @GetMapping("/internal/clusters/{clusterId}/target-details")
    @ResponseBody
    TargetClusterVo queryTargetClusterDetailsByClusterId(@PathVariable("clusterId") Integer clusterId);

    /**
     * get local storage details
     *
     * @return list
     */
    @ExterAttack
    @GetMapping("/internal/clusters/details")
    @ResponseBody
    ClusterDetailInfo queryClusterDetails();

    /**
     * get local Cluster nodes
     *
     * @return list
     */
    @ExterAttack
    @GetMapping("/internal/clusters/local/nodes")
    @ResponseBody
    List<ClusterStorageNodeVo> queryLocalClusterNodes();
}
