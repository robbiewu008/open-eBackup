/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * PM向Agent查询应用详细信息(V2接口)的请求对象
 *
 * @author 30009433
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-19
 */
@Getter
@Setter
public class ListResourceV2Req {
    private int pageNo;

    private int pageSize;

    private String conditions;

    private List<String> orders;

    private AppEnv appEnv;

    private List<Application> applications;
}