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
package com.huawei.oceanprotect.system.base.initialize.network.util;

import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.ethport.EthPort;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.apache.poi.hssf.usermodel.HSSFCell;
import org.apache.poi.ss.usermodel.CellType;
import org.apache.poi.ss.usermodel.DateUtil;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * 网络路由信息处理公共类
 *
 */
public class NetworkConfigUtils {
    private static final Pattern IPV4_PATTERN = Pattern.compile("(\\d+\\.){3}\\d+");

    private NetworkConfigUtils() {
    }

    /**
     * 获取当前子网ip
     *
     * @param ip 起始ip
     * @param mask 子网掩码
     * @return 子网ip/无
     */
    public static String getNetBaseIpv4(String ip, String mask) {
        Matcher matcher = IPV4_PATTERN.matcher(ip);
        if (matcher.find()) {
            return calcSubnetIpv4(matcher.group(), mask);
        } else {
            return "";
        }
    }

    private static String calcSubnetIpv4(String ip, String mask) {
        String[] ips = ip.split("\\.");
        List<Integer> allIps = new LinkedList<>();
        Arrays.stream(ips).forEach(value -> allIps.add(Integer.parseInt(value.trim())));

        List<Integer> maskList = new LinkedList<>();
        ips = mask.split("\\.");
        Arrays.stream(ips).forEach(value -> maskList.add(Integer.parseInt(value.trim())));

        List<Integer> subnetList = new LinkedList<>();
        for (int index = 0; index < Math.max(allIps.size(), maskList.size()); index++) {
            subnetList.add(allIps.get(index) & maskList.get(index));
        }

        StringBuffer sb = new StringBuffer();
        subnetList.forEach(value -> {
            if (sb.length() > 0) {
                sb.append(".").append(value);
            } else {
                sb.append(value);
            }
        });
        return sb.toString();
    }

    /**
     * 获取过滤后的端口集合
     *
     * @param ethPortOwnIngLocation 控制器的前端卡 LocationName;
     * @param collect 过滤前的端口集合
     * @param controllerLocation 控制器的location
     * @return 过滤后的端口集合
     */
    public static List<EthPort> getEthPort(String ethPortOwnIngLocation, List<EthPort> collect,
        String controllerLocation) {
        List<EthPort> ethPortList = new ArrayList<>();
        if (collect.size() == 0) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "no find ethPort");
        }

        for (EthPort ethPort : collect) {
            if (!ethPort.getLocation().contains(ethPortOwnIngLocation)) {
                continue;
            }

            if (ethPort.getLocation().contains(InitConfigConstant.P0) || ethPort.getLocation()
                .contains(InitConfigConstant.P1)) {
                ethPortList.add(ethPort);
            }
        }
        return ethPortList;
    }


    /**
     * 获取查询信息返回对应具体信息
     *
     * @param cell 查询信息cell
     * @return 具体信息
     */
    public static Object getCellValue(HSSFCell cell) {
        Object cellValue = null;
        CellType cellType = cell.getCellType();
        if (cellType == CellType.STRING) {
            cellValue = cell.getStringCellValue();
        } else if (cellType == CellType.NUMERIC) {
            if (DateUtil.isCellDateFormatted(cell)) {
                cellValue = cell.getDateCellValue();
            } else {
                cellValue = cell.getNumericCellValue();
            }
        } else if (cellType == CellType.BOOLEAN) {
            cellValue = cell.getBooleanCellValue();
        } else if (cellType == CellType.FORMULA) {
            cellValue = cell.getCellFormula();
        } else {
            cellValue = "";
        }
        return cellValue;
    }
}
