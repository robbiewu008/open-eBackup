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
package openbackup.system.base.sdk.livemount;

import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;

/**
 * Live Mount Ownership Verify Client Rest Api
 *
 * @author l00272247
 * @since 2020-11-28
 */
@FeignClient(name = "live-mount-ownership-verify-client-rest-api",
    url = "${service.url.pm-live-mount}/v1/internal",
    configuration = CommonFeignConfiguration.class)
public interface LiveMountOwnershipVerifyClientRestApi extends LiveMountOwnershipVerifyRestApi {
}
