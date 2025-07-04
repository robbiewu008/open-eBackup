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
package openbackup.data.access.framework.livemount.provider;

import openbackup.data.protection.access.provider.sdk.exception.NotImplementedException;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCancelTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCreateTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountInterceptorProvider;

import org.springframework.stereotype.Component;

/**
 * Void Live Mount Interceptor Provider
 *
 */
@Component
public class VoidLiveMountInterceptorProvider implements LiveMountInterceptorProvider {
    /**
     * init live mount create param
     *
     * @param task task
     */
    @Override
    public void initialize(LiveMountCreateTask task) {
        throw new NotImplementedException("not implemented");
    }

    /**
     * init live mount cancel task param
     *
     * @param task task
     */
    @Override
    public void finalize(LiveMountCancelTask task) {
        throw new NotImplementedException("not implemented");
    }

    /**
     * detect resource type applicable
     *
     * @param resourceType resource type
     * @return detect result
     */
    @Override
    public boolean applicable(String resourceType) {
        return resourceType == null;
    }
}
