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
package openbackup.system.base.sdk.cluster;

import openbackup.system.base.sdk.resource.model.ResourceLockRequest;
import openbackup.system.base.security.exterattack.ExterAttack;

import feign.Headers;
import feign.Param;
import feign.RequestLine;

import org.hibernate.validator.constraints.Length;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.RequestBody;

import java.net.URI;

/**
 * 备份集群任务客户端
 * <p>
 * 调用其它集群的任务接口
 * </p>
 *
 **/
@Validated
public interface BackupClusterJobClient {
    /**
     * 执行特定集群上的某一个任务
     *
     * @param uri 目标集群的地址
     * @param token 认证token，通过认证接口获取
     * @param jobId 需要执行的任务id
     * @param isNeedSchedule true需要走排队限流，false不需要，立即执行任务
     */
    @ExterAttack
    @RequestLine("PUT /v1/jobs/{jobId}/action/execute?needSchedule={needSchedule}")
    @Headers({"x-auth-token: {token}"})
    void executeJob(URI uri, @Param("token") String token, @Param("jobId") @Length(max = 256) String jobId,
        @Param("needSchedule") Boolean isNeedSchedule);

    /**
     * 通知集群其他节点任务完成
     *
     * @param uri 目标集群的地址
     * @param token 认证token，通过认证接口获取
     */
    @ExterAttack
    @RequestLine("PUT /v1/jobs/action/finished")
    @Headers({"x-auth-token: {token}"})
    void jobComplete(URI uri, @Param("token") String token);

    /**
     * 添加特定集群redis冗余锁
     *
     * @param uri 目标集群的地址
     * @param token 认证token，通过认证接口获取
     * @param request request
     * @return 是否成功
     */
    @ExterAttack
    @RequestLine("PUT /v2/resources/redis/lock")
    @Headers({"x-auth-token: {token}"})
    boolean acquireLock(URI uri, @Param("token") String token, @RequestBody ResourceLockRequest request);

    /**
     * 删除特定集群redis冗余锁
     *
     * @param uri 目标集群的地址
     * @param token 认证token，通过认证接口获取
     * @param request request
     * @return 是否成功
     */
    @ExterAttack
    @RequestLine("DELETE /v2/resources/redis/lock")
    @Headers({"x-auth-token: {token}"})
    boolean unlock(URI uri, @Param("token") String token, @RequestBody ResourceLockRequest request);

    /**
     * 停止指定节点的任务
     *
     * @param uri 目标集群的地址
     * @param token 认证token，通过认证接口获取
     * @param jobId jobId
     */
    @ExterAttack
    @RequestLine("PUT /v1/jobs/{jobId}/action/stop")
    @Headers({"x-auth-token: {token}"})
    void stopTask(URI uri, @Param("token") String token, @Param("jobId") String jobId);
}
