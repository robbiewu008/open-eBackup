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
package openbackup.system.base.sdk.dee.model;

import lombok.Data;

import java.util.List;

/**
 * 副本相关的信息
 *
 */
@Data
public class CopyCatalogsRequest {
    private FinegrainedRestoreCopy copyInfo;

    private String parentPath;

    private String name;

    private Integer pageSize;

    private Integer pageNum;

    private String conditions;

    private List<Object> searchAfter;
}

