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

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.PropertiesUtil;
import openbackup.system.base.util.SpringBeanUtils;

import org.apache.commons.lang3.StringUtils;

import java.io.File;

/**
 * 获取集群配置
 *
 */
@Slf4j
public class BackupClusterConfigUtil {
    /**
     * 查询configMap中的主节点的Esn。
     *
     * @return configMap中的主节点的Esn
     */
    public static String getBackupClusterEsn() {
        return PropertiesUtil.fileToString(
                BackupClusterConfigConstants.OPT_CONFIG + File.separator + BackupClusterConfigConstants.CLUSTER_ESN);
    }

    /**
     * 查询configMap中的role
     *
     * @return configMap中的role
     */
    public static String getBackupClusterRole() {
        DeployTypeService deployTypeService = SpringBeanUtils.getBean(DeployTypeService.class);
        if (deployTypeService.isE1000()) {
            return "";
        }
        return PropertiesUtil.fileToString(
                BackupClusterConfigConstants.OPT_CONFIG + File.separator + BackupClusterConfigConstants.CLUSTER_ROLE);
    }

    /**
     * 查看当前节点是否是主节点
     *
     * @return 是否是主节点
     */
    public static boolean isMasterCluster() {
        String role = getBackupClusterRole();
        // 不组成集群，认定为主节点
        if (StringUtils.equals(role, StringUtils.EMPTY)) {
            return true;
        }
        String roleType = ClusterEnum.BackupRoleTypeEnum.getBackupRoleTypeByRoleType(role);
        return StringUtils.equalsAnyIgnoreCase(roleType, ClusterEnum.BackupRoleTypeEnum.PRIMARY.getBackupRoleType());
    }

    /**
     * 查看当前节点是否是备节点
     *
     * @return 是否是主节点
     */
    public static boolean isStandbyCluster() {
        String role = getBackupClusterRole();
        // 不组成集群，认定为主节点
        if (StringUtils.equals(role, StringUtils.EMPTY)) {
            return false;
        }
        String roleType = ClusterEnum.BackupRoleTypeEnum.getBackupRoleTypeByRoleType(role);
        return StringUtils.equalsAnyIgnoreCase(roleType, ClusterEnum.BackupRoleTypeEnum.STANDBY.getBackupRoleType());
    }
}
