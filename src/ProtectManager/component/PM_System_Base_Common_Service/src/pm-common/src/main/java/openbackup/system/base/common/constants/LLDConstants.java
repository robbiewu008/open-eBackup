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
 * LLD常量
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-20
 */
public class LLDConstants {
    /**
     * LLD文件的后缀
     */
    public static final String LLD_SUFFIX = ".xls";

    /**
     * E6000LLDConstants
     */
    public static class E6000LLDConstants {
        /**
         * lld所在的sheet的index
         */
        public static final int SHEET_INDEX = 9;

        /**
         * 配置的起始行
         */
        public static final int CONFIG_STRAT_ROW_INDEX = 3;

        /**
         * 设备名称所在的cell的index
         */
        public static final int DEVICE_CELL_INDEX = 1;

        /**
         * 网络类型所在的cell的index
         */
        public static final int NETWORK_TYPE_CELL_INDEX = 2;

        /**
         * 管理ip所在的cell的index
         */
        public static final int MANAGE_IP_CELL_INDEX = 3;

        /**
         * 端口所在的cell的index
         */
        public static final int PORT_CELL_INDEX = 4;

        /**
         * 业务ip所在的cell的index
         */
        public static final int BUSINESS_IP_CELL_INDEX = 5;

        /**
         * E6000_MANAGER_IP_INIT_BY_LLD_SYSTEM_LABEL
         */
        public static final String E6000_MANAGER_IP_INIT_BY_LLD_SYSTEM_LABEL =
            "e6000_manager_ip_init_by_lld_system_label";

        /**
         * E6000_NETWORK_TYPE_INIT_BY_LLD_SYSTEM_LABEL
         */
        public static final String E6000_NETWORK_TYPE_INIT_BY_LLD_SYSTEM_LABEL =
            "e6000_network_type_init_by_lld_system_label";

        /**
         * E6000_IP_ADDRESS_INIT_BY_LLD_SYSTEM_LABEL
         */
        public static final String E6000_IP_ADDRESS_INIT_BY_LLD_SYSTEM_LABEL =
            "e6000_ip_address_init_by_lld_system_label";

        /**
         * E6000_IFACENAME_INIT_BY_LLD_SYSTEM_LABEL
         */
        public static final String E6000_IFACENAME_INIT_BY_LLD_SYSTEM_LABEL =
            "e6000_ifacename_init_by_lld_system_label";
    }
}
