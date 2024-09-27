/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 */

package openbackup.system.base.bean;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.List;

/**
 * 写入到configmap中的初始化网络信息映射关系类
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023/12/21
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
public class NetWorkConfigInfo {
    private String nodeId;

    @JsonProperty("logic_ip_list")
    private List<NetWorkLogicIp> logicIpList;

    @JsonProperty("ips_route_table")
    private List<NetWorkIpRoutesInfo> ipRouteList;
}
