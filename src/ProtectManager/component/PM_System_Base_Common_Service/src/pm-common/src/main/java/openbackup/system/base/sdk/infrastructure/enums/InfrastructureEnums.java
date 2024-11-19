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
package openbackup.system.base.sdk.infrastructure.enums;

import lombok.Getter;

/**
 * InfrastructureEnums
 *
 */
public class InfrastructureEnums {
    /**
     * NodeRoleEnums
     *
     */
    @Getter
    public enum NodeRoleEnums {
        /**
         * 主节点
         */
        MASTER0("master0", "primary"),

        /**
         * 备节点
         */
        MASTER("master", "standby"),

        /**
         * worker节点
         */
        WORKER("worker", "member");
        private final String nativeRole;
        private final String convertRole;
        NodeRoleEnums(String nativeRole, String convertRole) {
            this.nativeRole = nativeRole;
            this.convertRole = convertRole;
        }

        /**
         * getNodeRoleEnumsByNativeRole
         *
         * @param nativeRole nativeRole
         * @return NodeRoleEnums
         */
        public static NodeRoleEnums getNodeRoleEnumsByNativeRole(String nativeRole) {
            for (NodeRoleEnums nodeRoleEnums : NodeRoleEnums.values()) {
                if (nodeRoleEnums.getNativeRole().equals(nativeRole)) {
                    return nodeRoleEnums;
                }
            }
            return MASTER0;
        }
    }

    /**
     * NodeStatusEnums
     *
     */
    @Getter
    public enum NodeStatusEnums {
        /**
         * Ready
         */
        READY("Ready"),

        /**
         * Other
         */
        OTHER("Other");
        private final String value;
        NodeStatusEnums(String value) {
            this.value = value;
        }
    }
}
