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

import openbackup.data.access.client.sdk.api.framework.dee.model.OcLiveMountCloneReq;
import openbackup.data.access.client.sdk.api.framework.dee.model.OcLiveMountFsShareReq;
import openbackup.data.access.client.sdk.api.framework.dee.model.OcLiveMountTaskReq;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;

/**
 * 共享路径恢复调用DEE内部接口类
 *
 */
@FeignClient(name = "DeeLiveMountRestApi",
    url = "${data-enable-engine-server.url}/v1/internal/", configuration = CommonFeignConfiguration.class)
public interface DeeLiveMountRestApi {
    /**
     * 克隆文件系统
     *
     * @param cloneFileSystemRequest 克隆文件系统请求参数
     */
    @ExterAttack
    @PostMapping("anti/ransomware/live-mount/clone")
    void createFilesystemClone(@RequestBody OcLiveMountCloneReq cloneFileSystemRequest);

    /**
     * 创建文件系统共享
     *
     * @param liveMountFsShareReq 创建文件系统共享请求参数
     */
    @PostMapping("anti/ransomware/live-mount/shared")
    void createFilesystemShare(@RequestBody OcLiveMountFsShareReq liveMountFsShareReq);

    /**
     * 创建任务
     *
     * @param liveMountTaskReq 创建即使挂载任务请求参数
     */
    @PostMapping("anti/ransomware/live-mount/task")
    void createLiveMountTask(@RequestBody OcLiveMountTaskReq liveMountTaskReq);

    /**
     * 更新任务状态
     *
     * @param liveMountTaskReq 更新任务状态
     */
    @PutMapping("anti/ransomware/live-mount/task")
    void updateLiveMountTask(@RequestBody OcLiveMountTaskReq liveMountTaskReq);
}
