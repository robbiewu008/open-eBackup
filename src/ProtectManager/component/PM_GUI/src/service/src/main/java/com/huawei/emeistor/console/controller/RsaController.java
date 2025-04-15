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
package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.controller.response.PublicKeyResponse;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.service.RsaService;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

/**
 * rsa公私钥对生成控制层
 *
 */
@RestController
@RequestMapping(ConfigConstant.CONSOLE)
public class RsaController {
    @Autowired
    private RsaService rsaService;

    /**
     * rsa公私钥对生成入口函数
     *
     * @return 公钥
     */
    @ExterAttack
    @GetMapping("/v1/rsa")
    public PublicKeyResponse getPublicKey() {
        String publicKey = rsaService.queryPublicKey();
        return PublicKeyResponse.builder()
            .publicKey(publicKey != null ? publicKey : "")
            .build();
    }
}
