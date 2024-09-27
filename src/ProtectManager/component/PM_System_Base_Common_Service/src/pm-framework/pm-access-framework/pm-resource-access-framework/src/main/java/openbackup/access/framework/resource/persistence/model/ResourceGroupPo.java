/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.access.framework.resource.persistence.model;

import openbackup.system.base.query.PageQueryConfig;

import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

/**
 * ResourceGroup Po
 *
 * @author c00631681
 * @since 2024-1-18
 */
@TableName("T_RESOURCE_GROUP")
@Getter
@Setter
@PageQueryConfig(
        conditions = {"path", "source_sub_type", "%name%", "%uuid%", "protection_status"},
        orders = {"created_time"})
public class ResourceGroupPo {
    /**
     * 资源组UUID
     */
    @TableId
    private String uuid;

    /**
     * 资源组名称
     */
    private String name;

    /**
     * 资源路径
     */
    private String path;

    /**
     * 资源类别
     */
    private String sourceType;

    /**
     * 资源子类
     */
    private String sourceSubType;

    /**
     * 创建时间
     */
    private Timestamp createdTime;

    /**
     * 扩展信息
     */
    private String extendStr;

    /**
     * 资源组所属的用户
     */
    private String userId;

    /**
     * 资源组保护状态
     */
    private Integer protectionStatus;

    /**
     * 资源组所属范围资源uuid
     */
    private String scopeResourceId;
}