/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.core.entity;

import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Builder;
import lombok.Data;

/**
 * 功能描述
 *
 * @author l30044826
 * @since 2024-01-29
 */
@Data
@Builder
@TableName("COPIES_PROTECTION")
public class CopiesProtectionEntity {
    @TableId
    private String protectedResourceId;

    private String protectedObjectUuid;

    private String protectedSlaId;

    private String protectedSlaName;

    private Boolean protectedStatus;

    private String protectedType;

    private String protectedSubType;

    private String protectedChainId;
}
