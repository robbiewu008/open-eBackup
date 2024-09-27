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
package openbackup.system.base.common.cluster;

/**
 * 获取configMap中集群配置常量类
 *
 * @author z00613137
 * @since 2023-05-26
 */
public class BackupClusterConfigConstants {
    /**
     * 挂载路径
     */
    public static final String OPT_CONFIG = "/opt/cluster_config";

    /**
     * 节点Esn
     */
    public static final String CLUSTER_ESN = "CLUSTER_ESN";

    /**
     * 节点角色ROLE
     */
    public static final String CLUSTER_ROLE = "CLUSTER_ROLE";

    /**
     * service ip
     */
    public static final String CLUSTER_SERVICE_IPS = "CLUSTER_SERVICE_IPS";
}
