/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Data;

import java.util.List;

/**
 * Agent返回的插件数据模型，包括插件名字、插件版本以及插件支持的应用
 *
 * @author w00616953
 * @since 2021-11-30
 */
@Data
public class SupportPluginDto {
    /**
     * 插件名称
     */
    private String pluginName;

    /**
     * 插件版本号
     */
    private String pluginVersion;

    /**
     * 支持应用的版本信息
     */
    private List<SupportApplicationDto> supportApplications;
}
