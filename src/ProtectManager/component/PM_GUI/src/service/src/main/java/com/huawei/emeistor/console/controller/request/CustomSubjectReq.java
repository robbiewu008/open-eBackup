/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package com.huawei.emeistor.console.controller.request;

import lombok.Getter;
import lombok.Setter;

import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 自定义subject类
 *
 **/
@Getter
@Setter
public class CustomSubjectReq {
    /**
     * 申请证书的国家代码（两个字母）
     */
    @Pattern(regexp = "^$|^[a-zA-Z]{2}$",
        message = "Country can only contain letters and has a length of 0 or 2")
    private String country;

    /**
     * 申请证书的州或省份名称
     */
    @Size(max = 128)
    @Pattern(regexp = "^$|^[a-zA-Z\\d \\\\.*,_-]*$",
        message = "State can only contain letters, numbers, spaces, \",\", \".\", \"_\", \"-\" and \"*\"")
    private String state;

    /**
     * 申请证书的城市或地区名称
     */
    @Size(max = 128)
    @Pattern(regexp = "^$|^[a-zA-Z\\d \\\\.*,_-]*$",
        message = "City can only contain letters, numbers, spaces, \",\", \".\", \"_\", \"-\" and \"*\"")
    private String city;

    /**
     * 申请证书的组织名称
     */
    @Size(max = 64)
    @Pattern(regexp = "^$|^[a-zA-Z\\d \\\\.*,_-]*$",
        message = "Organization can only contain letters, numbers, spaces, \",\", \".\", \"_\", \"-\" and \"*\"")
    private String organization;

    /**
     * 申请证书的组织单位名称
     */
    @Size(max = 64)
    @Pattern(regexp = "^$|^[a-zA-Z\\d \\\\.*,_-]*$",
        message = "Organization Unit can only contain letters, numbers, spaces, \",\", \".\", \"_\", \"-\" and \"*\"")
    private String organizationUnit;

    /**
     * 公共名称，通常用于标识服务器的域名或主机名
     */
    @Size(max = 64)
    @Pattern(regexp = "^$|^[a-zA-Z\\d \\\\.@*,_-]*$",
        message = "Common Name can only contain letters, numbers, spaces, \",\", \".\", \"_\", \"-\", \"@\" and \"*\"")
    private String commonName;
}

