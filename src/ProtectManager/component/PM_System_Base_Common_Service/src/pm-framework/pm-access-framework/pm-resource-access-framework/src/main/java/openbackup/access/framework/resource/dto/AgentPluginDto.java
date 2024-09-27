/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.dto;

import lombok.Data;

/**
 * Agent Plugin Dto
 *
 * @author fwx1022842
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022/2/28
 */
@Data
public class AgentPluginDto {
    private String bodyErr;

    private long code;

    private String message;
}
