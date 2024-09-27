/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.database.base.plugin.enums;

/**
 * 数据库节点类型
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/12
 */
public enum NodeType {
    /**
     * 主节点
     */
    MASTER("1"),

    /**
     * 备节点
     */
    SLAVE("2");

    private final String nodeType;

    /**
     * 构造方法
     *
     * @param nodeType 节点类型
     */
    NodeType(String nodeType) {
        this.nodeType = nodeType;
    }

    public String getNodeType() {
        return nodeType;
    }
}
