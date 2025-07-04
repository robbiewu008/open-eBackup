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
package com.huawei.emeistor.console.service;

/**
 * rsa公私钥对生成接口层
 *
 */
public interface RsaService {
    /**
     * 公钥私钥生成
     *
     * @param isOverwrite 是否覆盖已有的
     */
    void generateKeyPair(boolean isOverwrite) ;

    /**
     * 解密操作
     *
     * @param encrytedText 加密内容
     * @return 解密内容
     */
    String decrypt(String encrytedText);

    /**
     * 查询公钥
     *
     * @return 公钥
     */
    String queryPublicKey();
}
