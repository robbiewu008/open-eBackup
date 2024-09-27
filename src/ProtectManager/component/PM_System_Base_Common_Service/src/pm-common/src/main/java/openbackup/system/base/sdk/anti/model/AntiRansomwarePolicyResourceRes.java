package openbackup.system.base.sdk.anti.model;

import lombok.Data;

/**
 * 防勒索资源对象
 *
 * @author nwx1077006
 * @since 2021-11-12
 */
@Data
public class AntiRansomwarePolicyResourceRes {
    // 资源id
    private String resourceId;

    // 资源名称
    private String resourceName;

    // 资源路径
    private String resourceLocation;

    // 资源子类型
    private String resourceSubType;
}