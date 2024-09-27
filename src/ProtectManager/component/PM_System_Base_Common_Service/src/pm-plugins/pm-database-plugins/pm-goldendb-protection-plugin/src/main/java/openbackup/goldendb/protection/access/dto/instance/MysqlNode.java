/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.goldendb.protection.access.dto.instance;

import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述 mysql节点
 *
 * @author s30036254
 * @since 2023-02-14
 */
@NoArgsConstructor
@Data
public class MysqlNode {
    /**
     * uuid唯一id
     */
    private String uuid;

    /**
     * id
     */
    private String id;

    /**
     * name名称
     */
    private String name;

    /**
     * role角色
     */
    private String role;

    /**
     * ip
     */
    private String ip;

    /**
     * port端口
     */
    private String port;

    /**
     * osUser
     */
    private String osUser;

    /**
     * nodeType节点类型
     */
    private String nodeType;

    /**
     * parentUuid agentId
     */
    private String parentUuid;

    /**
     * 连接状态
     */
    private String linkStatus;

    /**
     * 分组
     */
    private String group;

    /**
     * agent
     */
    private String parent;

    /**
     * agent name
     */
    private String parentName;
}
