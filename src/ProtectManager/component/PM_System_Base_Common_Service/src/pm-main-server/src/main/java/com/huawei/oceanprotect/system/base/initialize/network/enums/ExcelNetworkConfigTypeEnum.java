/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.enums;

import com.huawei.oceanprotect.system.base.initialize.network.common.ExcelNetworkConfigBean;
import com.huawei.oceanprotect.system.base.initialize.network.util.NetworkConfigUtils;

import lombok.Getter;

import org.apache.commons.lang3.StringUtils;
import org.apache.poi.hssf.usermodel.HSSFRow;

/**
 * 配置LD参数的枚举类
 *
 * @author swx1010572
 * @version: [DataBackup 1.5.0]
 * @since 2023-07-25
 */
@Getter
public enum ExcelNetworkConfigTypeEnum {
    /**
     * 业务端口类型
     */
    SERVICEPPORTTYPE(4, "servicePortType"),

    /**
     * 控制器
     */
    CONTRONLLER(5, "controller"),

    /**
     * 以太网端口
     */
    ETHERNETPORT(6, "ethernetPort"),

    /**
     * 端口类型
     */
    PORTTYPE(7, "portType"),

    /**
     * 创建绑定端口名称
     */
    BONDPORTNAME(8, "bondPortName"),


    /**
     * vlan ID
     */
    VLANID(9, "vlanID"),

    /**
     * 网络信息
     */
    NETWORKINFO(10, "networkInfo"),

    /**
     * 路由
     */
    ROUTE(11, "route");

    private final Integer page;

    private final String value;

    ExcelNetworkConfigTypeEnum(Integer page, String value) {
        this.page = page;
        this.value = value;
    }

    /**
     * 更具查询信息行数返回对应具体信息
     *
     * @param row 行列表对象
     * @return 对应枚举类
     */
    public static ExcelNetworkConfigBean updateToNetworkConfigBean(HSSFRow row) {
        ExcelNetworkConfigBean excelNetworkConfigBean = new ExcelNetworkConfigBean();
        for (ExcelNetworkConfigTypeEnum each : ExcelNetworkConfigTypeEnum.values()) {
            String cellValue = row.getCell(each.getPage()) == null
                ? StringUtils.EMPTY
                : NetworkConfigUtils.getCellValue(row.getCell(each.getPage())).toString();
            excelNetworkConfigBean.setValues(cellValue, each.getValue());
        }
        return excelNetworkConfigBean;
    }
}
