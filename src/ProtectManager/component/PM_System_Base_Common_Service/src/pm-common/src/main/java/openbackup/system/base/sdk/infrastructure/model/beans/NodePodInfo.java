/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.infrastructure.model.beans;

import com.fasterxml.jackson.annotation.JsonIgnore;
import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.LinkedList;
import java.util.List;

/**
 * 网络平面信息
 *
 * @author w00493811
 * @since 2021-01-25
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class NodePodInfo {
    @JsonProperty("nodeName")
    private String nodeName;

    @JsonProperty("namespace")
    private String namespace;

    @JsonProperty("podName")
    private String podName;

    @JsonProperty("podStatus")
    private String podStatus;

    @JsonProperty("netPlaneInfo")
    private List<NetPlaneInfo> netPlaneInfos;

    /**
     * 容器状态，/pod/status接口返回
     */
    @JsonProperty("containerInfo")
    private List<ContainerInfo> containerInfos;

    /**
     * 获取网络平面所有地址信息
     *
     * @return 地址列表
     */
    @JsonIgnore
    public List<String> getAllIps() {
        List<String> allIps = new LinkedList<>();
        for (NetPlaneInfo info : netPlaneInfos) {
            allIps.add(info.getIpAddress());
        }
        return allIps;
    }
}
