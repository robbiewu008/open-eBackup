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
package openbackup.system.base.sdk.kmc;

import openbackup.system.base.sdk.kmc.model.CiphertextVo;
import openbackup.system.base.sdk.kmc.model.PlaintextVo;

/**
 * 解密api接口
 *
 * @author t00482481
 * @since 2020-07-05
 */
public interface EncryptorRestApi {
    /**
     * 解密
     *
     * @param ciphertextVo 解密对象
     * @return 解密后的产生的对象
     */
    PlaintextVo decrypt(CiphertextVo ciphertextVo);

    /**
     * 解密
     *
     * @param plaintextVo 加密密对象
     * @return 解密后的产生的对象
     */
    CiphertextVo encrypt(PlaintextVo plaintextVo);
}
