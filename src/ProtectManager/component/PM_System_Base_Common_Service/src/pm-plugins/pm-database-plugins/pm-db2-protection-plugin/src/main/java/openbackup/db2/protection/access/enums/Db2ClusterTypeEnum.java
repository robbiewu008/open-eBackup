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
package openbackup.db2.protection.access.enums;

import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;

import java.util.Arrays;

/**
 * db2集群类型
 *
 */
public enum Db2ClusterTypeEnum {
    SINGLE("single"),

    DPF("dpf"),

    POWER_HA("powerHA"),

    HADR("hadr"),

    RHEL_HA("rhelHA");

    Db2ClusterTypeEnum(String type) {
        this.type = type;
    }

    private final String type;

    public String getType() {
        return type;
    }

    /**
     * 根据type获取到对应的枚举
     *
     * @param type 枚举值
     * @return enum
     */
    public static Db2ClusterTypeEnum getByType(String type) {
        return Arrays.stream(Db2ClusterTypeEnum.values())
            .filter(location -> location.type.equals(type))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }

    /**
     * 根据集群类型获取到对应的部署类型
     *
     * @param clusterType 集群类型
     * @return 部署类型
     */
    public static String getDeployType(String clusterType) {
        Db2ClusterTypeEnum clusterTypeEnum = Db2ClusterTypeEnum.getByType(clusterType);
        switch (clusterTypeEnum) {
            case DPF:
                return DatabaseDeployTypeEnum.AP.getType();
            case POWER_HA:
            case RHEL_HA:
                return DatabaseDeployTypeEnum.SHARDING.getType();
            case HADR:
                return DatabaseDeployTypeEnum.AA.getType();
            default:
                return DatabaseDeployTypeEnum.SINGLE.getType();
        }
    }
}
