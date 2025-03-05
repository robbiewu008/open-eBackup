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

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.Objects;

/**
 * NetWorkRouteInfo
 *
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class NetWorkRouteInfo {
    private String type;

    private String destination;

    private String mask;

    private String gateway;

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null || getClass() != obj.getClass()) {
            return false;
        }
        NetWorkRouteInfo routeInfo = (NetWorkRouteInfo) obj;
        return Objects.equals(destination, routeInfo.destination) && Objects.equals(gateway, routeInfo.gateway)
            && Objects.equals(mask, routeInfo.mask) && Objects.equals(type, routeInfo.type);
    }

    @Override
    public int hashCode() {
        return Objects.hash(destination, gateway, mask, type);
    }
}
