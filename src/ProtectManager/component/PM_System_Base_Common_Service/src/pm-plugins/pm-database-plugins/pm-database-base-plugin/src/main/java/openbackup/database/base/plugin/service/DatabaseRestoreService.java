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
package openbackup.database.base.plugin.service;

/**
 * 数据库恢复服务
 *
 */
public interface DatabaseRestoreService {
    /**
     * 检查恢复的部署操作系统
     *
     * @param sourceDeployOs 源端部署系统
     * @param targetDeployOs 目标端部署系统
     */
    void checkDeployOperatingSystem(String sourceDeployOs, String targetDeployOs);

    /**
     * 检验恢复的资源类型
     *
     * @param sourceSubType 源端资源类型
     * @param targetSubType 目标端资源类型
     */
    void checkResourceSubType(String sourceSubType, String targetSubType);

    /**
     * 检验恢复的集群类型
     *
     * @param sourceClusterType 源端集群类型
     * @param targetClusterType 目标端集群类型
     */
    void checkClusterType(String sourceClusterType, String targetClusterType);

    /**
     * 检验version是否一致
     *
     * @param sourceVersion 源端版本
     * @param targetVersion 目标端版本
     */
    void checkVersion(String sourceVersion, String targetVersion);
}
