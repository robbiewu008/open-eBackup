/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * @author l00272247
 * @since 2021-10-20
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
