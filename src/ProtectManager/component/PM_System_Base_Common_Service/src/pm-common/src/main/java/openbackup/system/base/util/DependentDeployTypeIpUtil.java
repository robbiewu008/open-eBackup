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
package openbackup.system.base.util;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.bean.NetWorkConfigInfo;
import openbackup.system.base.bean.NetWorkLogicIp;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.constants.SymbolConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.process.ProcessException;
import openbackup.system.base.common.process.ProcessResult;
import openbackup.system.base.common.process.ProcessUtil;
import openbackup.system.base.common.utils.ExceptionUtil;

import org.apache.commons.lang3.StringUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 */
@Slf4j
public class DependentDeployTypeIpUtil {
    // 获取ip信息命令
    private static final List<String> IP_ADDR_COMMAND = Arrays.asList("/bin/sh", "-c", "ip -br addr show | grep UP");

    private DependentDeployTypeIpUtil() {
    }

    /**
     * 将网络配置list转化为key为ifname value为ip的map
     *
     * @param ipList iplist
     * @return 网络配置map
     */
    public static Map<String, String> convertIpStringToMap(List<String> ipList) {
        Map<String, String> ipInfoMap = new HashMap<>();
        ipList.forEach(line -> {
            List<String> splitResult = Arrays.stream(line.split(" "))
                    .filter(split -> !" ".equals(split) && !split.isEmpty()).collect(Collectors.toList());
            if (splitResult.size() > IsmNumberConstant.TWO) {
                ipInfoMap.put(splitResult.get(0), splitResult.get(IsmNumberConstant.TWO));
            }
        });
        return ipInfoMap;
    }

    /**
     * 获取宿主机IP配置(ifname UP ip/mask)
     *
     * @return ipList
     */
    public static List<String> getIpInfoList() {
        try {
            ProcessResult processResult = ProcessUtil.executeInMinutes(IP_ADDR_COMMAND, 1);
            if (processResult.isNok()) {
                log.error("Get ip addresses command failed.");
                return Collections.emptyList();
            }
            log.info("Get ip addresses success: {}", processResult.getOutputList().toString());
            return processResult.getOutputList();
        } catch (ProcessException e) {
            log.error("Get ip addresses command failed.", ExceptionUtil.getErrorMessage(e));
            return Collections.emptyList();
        }
    }

    /**
     * 从ipAddress(ip/mask)中获取ip
     *
     * @param ipAddress ipAddress
     * @return ip
     */
    public static String getIpFromIpAddress(String ipAddress) {
        String[] split = ipAddress.split(SymbolConstant.OBLIQUE_LINE);
        if (split.length < LegoNumberConstant.TWO) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Get ip from ipAddress failed.");
        }
        return split[LegoNumberConstant.ZERO];
    }

    /**
     * 从ipAddress(ip/mask)中获取NetWorkLogicIp (192.168.111.222/16)
     *
     * @param ipAddress ipAddress
     * @return NetWorkLogicIp (192.168.111.222/255.255.0.0)
     */
    public static NetWorkLogicIp getNetWorkLogicIp(String ipAddress) {
        String[] split = ipAddress.split(SymbolConstant.OBLIQUE_LINE);
        if (split.length < LegoNumberConstant.TWO) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Get ip from ipAddress failed.");
        }
        NetWorkLogicIp logicIp = new NetWorkLogicIp();
        logicIp.setIp(split[0]);
        int length = Integer.parseInt(split[1]);
        if (length > 32) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Get ip from ipAddress failed.");
        }
        String maskString = StringUtils.repeat('1', length) + StringUtils.repeat('0', 32 - length);
        int maskIp1 = Integer.parseInt(maskString.substring(0, 8), 2);
        int maskIp2 = Integer.parseInt(maskString.substring(8, 16), 2);
        int maskIp3 = Integer.parseInt(maskString.substring(16, 24), 2);
        int maskIp4 = Integer.parseInt(maskString.substring(24, 32), 2);
        String mask = maskIp1 + "." + maskIp2 + "." + maskIp3 + "." + maskIp4;
        logicIp.setMask(mask);
        return logicIp;
    }

    /**
     * 生成存入configmap的参数形式
     *
     * @param nodeId 节点名称
     * @param ipInfoList ip信息
     * @return 参数
     */
    public static NetWorkConfigInfo generateConfigMapParam(String nodeId, List<String> ipInfoList) {
        // 获取存储到network-configmap的参数
        NetWorkConfigInfo netWorkConfigInfo = new NetWorkConfigInfo();
        // 节点名称
        netWorkConfigInfo.setNodeId(nodeId);
        // 软硬解耦没有RouteInfoList
        netWorkConfigInfo.setIpRouteList(new ArrayList<>());
        // 用户所选业务网络IP信息(ip/mask)
        List<NetWorkLogicIp> ipPortList = new ArrayList<>();
        ipInfoList.forEach(ipInfo -> {
            ipPortList.add(DependentDeployTypeIpUtil.getNetWorkLogicIp(ipInfo));
        });
        netWorkConfigInfo.setLogicIpList(ipPortList);
        return netWorkConfigInfo;
    }
}
