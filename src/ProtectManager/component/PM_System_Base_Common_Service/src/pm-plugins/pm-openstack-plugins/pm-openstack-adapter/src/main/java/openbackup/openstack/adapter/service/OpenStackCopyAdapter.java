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
package openbackup.openstack.adapter.service;

import openbackup.openstack.adapter.dto.OpenStackCopyDto;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * OpenStack副本适配器
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-18
 */
@Slf4j
@Component
public class OpenStackCopyAdapter {
    private final OpenStackCopyManager copyManager;

    public OpenStackCopyAdapter(OpenStackCopyManager copyManager) {
        this.copyManager = copyManager;
    }

    /**
     * 查询指定副本信息
     *
     * @param copyId 副本id
     * @return 副本信息
     */
    public OpenStackCopyDto queryCopy(String copyId) {
        return copyManager.queryCopy(copyId);
    }

    /**
     * 查询所有副本
     *
     * @param resourceId 资源id
     * @return 所有副本信息
     */
    public List<OpenStackCopyDto> queryCopies(String resourceId) {
        return copyManager.queryCopies(resourceId);
    }

    /**
     * 删除副本
     *
     * @param copyId 副本id
     */
    public void deleteCopy(String copyId) {
        copyManager.deleteCopy(copyId);
    }
}
