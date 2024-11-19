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
package openbackup.system.base.sdk.cluster.constants;

/**
 * ClusterConstants
 *
 */
public class ClusterSchedulerConstants {
    /**
     * MemberClusterSchedulerConstants
     */
    public static class MemberClusterSchedulerConstants {
        /**
         * 定时监控本地集群状态频率：5分钟
         */
        public static final long FIXED_RATE = 1000 * 60 * 5;

        /**
         * 定时上报本地集群心跳：1分钟
         */
        public static final long HEARTBEAT_FIXED_RATE = 1000 * 60;

        /**
         * 定时删除添加失败的成员节点：2小时
         */
        public static final long CLEAR_FIXED_RATE = 1000 * 60 * 60 * 2;

        /**
         * 定时监控本地集群状态：1分钟
         */
        public static final long STATUS_FIXED_RATE = 1000 * 60;

        /**
         * 超过5分钟认为离线
         */
        public static final long OFF_LINE_TIME = 1000 * 60 * 5;

        /**
         * 定时更新成员device secret：1分钟
         */
        public static final long UPDATE_MEMBER_DEVICE_SECRET_RATE = 1000 * 60;

        /**
         * CLUSTER_OFFLINE_ALARM_ID
         */
        public static final String CLUSTER_OFFLINE_ALARM_ID = "0x131F430001";

        /**
         * MO_IP
         */
        public static final String MO_IP = "127.0.0.1";

        /**
         * NTP_CONFIG_EXISTS_FLAG
         */
        public static final String NTP_CONFIG_EXISTS_FLAG = "NTP_CONFIG_EXISTS_FLAG:";

        /**
         * NTP_SERVER_ENABLED
         */
        public static final String NTP_SERVER_ENABLED = "1";

        /**
         * UPDATE_DEVICE_SECRET_BY_PRODUCT_STORAGE
         */
        public static final String UPDATE_DEVICE_SECRET_BY_PRODUCT_STORAGE =
            "/update_device_secret_by_product_storage_";
    }
}
