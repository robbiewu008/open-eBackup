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
package openbackup.data.access.client.sdk.api.framework.dmc;

import openbackup.data.access.client.sdk.api.config.achive.DmeArchiveFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.context.annotation.Primary;

/**
 * Dmc Copy Service
 *
 */
@FeignClient(
        name = "dmcCopyService",
        url = "${services.endpoints.protectengine.dme}",
        configuration = DmeArchiveFeignConfiguration.class)
@Primary
public interface DmcCopyService extends DmcCopyServiceApi {
}
