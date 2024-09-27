package openbackup.access.framework.resource.vo;

import lombok.Getter;
import lombok.Setter;

/**
 * ProtectedResourceLoggingVo resource 事件vo对象，主要用于组装后返回给logging框架记录detail
 *
 * @author x00635457
 * @since 2023/6/8
 */
@Getter
@Setter
public class ProtectedResourceLoggingVo {
    /**
     * 默认信息内容 --
     */
    public static final String DEFAULT_VAL = "--";

    private String storageName = DEFAULT_VAL;

    private String storageUUID = DEFAULT_VAL;

    private String storageType = DEFAULT_VAL;

    private String tenantName = DEFAULT_VAL;

    private String tenantId = DEFAULT_VAL;

    private String resourceName = DEFAULT_VAL;

    private String resourceId = DEFAULT_VAL;
}
