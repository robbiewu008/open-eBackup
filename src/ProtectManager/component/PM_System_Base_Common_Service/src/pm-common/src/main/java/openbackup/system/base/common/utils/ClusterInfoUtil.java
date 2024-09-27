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
package openbackup.system.base.common.utils;

import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClustersInfoVo;
import openbackup.system.base.security.callee.CalleeMethod;
import openbackup.system.base.security.callee.CalleeMethods;

import org.springframework.stereotype.Component;

/**
 * 获取当前集群信息工具类
 *
 * @author y30044273
 * @since 2023-09-18
 */
@Component
@CalleeMethods(name = "cluster_info_util", value = {@CalleeMethod(name = "getCurrentClusterInfo")})
public class ClusterInfoUtil {
    private final ClusterNativeApi clusterNativeApi;

    private ClusterInfoUtil(ClusterNativeApi clusterNativeApi) {
        this.clusterNativeApi = clusterNativeApi;
    }

    /**
     * 记录操作事件logging使用
     *
     * @return 当前节点信息
     */
    public ClustersInfoVo getCurrentClusterInfo() {
        return clusterNativeApi.getCurrentClusterVoInfo();
    }
}
