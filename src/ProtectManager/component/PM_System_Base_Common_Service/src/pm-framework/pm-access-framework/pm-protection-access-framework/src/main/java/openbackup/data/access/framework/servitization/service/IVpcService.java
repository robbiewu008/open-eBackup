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
package openbackup.data.access.framework.servitization.service;

import openbackup.data.access.framework.servitization.entity.VpcInfoEntity;

import java.util.List;
import java.util.Set;

/**
 * IVpcService
 *
 */
public interface IVpcService {
    /**
     * 保存vpcInfo
     *
     * @param projectId 项目id
     * @param vpcId vpcId
     * @param markId markId
     * @return id
     */
    String saveVpcInfo(String projectId, String vpcId, String markId);

    /**
     * 查询vpc信息
     *
     * @return vpcInfos
     */
    List<VpcInfoEntity> getVpcInfos();

    /**
     * 删除vpc
     *
     * @param markId markId
     * @return 是否删除成功
     */
    boolean deleteVpcInfo(String markId);

    /**
     * 根据markId查询
     *
     * @param markId markId
     * @return vpc信息
     */
    VpcInfoEntity getProjectIdByMarkId(String markId);

    /**
     * 根据userId查询
     *
     * @param userId userId
     * @return vpc信息列表
     */
    List<VpcInfoEntity> getVpcInfoEntityByProjectId(String userId);

    /**
     * 根据vpcId查询vpc
     *
     * @param vpcIds vpcIds
     * @return vpc信息
     */
    List<VpcInfoEntity> getVpcByVpcIds(Set<String> vpcIds);
}
