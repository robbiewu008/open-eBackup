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
package openbackup.data.access.client.sdk.api;

import openbackup.system.base.sdk.copy.model.CopyReplicationImportParam;
import openbackup.system.base.security.exterattack.ExterAttack;

import feign.Headers;
import feign.Param;
import feign.RequestLine;

import org.springframework.web.bind.annotation.RequestBody;

import java.net.URI;

/**
 * Access Point Rest Api
 *
 */
public interface AccessPointRestApi {
    /**
     * import copy
     *
     * @param uri uri
     * @param token token
     * @param importParam import param
     * @return result
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters/copies/import")
    @Headers("x-auth-token: {token}")
    boolean importCopy(URI uri, @Param("token") String token, @RequestBody CopyReplicationImportParam importParam);
}
