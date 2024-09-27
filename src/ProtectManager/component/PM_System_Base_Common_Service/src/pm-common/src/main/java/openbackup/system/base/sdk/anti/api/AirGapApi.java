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
package openbackup.system.base.sdk.anti.api;

import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.anti.model.AirGapDeviceInfoRsp;
import openbackup.system.base.sdk.anti.model.AirGapDeviceShowPageReq;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;

/**
 * 功能描述 airGap
 *
 * @author y30037959
 * @since 2023-03-30
 */
@FeignClient(name = "airGapApiSdk", url = "${pm-system-base.url}/v1", configuration = CommonFeignConfiguration.class)
public interface AirGapApi {
    /**
     * 获取AirGap设备列表
     *
     * @param airGapDeviceShowPageRequest airGapDeviceShowPageRequest
     * @return PageListResponse<AirGapDeviceInfo>
     */
    @PostMapping("/internal/anti-ransomware/airgap/devices/page")
    PageListResponse<AirGapDeviceInfoRsp>
        showAirGapDevicePage(@RequestBody AirGapDeviceShowPageReq airGapDeviceShowPageRequest);
}
