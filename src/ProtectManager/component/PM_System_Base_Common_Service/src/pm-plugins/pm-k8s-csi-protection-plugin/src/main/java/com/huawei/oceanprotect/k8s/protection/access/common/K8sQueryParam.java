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
package com.huawei.oceanprotect.k8s.protection.access.common;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.List;

/**
 * k8s查询条件
 * </br>
 * kind为k8s定义的资源，例如 Namespace、PersistentVolume
 * kind为Namesapce时不需传下面super的值; 当该值为空时查询集群资源；
 * fieldSelector，labelSelector始终生效
 * fieldSelector和labelSelector存在时，只关注namespace，不关注super其他值
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/7/18
 */
@Getter
@Setter
public class K8sQueryParam {
    /**
     * 要查询的类型
     */
    private String kind;

    /**
     * 要查询的资源在哪个namespace下
     */
    private String namespace;

    /**
     * 上层条件
     * 当在某个资源下面时，需要加该条件
     */
    @JsonProperty("super")
    private List<KindPair> superPairs;

    /**
     * k8s标准field selector
     */
    private String fieldSelector;

    /**
     * k8s标准label selector
     */
    private String labelSelector;

    /**
     * 类型条件pair
     */
    @Setter
    @Getter
    @AllArgsConstructor
    @NoArgsConstructor
    public static class KindPair {
        /**
         * 资源类型，如pods
         */
        private String kind;

        /**
         * 具体值，如pod-0
         */
        private String value;
    }
}
