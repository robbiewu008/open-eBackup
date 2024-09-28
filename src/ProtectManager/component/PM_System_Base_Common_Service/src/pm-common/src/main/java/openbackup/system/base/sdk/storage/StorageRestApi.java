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
package openbackup.system.base.sdk.storage;

import openbackup.system.base.common.model.repository.StorageInfo;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * Storage Rest Api
 *
 */
public interface StorageRestApi {
    /**
     * verify storage ownership
     *
     * @param userId   user id
     * @param uuidList uuid list
     */
    @GetMapping("/internal/product-storages/action/verify")
    @ResponseBody
    void verifyStorageOwnership(@RequestParam("userId") String userId,
        @RequestParam("idList") List<String> uuidList);

    /**
     * 获取存储信息
     *
     * @return 存储信息
     */
    @ExterAttack
    @GetMapping("/internal/storages/storage-info")
    @ResponseBody
    List<StorageInfo> storageInfo();
}
