/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Builder;
import lombok.Data;

/**
 * 功能描述
 *
 * @author w00504341
 * @since 2023-02-28
 */
@Builder
@Data
public class CleanAgentLogReq {
    /**
     * agent记录ID
     */
    private String id;
}
