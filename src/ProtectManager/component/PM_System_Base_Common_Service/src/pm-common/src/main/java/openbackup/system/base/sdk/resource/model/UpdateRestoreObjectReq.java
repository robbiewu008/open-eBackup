/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 批量更新资源可恢复情况请求体
 *
 * @author s30031954
 * @version [OceanProtect X8000 1.5.0]
 * @since 2024-04-11
 */
@Getter
@Setter
public class UpdateRestoreObjectReq {
    /**
     * 资源id列表
     */
    private List<String> resourceIds;

    /**
     * 当前资源是否支持恢复，true: 支持恢复， false:不支持恢复
     */
    private String isAllowRestore;
}
