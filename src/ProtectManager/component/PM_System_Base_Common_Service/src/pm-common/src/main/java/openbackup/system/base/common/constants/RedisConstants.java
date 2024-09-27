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
package openbackup.system.base.common.constants;

/**
 * Redis Constants
 *
 * @author l00272247
 * @since 2021-01-04
 */
public interface RedisConstants {
    /**
     * TARGET_CLUSTER_RELATED_TASKS
     */
    String TARGET_CLUSTER_RELATED_TASKS = "target-cluster-related-tasks-";

    /**
     * 同步文件到所有k8s节点的data的key
     */
    String SYNC_FILE_TO_ALL_K8S_NODE_DATA_KEY = "syncFileDataKey";

    /**
     * 同步文件到所有k8s节点的stream name
     */
    String SYNC_FILE_TO_ALL_K8S_NODE_NAME = "syncFileToAllK8sNodeName";
}
