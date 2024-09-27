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

import lombok.Data;
import lombok.ToString;

import org.springframework.web.multipart.MultipartFile;

/**
 * ADFS保存配置请求体
 *
 * @author y30021475
 * @since 2023-05-15
 */
@Data
@ToString(exclude = {"clientPwd"})
public class ADFSConfigRequest {
    /**
     * 是否开启ADFS
     */
    private boolean isAdfsEnable;

    private String configName;

    /**
     * ADFS地址
     */
    private String providerUrl;

    /**
     * 客户端id
     */
    private String clientId;

    /**
     * 客户端秘钥
     */
    private String clientPwd;

    /**
     * CA证书
     */
    private MultipartFile caFile;
}
