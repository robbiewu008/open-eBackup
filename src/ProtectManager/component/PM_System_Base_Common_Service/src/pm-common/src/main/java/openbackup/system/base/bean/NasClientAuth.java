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
package openbackup.system.base.bean;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * Nas共享客户端下AUTH
 *
 * @author fwx1022842
 * @version [BCManager 8.0.0]
 * @since 2021-06-11
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class NasClientAuth {
    /**
     * ID
     */
    @JsonProperty("ID")
    private String id;

    /**
     * 权限（0：只读，1：读写）1
     */
    @JsonProperty("ACCESSVAL")
    private String accessVal;

    /**
     * 权限限制（0：all_squash，1：no_all_squash ）1
     */
    @JsonProperty("ALLSQUASH")
    private String allSquash;

    /**
     * 客户端IP或主机名或网络组名
     * 参数长度：1~25600
     */
    @JsonProperty("NAME")
    private String name;

    /**
     * root权限限制(0：root_squash，1：no_root_squash)1
     */
    @JsonProperty("ROOTSQUASH")
    private String rootSquash;

    /**
     * 写入模式（0：同步，1：异步）0
     */
    @JsonProperty("SYNC")
    private String sync;

    /**
     * 父对象ID
     */
    @JsonProperty("PARENTID")
    private String parentId;
}
