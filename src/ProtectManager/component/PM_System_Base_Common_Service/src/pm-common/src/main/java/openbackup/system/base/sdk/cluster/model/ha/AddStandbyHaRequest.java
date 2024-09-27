/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model.ha;

import static openbackup.system.base.common.constants.IsmNumberConstant.FOUR;
import static openbackup.system.base.common.constants.IsmNumberConstant.THIRTY_TWO;

import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Data;

import java.util.List;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 添加备节点HA请求体
 *
 * @author w00607005
 * @since 2023-05-12
 */
@Data
public class AddStandbyHaRequest {
    /**
     * 对端GaussDB POD对应的内部通信网络ip地址
     */
    @NotNull(message = "The connectionIp cannot be null.")
    @Size(max = IsmNumberConstant.TWO_HUNDRED_FIFTY_SIX, min = IsmNumberConstant.ONE,
        message = "The length of connectionIp is 1-256 characters")
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "connectionIp is invalid")
    private String connectionIp;

    /**
     * 浮动IP地址
     */
    @NotNull(message = "The floatIpAddress cannot be null. ")
    @Size(max = IsmNumberConstant.TWO_HUNDRED_FIFTY_SIX, min = IsmNumberConstant.ONE,
        message = "The length of floatIpAddress is 1-256 characters")
    @Pattern(regexp = RegexpConstants.IP_V4V6_ADDRESS, message = "floatIpAddress is invalid")
    private String floatIpAddress;

    /**
     * 仲裁网关列表
     */
    @NotNull(message = "The gatewayIpList cannot be null. ")
    @Size(max = IsmNumberConstant.THREE, min = IsmNumberConstant.ONE, message = "The size of gatewayIpList is 1-3")
    private List<String> gatewayIpList;

    /**
     * 节点角色，Primary：主节点，Standby：从节点，Member：成员节点
     */
    @NotNull(message = "The role cannot be null. ")
    @Pattern(regexp = "primary|standby")
    private String role;

    /**
     * 本节点名称
     */
    @NotNull(message = "The localName cannot be null. ")
    @Size(max = THIRTY_TWO, min = FOUR, message = "The length of localName is 4-32 characters")
    @Pattern(regexp = RegexpConstants.NAME_STR, message = "localName is invalid")
    private String localName;

    /**
     * 对端节点名称
     */
    @NotNull(message = "The peerName cannot be null. ")
    @Size(max = THIRTY_TWO, min = FOUR, message = "The length of peerName is 4-32 characters")
    @Pattern(regexp = RegexpConstants.NAME_STR, message = "peerName is invalid")
    private String peerName;
}
