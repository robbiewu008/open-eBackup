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

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.dee.model.DownloadFilesRequest;
import openbackup.system.base.sdk.dee.model.VmBrowserMountRequest;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;

/**
 * dee BaseParse
 *
 */
@FeignClient(name = "deeBaseParseRest", url = "${protectengine-e-dee-base-parser.url}/v1/internal",
    configuration = CommonFeignConfiguration.class)
public interface DeeBaseParseRest {
    /**
     * 下载副本中的文件
     *
     * @param downloadFilesRequest 下载副本中的文件请求体
     */
    @PostMapping("/flr/action/export")
    void downloadFiles(@RequestBody DownloadFilesRequest downloadFilesRequest);

    /**
     * 开启副本guest system浏览
     *
     * @param copyId 副本id
     * @param vmBrowserMountRequest 请求体
     */
    @ExterAttack
    @PostMapping("/browse/guest-system/open/{copyId}")
    void openCopyGuestSystem(
        @PathVariable("copyId") String copyId, @Validated @RequestBody VmBrowserMountRequest vmBrowserMountRequest);

    /**
     * 关闭副本guest system浏览
     *
     * @param copyId 副本id
     */
    @ExterAttack
    @PutMapping("/browse/guest-system/close/{copyId}")
    void closeCopyGuestSystem(@PathVariable @RequestParam("copyId") String copyId);
}
