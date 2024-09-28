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
package openbackup.data.access.client.sdk.api.framework.dee;

import openbackup.data.access.client.sdk.api.framework.dee.model.CreateFsSnapshotRequest;
import openbackup.data.access.client.sdk.api.framework.dee.model.DeleteFsSnapshotRequest;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;

/**
 * 文件系统快照DEE REST接口
 *
 */
@FeignClient(name = "deeFsSnapshotRestApi",
    url = "${data-enable-engine-server.url}/v1/internal/",
    configuration = CommonFeignConfiguration.class)
public interface DeeFsSnapshotRestApi {
    /**
     * 创建文件系统快照
     *
     * @param createFsSnapshotRequest 创建文件系统快照请求参数
     */
    @ExterAttack
    @PostMapping("anti/ransomware/fssnapshots")
    void createFsSnapshot(@RequestBody CreateFsSnapshotRequest createFsSnapshotRequest);

    /**
     * 删除文件系统快照
     *
     * @param deleteFsSnapshotRequest  删除文件系统快照请求参数
     */
    @ExterAttack
    @DeleteMapping("anti/ransomware/fssnapshots")
    void deleteFsSnapshot(@RequestBody DeleteFsSnapshotRequest deleteFsSnapshotRequest);
}
