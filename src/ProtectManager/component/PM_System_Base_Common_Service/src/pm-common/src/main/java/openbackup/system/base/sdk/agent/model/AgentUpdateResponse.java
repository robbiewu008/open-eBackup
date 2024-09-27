/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.agent.model;

import lombok.Data;

/**
 * 功能描述
 *
 * @author s00455050
 * @since 2021-08-06
 */
@Data
public class AgentUpdateResponse {
    /**
     * -1:未返回值；0：失败；1：成功；2：中间状态；8：异常状态；9：初始状态
     */
    private int revStatus = -1;
}
