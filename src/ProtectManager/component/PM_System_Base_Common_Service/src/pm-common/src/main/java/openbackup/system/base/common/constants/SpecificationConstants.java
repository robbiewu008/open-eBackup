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
 * 规格常量
 *
 * @author w00607005
 * @since 2023-08-04
 */
public class SpecificationConstants {
    /**
     * 集群规格
     */
    public static class Cluster {
        /**
         * 本地集群添加的复制或防病毒外部集群最大数量
         */
        public static final int MAX_REP_TARGET_CLUSTER_COUNT = 4;

        /**
         * 本地集群添加的被管理集群最大数量
         */
        public static final int MAX_MNG_TARGET_CLUSTER_COUNT = 32;

        /**
         * 备份存储集群数量
         */
        public static final int BACKUP_CLUSTER_COUNT = 128 * 31;

        /**
         * 作为外部集群，最多同时被其他集群接入的个数
         */
        public static final int TARGET_CLUSTER_MAX_ADDED_COUNT = 64;

        /**
         * 备份成员节点的最大数量
         */
        public static final int MAX_BACKUP_MEMBER_CLUSTER_COUNT = 31;
    }
}
