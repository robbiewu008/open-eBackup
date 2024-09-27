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
package openbackup.mongodb.protection.access.enums;

import java.util.HashMap;
import java.util.Map;

/**
 * MongoDB集群类型
 *
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-07
 */
public enum MongoDBClusterTypeEnum {
    // 主从
    MASTER_SLAVE("0"),

    // 副本
    REPLICATION("1"),

    // shard
    SHARD("2"),

    // 单机
    SINGLE("3");

    private static final Map<String, MongoDBClusterTypeEnum> MONGODB_CLUSTER_MAP = new HashMap<>(
        MongoDBClusterTypeEnum.values().length);

    static {
        for (MongoDBClusterTypeEnum each : MongoDBClusterTypeEnum.values()) {
            MONGODB_CLUSTER_MAP.put(each.getType(), each);
        }
    }

    private final String type;

    MongoDBClusterTypeEnum(String type) {
        this.type = type;
    }

    /**
     * 根据集群类型获取到对应的部署类型
     *
     * @param type 集群节点类型
     * @return 部署类型
     */
    public static MongoDBClusterTypeEnum getMongoDBClusterType(String type) {
        return MONGODB_CLUSTER_MAP.get(type);
    }

    public String getType() {
        return type;
    }
}
