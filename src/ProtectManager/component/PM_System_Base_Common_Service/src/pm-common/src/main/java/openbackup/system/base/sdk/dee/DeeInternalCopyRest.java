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
package openbackup.system.base.sdk.dee;

import openbackup.system.base.common.rest.DeeCatalogFeignConfiguration;
import openbackup.system.base.sdk.dee.model.CopyCatalogsRequest;
import openbackup.system.base.sdk.dee.model.RestoreFilesResponse;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;

/**
 * dee 副本相关的内部接口
 *
 */
@FeignClient(name = "deeInternalCopyRest", url = "${protectengine-e-dee.url}/v1/internal",
    configuration = DeeCatalogFeignConfiguration.class)
public interface DeeInternalCopyRest {
    /**
     * 浏览副本文件和目录
     *
     * @param copyCatalogsRequest 副本文件和目录请求体
     * @return 副本文件和目录信息
     */
    @ExterAttack
    @PostMapping("/indexes/folder/action/list")
    RestoreFilesResponse listCopyCatalogs(@RequestBody CopyCatalogsRequest copyCatalogsRequest);
}
