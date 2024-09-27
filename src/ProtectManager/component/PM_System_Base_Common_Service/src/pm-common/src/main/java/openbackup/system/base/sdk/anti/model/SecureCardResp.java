/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.anti.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 安全卡响应
 *
 * @author j00619968
 * @since 2024-01-23
 */
@Getter
@Setter
public class SecureCardResp {
    /**
     * 安全卡模块是否存在。true：存在；false：不存在
     */
    @JsonProperty("securityModuleFeature")
    boolean hasSecurityModuleFeature;

    /**
     * 安全卡模块详细信息。
     */
    @JsonProperty("securityModuleinfo")
    List<SecureCardInfo> securityModuleinfo;
}
