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
package openbackup.data.access.client.sdk.api.framework.dme.replicate.model;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import openbackup.system.base.sdk.accesspoint.model.DmeLocalDevice;
import openbackup.system.base.sdk.accesspoint.model.DmeRemoteDevice;

import java.util.List;

/**
 * Dme Replicate Request
 *
 */
@Data
public class DmeReplicateRequest {
    private List<DmeLocalDevice> localDevices;

    private List<DmeRemoteDevice> remoteDevice;

    @JsonProperty("resourceID")
    private String resourceId;

    @JsonInclude(JsonInclude.Include.NON_DEFAULT)
    private int qos;

    private long startReplicateTime;

    private String applicationType;

    @JsonProperty("pmmetadata")
    private String metadata;

    @JsonProperty("taskID")
    private String taskId;

    @JsonProperty("job_request_id")
    private String jobRequestId;

    /**
     * 是否开启压缩
     */
    @JsonProperty("enableCompress")
    private boolean isEnableCompress;

    /**
     * 是否开启重删
     */
    @JsonProperty("enableDedupe")
    private boolean isEnableDeduplication;

    /**
     * 是否开启加密
     */
    @JsonProperty("enableIPsec")
    private boolean isEnableEncryption = false;

    /**
     * 相同副本链所有副本id
     */
    private List<String> sameChainCopies;

    private String copyId;

    private int copyFormat;

    private String localEsn;

    /**
     * 是否域内
     */
    @JsonProperty("intra")
    private boolean isIntra = false;

    /**
     * 复制类型:"0"日志复制,"1"数据复制
     */
    private String repType;
}
