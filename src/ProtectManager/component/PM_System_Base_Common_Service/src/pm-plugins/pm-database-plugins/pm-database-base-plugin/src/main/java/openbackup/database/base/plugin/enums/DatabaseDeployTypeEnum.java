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
package openbackup.database.base.plugin.enums;

import openbackup.system.base.util.EnumUtil;

/**
 * Database应用部署模式枚举
 *
 */
public enum DatabaseDeployTypeEnum {
    /**
     * 单机
     */
    SINGLE("1"),

    /**
     * 互为主
     */
    AA("2"),

    /**
     * 主备
     */
    AP("3"),

    /**
     * 共享
     */
    SHARDING("4"),

    /**
     * 分布式
     */
    DISTRIBUTED("5");

    private static final String SINGLE_DEPLOY_TYPE_LABEL = "database_single_deploy_type_label";

    private static final String AA_DEPLOY_TYPE_LABEL = "database_aa_deploy_type_label";

    private static final String AP_DEPLOY_TYPE_LABEL = "database_ap_deploy_type_label";

    private static final String SHARDING_DEPLOY_TYPE_LABEL = "database_sharding_deploy_type_label";

    private final String type;

    DatabaseDeployTypeEnum(String type) {
        this.type = type;
    }

    public String getType() {
        return type;
    }

    /**
     * get type by value
     *
     * @param deployType deployType
     * @return DeployTypeEnum
     */
    public static DatabaseDeployTypeEnum getEnum(String deployType) {
        return EnumUtil.get(DatabaseDeployTypeEnum.class, DatabaseDeployTypeEnum::getType, deployType);
    }

    /**
     * 获取对应的部署形态label
     *
     * @param deployType 部署形式
     * @return label
     */
    public static String getLabel(String deployType) {
        switch (getEnum(deployType)) {
            case SINGLE:
                return SINGLE_DEPLOY_TYPE_LABEL;
            case AA:
                return AA_DEPLOY_TYPE_LABEL;
            case AP:
                return AP_DEPLOY_TYPE_LABEL;
            case SHARDING:
                return SHARDING_DEPLOY_TYPE_LABEL;
            default:
                return "";
        }
    }
}
