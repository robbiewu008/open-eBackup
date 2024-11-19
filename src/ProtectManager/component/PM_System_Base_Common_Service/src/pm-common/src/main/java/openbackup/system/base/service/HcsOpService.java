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
package openbackup.system.base.service;

import openbackup.system.base.sdk.auth.model.HcsTokenParam;
import openbackup.system.base.sdk.auth.model.TokenParam;

/**
 * op服务化，跨云复制服务类
 *
 */
public interface HcsOpService {
    /**
     * 获取hcs的token
     *
     * @param hcsTokenParam 查询参数
     * @return token
     */
    String getHcsToken(HcsTokenParam hcsTokenParam);

    /**
     * 添加域名映射
     *
     * @param ip 待映射的ip
     * @param host 待映射的host
     * @return 添加域名映射是否成功
     */
    boolean addHostMapping(String ip, String host);

    /**
     * 刪除域名映射
     *
     * @param ip 待刪除的ip
     * @return 刪除域名映射是否成功
     */
    boolean deleteHostMapping(String ip);

    /**
     * 获取hcs运营面的token，跨云复制使用
     *
     * @param param 获取token参数
     * @return token信息
     */
    String getHcsMoToken(TokenParam param);

    /**
     * 添加iam和sc的域名映射信息到/etc/hosts中
     *
     * @param ip IP地址
     * @param domain 域名
     */
    void addIamAndScMapping(String ip, String domain);

    /**
     * 检查hcs集群的token
     *
     * @param clusterId 集群id
     * @param moToken token
     * @param errorCode 错误码
     */
    void checkHcsToken(int clusterId, String moToken, long errorCode);
}
