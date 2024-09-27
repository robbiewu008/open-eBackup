/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.access.framework.resource.service;

import openbackup.system.base.invoke.Invoker;

import java.util.Collections;
import java.util.List;

/**
 * Protected Resource Monitor
 *
 * @author l00272247
 * @since 2021-10-19
 */
public interface ProtectedResourceMonitor extends Invoker<ProtectedResourceEvent, Object> {
    /**
     * event type
     *
     * @return event type
     */
    default List<String> getTypes() {
        return Collections.emptyList();
    }
}
