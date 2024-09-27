/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.dto.instance;

import lombok.Data;

import java.util.List;

/**
 * 分布式实例分片和数据节点信息
 *
 * @author z00445440
 * @since 2023-11-14
 */
@Data
public class GroupInfo {
    /**
     * 分布式实例groupId
     */
    private String groupId;

    /**
     * 分片setId
     */
    private List<String> setIds;

    /**
     * 数据节点
     */
    private List<DataNode> dataNodes;
}
