/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.dto.instance;

import lombok.Data;

import java.util.List;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-05-23
 */
@Data
public class TdsqlInstance {
    /**
     * id
     */
    private String id;

    /**
     * name
     */
    private String name;

    /**
     * type分布式/非分布式
     */
    private String type;

    /**
     * 关联的集群
     */
    private String cluster;

    /**
     * group 分片
     */
    private List<Group> groups;
}
