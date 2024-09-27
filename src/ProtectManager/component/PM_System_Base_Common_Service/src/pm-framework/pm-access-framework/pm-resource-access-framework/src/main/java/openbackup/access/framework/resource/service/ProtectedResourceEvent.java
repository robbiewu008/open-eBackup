package openbackup.access.framework.resource.service;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

/**
 * Protected Resource Event
 *
 * @author l00272247
 * @since 2021-10-19
 */
public class ProtectedResourceEvent {
    private String type;
    private ProtectedResource resource;

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public ProtectedResource getResource() {
        return resource;
    }

    public void setResource(ProtectedResource resource) {
        this.resource = resource;
    }
}
