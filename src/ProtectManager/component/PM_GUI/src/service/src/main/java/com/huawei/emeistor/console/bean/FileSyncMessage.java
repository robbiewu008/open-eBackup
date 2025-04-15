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
package com.huawei.emeistor.console.bean;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 文件在所有节点间同步的kafka消息
 *
 */
@Getter
@Setter
public class FileSyncMessage {
    /**
     * 请求id
     */
    private String requestId;

    /**
     * 文件信息实体类列表
     */
    private List<FileSyncEntity> fileSyncEntityList;

    /**
     * 0:ADD,1:DELETE
     */
    private Integer action;
}
