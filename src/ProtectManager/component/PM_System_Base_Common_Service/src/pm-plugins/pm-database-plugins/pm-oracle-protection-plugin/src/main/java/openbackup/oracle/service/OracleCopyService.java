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
package openbackup.oracle.service;

import openbackup.oracle.constants.ScnCopy;

import java.util.List;

/**
 * Oracle副本服务
 *
 * @author c30038333
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-04-04
 */
public interface OracleCopyService {
    /**
     * 根据scn查询副本
     *
     * @param resourceId 资源id
     * @param filterValue 筛选值
     * @return 副本信息
     */
    List<ScnCopy> listCopiesInfo(String resourceId, String filterValue);
}
