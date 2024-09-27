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
package openbackup.system.base.sdk.auth.model.response;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;
import openbackup.system.base.common.model.PageListResponse;

import java.util.List;

/**
 * 分页模板类
 * 此类已经过时，新增代码不要引用。新增代码统一使用Common模块的分页对象
 * {@link PageListResponse}
 *
 * @param <T> the body type
 * @author w00448845
 * @version [BCManager 8.0.0]
 * @since 2019-11-12
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
@Deprecated
public class UserPageListResponse<T> {
    private int total;

    private Long currentCount;

    private Integer startIndex;

    private Integer pageSize;

    private List<T> userList;
}
