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
package openbackup.system.base.common.validator.constants;

/**
 * ValidateGroups
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-04-02
 */
public class ValidateGroups {
    /**
     * 网段路由
     */
    public interface NetworkSegmentRoute {}

    /**
     * 主机路由
     */
    public interface MasterRoute {}

    /**
     * 默认路由
     */
    public interface DefaultRoute {}

    /**
     * IPV4
     */
    public interface IPv4Group {}

    /**
     * IPV6
     */
    public interface IPv6Group {}

    /**
     * IPV6
     */
    public interface VlanHomePort {}
}
