/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.base.v2;

import openbackup.data.protection.access.provider.sdk.enums.ClientProtocolTypeEnum;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 数据布局
 *
 * @author t30028453
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-17
 */
@Getter
@Setter
public class BaseDataLayout {
    // 客户端协议类型, 默认使用IP协议
    @JsonProperty("clientProtocolType")
    private Integer clientProtocolType = ClientProtocolTypeEnum.IP.getClientProtocolType();

    // windows任务使用
    @JsonProperty("characterSet")
    private Integer characterSet = 0;
}
