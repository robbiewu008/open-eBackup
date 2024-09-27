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
 * MongoDB类型集合
 *
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-07
 */
public enum MongoDBNodeTypeEnum {
    /**
     * 单机实例节点
     */
    SINGLE("single"),

    /**
     * 分片集群route节点
     */
    MONGOS("mongos"),

    /**
     * 复制集群节点
     */
    REPLICATION("replication"),
    /**
     * 分片集群分片节点
     */
    SHARD("shard"),

    /**
     * 分片集群配置节点
     */
    CONFIG("config");

    private static final Map<String, MongoDBNodeTypeEnum> MONGODB_NODE_MAP = new HashMap<>(
        MongoDBNodeTypeEnum.values().length);

    static {
        for (MongoDBNodeTypeEnum each : MongoDBNodeTypeEnum.values()) {
            MONGODB_NODE_MAP.put(each.getType(), each);
        }
    }

    private final String type;

    MongoDBNodeTypeEnum(String type) {
        this.type = type;
    }

    /**
     * 根据集群类型获取到对应的部署类型
     *
     * @param type 集群节点类型
     * @return 部署类型
     */
    public static MongoDBNodeTypeEnum getMongoDBNodeType(String type) {
        return MONGODB_NODE_MAP.get(type);
    }

    public String getType() {
        return type;
    }
}
