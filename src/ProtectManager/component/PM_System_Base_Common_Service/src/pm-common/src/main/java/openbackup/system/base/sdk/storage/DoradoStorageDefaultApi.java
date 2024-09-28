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

import openbackup.system.base.config.feign.dorado.DoradoClusterFeignConfiguration;
import openbackup.system.base.sdk.storage.model.StorageAlarmRecordReq;
import openbackup.system.base.sdk.storage.model.StorageCommonRes;

import feign.Param;
import feign.RequestLine;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * 调用DM接口,不走dma转发
 *
 */
@FeignClient(name = "DoradoStorageDefaultApi", url = "https://${repository.storage.ip}:${repository.storage.port}",
    configuration = DoradoClusterFeignConfiguration.class)
public interface DoradoStorageDefaultApi {
    /**
     * 上报/恢复 告警
     *
     * @param deviceId deviceId
     * @param storageAlarmRecordReq 存储告警上报BO
     * @return 是否成功
     */
    @RequestLine("POST /deviceManager/rest/{deviceId}/record_event_log")
    @ResponseBody
    StorageCommonRes<Object> recordAlarm(@Param("deviceId") String deviceId,
        @RequestBody StorageAlarmRecordReq storageAlarmRecordReq);
}
