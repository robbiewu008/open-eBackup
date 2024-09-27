/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
 * @author p30001902
 * @since 2020-11-19
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
