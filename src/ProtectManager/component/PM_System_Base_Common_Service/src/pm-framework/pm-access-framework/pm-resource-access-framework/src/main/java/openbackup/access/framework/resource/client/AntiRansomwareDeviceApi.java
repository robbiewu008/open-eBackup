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
package openbackup.access.framework.resource.client;

import openbackup.access.framework.resource.client.model.DeleteDeviceRequest;
import openbackup.access.framework.resource.client.model.DeviceUpdateStatusResponse;
import openbackup.access.framework.resource.client.model.UpdateDeviceRequest;
import openbackup.access.framework.resource.client.model.UpdateFileSystemRequest;
import openbackup.access.framework.resource.client.model.UpdateLunInfoReq;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * 功能描述 新增或删除设备信息
 *
 */
@FeignClient(name = "updateDeviceApi", url = "${data-enable-engine-server.url}/v1",
    configuration = CommonFeignConfiguration.class)
public interface AntiRansomwareDeviceApi {
    /**
     * url
     */
    String URL_UPDATE_DEVICE = "/internal/anti/ransomware/device";

    /**
     * 更新文件系统url
     */
    String URL_UPDATE_FILE_SYSTEM = "/internal/anti/ransomware/license/filesystem";

    /**
     * url
     */
    String URL_UPDATE_DEVICE_STATUS = "/internal/anti/ransomware/device/status/{device_id}";

    /**
     * 下发san资源接入信息
     */
    String URL_UPDATE_LUN_INFO = "/internal/anti/ransomware/lun/update";

    /**
     * 更新设备
     *
     * @param updateDeviceRequest 更新设备信息请求
     */
    @ExterAttack
    @PostMapping(URL_UPDATE_DEVICE)
    void updateDevice(@RequestBody UpdateDeviceRequest updateDeviceRequest);

    /**
     * 删除设备
     *
     * @param deleteDeviceRequest 删除设备信息请求
     */
    @ExterAttack
    @DeleteMapping(URL_UPDATE_DEVICE)
    void deleteDevice(@RequestBody DeleteDeviceRequest deleteDeviceRequest);

    /**
     * 更新文件系统
     *
     * @param updateFileSystemRequest 更新文件系统请求
     */
    @ExterAttack
    @PutMapping(URL_UPDATE_FILE_SYSTEM)
    void updateFileSystem(@RequestBody UpdateFileSystemRequest updateFileSystemRequest);

    /**
     * 获取设备更新状态
     *
     * @param deviceId 设备ID
     * @return 设备更新状态
     */
    @GetMapping(URL_UPDATE_DEVICE_STATUS)
    @ResponseBody
    @ExterAttack
    DeviceUpdateStatusResponse getDeviceUpdateStatus(@PathVariable("device_id") String deviceId);

    /**
     * 更新LUN信息
     *
     * @param updateLunInfoReq 更新LUN信息
     */
    @ExterAttack
    @PutMapping(URL_UPDATE_LUN_INFO)
    void updateLunInfo(@RequestBody UpdateLunInfoReq updateLunInfoReq);
}
