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

import openbackup.system.base.sdk.cluster.model.TargetClusterRequest;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RequestPart;
import org.springframework.web.multipart.MultipartFile;

import java.net.URI;

/**
 * 给所有节点发消息
 *
 */
public interface NodeRestApi {
    /**
     * 获取浮动ip
     *
     * @param uri uri
     * @return 浮动ip
     */
    @ExterAttack
    @GetMapping("/v1/internal/service/float-ip")
    String getFloatIp(URI uri);

    /**
     * 获取管理ip
     *
     * @param uri uri
     * @return 管理ip
     */
    @ExterAttack
    @GetMapping("/v1/internal/service/manager-ip")
    String getManagerIp(URI uri);

    /**
     * 同步事件转储文件
     *
     * @param uri uri
     * @param multipartFile multipartFile
     * @param filePath filePath
     */
    @ExterAttack
    @PostMapping(value = "/v1/internal/alarms/dumpfile", consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    void syncAlarmDumpFile(URI uri, @RequestPart("dumpFile") MultipartFile multipartFile,
        @RequestParam("filePath") String filePath);

    /**
     * createTargetCluster
     *
     * @param uri uri
     * @param request request
     * @return id
     */
    @ExterAttack
    @PostMapping(value = "/v1/internal/clusters/target-cluster")
    int createTargetCluster(URI uri, @RequestBody TargetClusterRequest request);

    /**
     * createTargetCluster
     *
     * @param uri uri
     * @param infraIp infraIp
     * @return id
     */
    @ExterAttack
    @GetMapping(value = "/v1/internal/clusters/primary-infra-ip-connection")
    boolean checkConnectionToPrimaryInfraIp(URI uri, @RequestParam String infraIp);

    /**
     * 获取可用的dme ip
     *
     * @param uri uri
     * @param agentUrl agent的ip和post
     * @return dme ip
     */
    @ExterAttack
    @GetMapping("/v1/internal/host-agent/connected/dme/ips")
    String getConnectedDmeIps(URI uri, @RequestParam("agentUrl") String agentUrl);

    /**
     * 同步文件到集群中的所有节点
     *
     * @param uri uri
     * @param multipartFile multipartFile
     * @param filePath filePath
     */
    @ExterAttack
    @PostMapping(value = "/v1/internal/service/syncfile", consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    void syncFile(URI uri, @RequestPart("file") MultipartFile multipartFile,
        @RequestParam("filePath") String filePath);

    /**
     * 删除集群中所有节点的文件
     *
     * @param uri uri
     * @param filePath filePath
     */
    @ExterAttack
    @DeleteMapping(value = "/v1/internal/service/deletefile")
    void deleteFile(URI uri, @RequestParam("filePath") String filePath);
}
