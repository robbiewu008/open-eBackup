/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Data;

/**
 * The LogCollectResponse
 *
 * @author w00504341
 * @since 2023-01-19
 */
@Data
public class CollectAgentLogRsp {
    /**
     * 记录ID
     */
    private String id;

    /**
     * 文件后缀
     */
    private String format;
}
