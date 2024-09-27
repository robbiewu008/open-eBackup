/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dme.replicate.model;

import openbackup.system.base.sdk.accesspoint.model.DmeLocalDevice;
import openbackup.system.base.sdk.accesspoint.model.DmeRemoteDevice;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * Dme Replicate Request
 *
 * @author l00272247
 * @since 2020-12-16
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
}
