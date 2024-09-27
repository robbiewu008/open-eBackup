package openbackup.system.base.sdk.common.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 是否允许恢复对象
 *
 * @author s30031954
 * @since 2024-04-10
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class AllowRestoreObject {
    /**
     * 是否允许恢复
     */
    private String uuid;

    /**
     * 当前资源是否支持恢复，true: 支持恢复， false:不支持恢复
     */
    private String isAllowRestore;
}
