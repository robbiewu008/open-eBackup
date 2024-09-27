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
package openbackup.system.base.sdk.cluster.netplane.validateprovider;

import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.validator.constants.ValidateGroups;
import openbackup.system.base.sdk.cluster.netplane.PortRoute;

import sun.net.util.IPAddressUtil;

import org.hibernate.validator.spi.group.DefaultGroupSequenceProvider;

import java.util.ArrayList;
import java.util.List;

/**
 * NetPlaneInfoReqGroupProvider
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-04-02
 */
public class PortRouteGroupProvider implements DefaultGroupSequenceProvider<PortRoute> {
    private static final String MASTER = "1";

    private static final String DEFAULT = "2";

    @Override
    public List<Class<?>> getValidationGroups(PortRoute portRoute) {
        List<Class<?>> defaultGroupSequence = new ArrayList<>();
        defaultGroupSequence.add(PortRoute.class);

        if (!VerifyUtil.isEmpty(portRoute)) {
            if (DEFAULT.equals(portRoute.getRouteType())) {
                defaultGroupSequence.add(ValidateGroups.DefaultRoute.class);
            } else if (MASTER.equals(portRoute.getRouteType())) {
                defaultGroupSequence.add(ValidateGroups.MasterRoute.class);
            } else {
                defaultGroupSequence.add(ValidateGroups.NetworkSegmentRoute.class);
            }

            if (!VerifyUtil.isEmpty(portRoute.getGateway()) && IPAddressUtil.isIPv4LiteralAddress(
                portRoute.getGateway())) {
                defaultGroupSequence.add(ValidateGroups.IPv4Group.class);
            } else {
                defaultGroupSequence.add(ValidateGroups.IPv6Group.class);
            }
        }
        return defaultGroupSequence;
    }
}
