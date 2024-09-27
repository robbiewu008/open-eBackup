/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * @author l00272247
 * @since 2021-12-29
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
