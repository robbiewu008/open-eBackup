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
package openbackup.system.base.sdk.auth.model.request;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.sdk.auth.model.Domain;

/**
 * HcsTokenAuth 字段
 * 取值为domain时，表示获取的Token可以访问指定账号下所有资源，domain支持id和name
 *
 */
@Getter
@Setter
public class Scope {
    private Domain domain;

    private Project project;
}
