package openbackup.data.protection.access.provider.sdk.resource;

import lombok.Data;

/**
 * 资源删除参数
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-19
 */
@Data
public class ResourceDeleteParams {
    /**
     * 强制删除标识。<br/>
     * 值为true时，不进行SLA绑定校验，直接删除；<br/>
     * 值为false时，会进行SLA绑定校验，仅在指定资源及其子资源未绑定SLA的情况下才会被删除。
     */
    private boolean isForce;

    /**
     * shouldDeleteRegister 是否删除手动注册的资源
     */
    private boolean shouldDeleteRegister;

    /**
     * 删除时是否忽略依赖
     */
    private boolean shouldIgnoreDependency = false;

    /**
     * 资源ID列表
     */
    private String[] resources;

    public ResourceDeleteParams() {
        isForce = false;
        shouldDeleteRegister = true;
        shouldIgnoreDependency = false;
    }

    public ResourceDeleteParams(boolean isForce, boolean shouldDeleteRegister, String[] resources) {
        this.isForce = isForce;
        this.shouldDeleteRegister = shouldDeleteRegister;
        this.resources = resources;
    }
}
