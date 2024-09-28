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
package openbackup.system.base.sdk.auth.api;

import openbackup.system.base.sdk.auth.model.DmeToken;

/**
 * 调用DME平台的Api
 *
 */
public interface DmeTokenApi {
    /**
     * IAM 校验DME token是否正确，正确则解析
     *
     * @param subjectToken 待校验的token
     * @return DmeToken
     */
    DmeToken verifyAuthToken(String subjectToken);
}
