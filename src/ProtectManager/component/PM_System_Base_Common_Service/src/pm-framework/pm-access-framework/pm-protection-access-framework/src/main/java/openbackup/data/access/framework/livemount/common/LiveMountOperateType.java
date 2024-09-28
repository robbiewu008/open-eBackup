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
package openbackup.data.access.framework.livemount.common;

/**
 * live mount operate type
 *
 **/
public final class LiveMountOperateType {
    /**
     * update
     */
    public static final String UPDATE = "update";

    /**
     * modify
     */
    public static final String MODIFY = "modify";

    /**
     * destory
     */
    public static final String DESTROY = "destroy";

    /**
     * activate
     */
    public static final String ACTIVATE = "activate";

    /**
     * deactivate
     */
    public static final String DEACTIVATE = "deactivate";

    /**
     * migrate
     */
    public static final String MIGRATE = "migrate";
}
