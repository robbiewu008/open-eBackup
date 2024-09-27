/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.goldendb.protection.access.dto.instance;

import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 功能描述 GoldenDB分片
 *
 * @author s30036254
 * @since 2023-02-14
 */
@NoArgsConstructor
@Data
public class Group {
    /**
     * groupId 分片id
     */
    private String groupId;

    /**
     * mysqlNodes mysql节点
     */
    private List<MysqlNode> mysqlNodes;
}
