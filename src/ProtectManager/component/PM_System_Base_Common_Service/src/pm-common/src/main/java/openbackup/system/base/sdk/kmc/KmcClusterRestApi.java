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

import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestPart;
import org.springframework.web.multipart.MultipartFile;

import java.net.URI;

/**
 * KmcClusterRestApi
 *
 * @author z00842230
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-10-12
 */
public interface KmcClusterRestApi {
    /**
     * 同步kmc文件
     *
     * @param uri target url addr
     * @param token token
     * @param file 从主节点获取的kmc文件
     */
    @ExterAttack
    @PostMapping(value = "/v1/kmc/backup/sync-kmc", consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    void syncKmcFiles(URI uri, @RequestHeader(name = "x-auth-token") String token,
                      @RequestPart("kmc") MultipartFile file);

    /**
     * 回退kmc文件
     *
     * @param uri target url addr
     * @param token token
     */
    @ExterAttack
    @PostMapping(value = "/v1/kmc/backup/rollback-kmc")
    void rollbackKmcFiles(URI uri, @RequestHeader(name = "x-auth-token") String token);
}
