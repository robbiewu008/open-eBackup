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
package openbackup.system.base.sdk.system;

import openbackup.system.base.sdk.cluster.netplane.NetPlaneInfoReq;

import java.util.List;

/**
 * 逻辑端口服务
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-06-19
 */
public interface PortService {
    /**
     * 检查网络平面配置的网络和相同物理端口已经存在的网络是否属于网段
     *
     * @param netPlaneInfoReq 内部网络请求信息
     * @param portIdList 物理端口id集合
     * @param isInternalNetworkExisted 内部网络是否已经存在
     */
    void checkNetworkSegment(NetPlaneInfoReq netPlaneInfoReq, List<String> portIdList,
                             boolean isInternalNetworkExisted);

    /**
     * 检查内部网络平面配置的网络和相同以太网端口已经存在的网络是否属于同网段
     *
     * @param netPlaneInfoReq 内部网络请求信息
     * @param ethPortIdList 内部网络的以太端口的id集合
     * @param isInternalNetworkExisted 内部网络是否已经存在
     */
    void checkNetworkSegmentDependEth(NetPlaneInfoReq netPlaneInfoReq, List<String> ethPortIdList,
                                      boolean isInternalNetworkExisted);


    /**
     * 检查内部网络平面配置的网络和复用物理端口已经存在的网络是否属于同网段
     *
     * @param netPlaneInfoReq 内部网络请求信息
     * @param reusePortIdList 复用物理端口id集合
     * @param isInternalNetworkExisted 内部网络是否已经存在
     */
    void checkNetworkSegmentWhenReuse(NetPlaneInfoReq netPlaneInfoReq, List<String> reusePortIdList,
                                      boolean isInternalNetworkExisted);
}
