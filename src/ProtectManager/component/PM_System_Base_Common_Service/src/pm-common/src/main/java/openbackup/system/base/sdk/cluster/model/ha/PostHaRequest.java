/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model.ha;

import lombok.Data;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

/**
 * HA后置任务请求体
 *
 * @author w00607005
 * @since 2023-05-22
 */
@Data
public class PostHaRequest {
    /**
     * 操作类型，add、modify、remove
     */
    @NotNull(message = "The type cannot be null. ")
    @Pattern(regexp = "add|modify|remove")
    private String type;

    /**
     * 任务结果，success：成功，fail：失败
     */
    @NotNull(message = "The result cannot be null. ")
    @Pattern(regexp = "success|fail")
    private String result;

    /**
     * 节点角色，PRIMARY：主节点，STANDBY：从节点
     */
    @NotNull(message = "The role cannot be null. ")
    @Pattern(regexp = "primary|standby")
    private String role;
}
