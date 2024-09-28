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
package openbackup.data.access.client.sdk.api.framework.dee.model;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.List;

/**
 * 删除副本索引请求
 *
 */
@AllArgsConstructor
@NoArgsConstructor
@Getter
@Setter
public class DeleteCopyIndexRequest {
    /**
     * 请求id
     */
    private String requestId;

    /**
     * 资源id
     */
    private String resourceId;

    /**
     * 副本id列表
     */
    private List<String> copyIdList;

    /**
     * 副本链id
     */
    private String chainId;

    /**
     * 用户id
     */
    private String userId;
}
