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
package openbackup.data.access.client.sdk.api.framework.dme.replicate;

import openbackup.data.access.client.sdk.api.config.achive.DmeArchiveFeignConfiguration;
import openbackup.data.access.client.sdk.api.config.achive.DmeResponse;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.model.DmeReplicateRequest;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.model.IpTablesActionRequest;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.model.ReceiveJobActionInfo;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.model.ReceiveJobActionRequest;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.model.RemoveCopyRequest;
import openbackup.system.base.sdk.accesspoint.model.CleanRemoteRequest;
import openbackup.system.base.sdk.accesspoint.model.DmeLogicIpsRequest;
import openbackup.system.base.sdk.cluster.model.DmeRemovePairRequest;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.ResponseBody;

import java.net.URI;

/**
 * Dme Replication Rest Api
 *
 */
@FeignClient(name = "dmeReplicationRestApi", url = "${services.endpoints.protectengine.replication}/v1/dme_replication",
    configuration = DmeArchiveFeignConfiguration.class)
public interface DmeReplicationRestApi {
    /**
     * remove pair
     *
     * @param request request
     * @return void
     */
    @PostMapping("/removepair")
    Void removePair(@RequestBody DmeRemovePairRequest request);

    /**
     * clean remote
     *
     * @param request request
     * @return void
     */
    @PostMapping("/cleanremote")
    Void cleanRemote(@RequestBody CleanRemoteRequest request);

    /**
     * replicate
     *
     * @param request request
     * @return void
     */
    @PostMapping("/replicate")
    DmeResponse<String> replicate(@RequestBody DmeReplicateRequest request);

    /**
     * replicate
     *
     * @param uri uri
     * @param request request
     * @return void
     */
    @PostMapping("/v1/dme_replication/replicate")
    DmeResponse<String> replicate(URI uri, @RequestBody DmeReplicateRequest request);

    /**
     * 修改逻辑端口IP
     *
     * @param request logic request
     * @return response
     */
    @PostMapping("/updateLogicIps")
    @ResponseBody
    DmeResponse<String> replicationLogicIps(@RequestBody DmeLogicIpsRequest request);

    /**
     * 删除副本通知
     *
     * @param removeCopyRequest removeCopyRequest
     * @return void
     */
    @PostMapping("/deletecopy")
    Void deleteCopy(@RequestBody RemoveCopyRequest removeCopyRequest);

    /**
     * ip表操作
     *
     * @param ipTablesActionRequest ipTablesActionRequest
     * @return void
     */
    @PostMapping("/iptablesaction")
    Void ipTablesAction(@RequestBody IpTablesActionRequest ipTablesActionRequest);

    /**
     * 接收复制任务操作
     *
     * @param receiveJobActionRequest receiveJobActionRequest
     * @return void
     */
    @PutMapping("/receivejob")
    Void receiveJobAction(@RequestBody ReceiveJobActionRequest receiveJobActionRequest);

    /**
     * 查询执行中的复制任务
     *
     * @return 复制任务返回对象
     */
    @GetMapping("/queryjob")
    DmeResponse<ReceiveJobActionInfo> queryJob();
}
