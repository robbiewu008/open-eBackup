/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.license.module;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * dorado activelicense 接口返回内容
 *
 * @author g00500588
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/6/8
 */
@Getter
@Setter
public class ActiveLicenseRes {
    @JsonProperty("LicenseResource")
    private List<LicenseResource> licenseResource;
    @JsonProperty("bomCodes")
    private List<BomCode> bomCodes;
}
