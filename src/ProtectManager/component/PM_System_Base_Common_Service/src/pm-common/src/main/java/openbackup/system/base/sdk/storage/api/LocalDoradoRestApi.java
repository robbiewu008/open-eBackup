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
package openbackup.system.base.sdk.storage.api;

import openbackup.system.base.config.feign.dorado.DoradoClusterFeignConfiguration;
import openbackup.system.base.sdk.storage.model.DoradoResponse;
import openbackup.system.base.sdk.storage.model.FileSystemScrubRequest;
import openbackup.system.base.sdk.storage.model.FileSystemScrubResponse;

import feign.Param;
import feign.RequestLine;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * 功能描述: 访问本地 dorado 存储统一 RestApi
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-26
 */
@FeignClient(name = "DoradoFileSystemRestApi", url = "https://${repository.storage.ip}:${repository.storage.port}",
        configuration = DoradoClusterFeignConfiguration.class)
public interface LocalDoradoRestApi {
    /**
     * 修改文件系统的扫描状态
     *
     * @param deviceId deviceId
     * @param request request
     * @return DoradoResponse
     */
    @RequestLine("POST /deviceManager/rest/{deviceId}/change_file_system_scrub")
    @ResponseBody
    DoradoResponse<Object> changeFileSystemScrub(
            @Param("deviceId") String deviceId, @RequestBody FileSystemScrubRequest request);

    /**
     * 查询文件系统扫描状态
     *
     * @param deviceId deviceId
     * @param fsId fsId
     * @return DoradoResponse
     */
    @RequestLine("GET /deviceManager/rest/{deviceId}/show_fs_system_scrub?file_system_id={fsId}")
    @ResponseBody
    DoradoResponse<FileSystemScrubResponse> queryFileSystemScrub(
            @Param("deviceId") String deviceId, @Param("fsId") String fsId);
}