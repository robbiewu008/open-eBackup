/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Builder;
import lombok.Data;

/**
 * The UpdateAgentConfigReq
 *
 * @author w00504341
 * @since 2023-01-19
 */
@Builder
@Data
public class UpdateAgentLevelReq {
    /**
     * 日志级别
     */
    private int level;
}
