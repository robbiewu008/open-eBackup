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
package openbackup.access.framework.resource.service;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.invoke.Invocation;
import openbackup.system.base.invoke.Invoker;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Protected Resource Monitor Service
 *
 */
@Component
public class ProtectedResourceMonitorService {
    private final List<ProtectedResourceMonitor> protectedResourceMonitors;

    /**
     * constructor
     *
     * @param protectedResourceWatchService protectedResourceWatchService
     * @param protectedResourceMonitors protectedResourceMonitors
     */
    public ProtectedResourceMonitorService(
            ProtectedResourceWatchService protectedResourceWatchService,
            List<ProtectedResourceMonitor> protectedResourceMonitors) {
        this.protectedResourceMonitors =
                Stream.concat(Stream.of(protectedResourceWatchService), protectedResourceMonitors.stream())
                        .distinct()
                        .collect(Collectors.toList());
        assert this.protectedResourceMonitors.containsAll(protectedResourceMonitors);
        assert this.protectedResourceMonitors.size() == protectedResourceMonitors.size();
    }

    /**
     * invoke method
     *
     * @param type type
     * @param resource resource
     * @param function function
     * @return result
     */
    public Object invoke(String type, ProtectedResource resource, Function<ProtectedResourceEvent, Object> function) {
        ProtectedResourceEvent event = new ProtectedResourceEvent();
        event.setType(type);
        event.setResource(resource);
        List<Invoker<ProtectedResourceEvent, Object>> monitors =
                protectedResourceMonitors.stream()
                        .filter(protectedResourceMonitor -> protectedResourceMonitor.getTypes().contains(type))
                        .collect(Collectors.toList());
        return new Invocation<>(function, monitors).invoke(event);
    }

    /**
     * invoke method
     *
     * @param type type
     * @param resource resource
     */
    public void invoke(String type, ProtectedResource resource) {
        invoke(type, resource, ProtectedResourceEvent::getResource);
    }
}
