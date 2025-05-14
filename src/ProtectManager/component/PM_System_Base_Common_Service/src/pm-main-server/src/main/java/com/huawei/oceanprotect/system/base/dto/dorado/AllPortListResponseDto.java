/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.dto.dorado;

import com.huawei.oceanprotect.system.base.model.VlanPo;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.logicport.LogicPortAddRequest;

import lombok.Data;

import java.util.List;

/**
 * 发送给外部所有端口列表集合
 *
 * @author swx1010572
 * @since 2022-12-13
 */
@Data
public class AllPortListResponseDto {
    /**
     * 绑定端口列表
     */
    List<BondPortDto> bondPortList;

    /**
     * 以太网端口
     */
    List<EthPortDto> ethPortDtoList;

    /**
     * PM逻辑端口列表
     */
    List<LogicPortDto> logicPortDtoList;

    /**
     * 底座逻辑端口列表
     */
    List<LogicPortAddRequest> dmLogicPortList;

    /**
     * vlan列表
     */
    List<VlanPo> vlanList;

    /**
     * 复用逻辑端口名称列表
     */
    List<String> reuseLogicPortNameList;

    /**
     * 是否有不生效的端口
     */
    boolean isAllLogicPortsValid;
}
