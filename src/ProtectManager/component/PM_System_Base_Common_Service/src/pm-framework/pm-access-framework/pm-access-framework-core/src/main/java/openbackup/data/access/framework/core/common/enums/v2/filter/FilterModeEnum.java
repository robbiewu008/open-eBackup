package openbackup.data.access.framework.core.common.enums.v2.filter;

/**
 * FilterModeEnum
 *
 * @description: 资源过滤模式枚举类
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/11/30
 **/
public enum FilterModeEnum {
    INCLUDE("INCLUDE"),
    EXCLUDE("EXCLUDE");

    private String mode;

    FilterModeEnum(String mode) {
        this.mode = mode;
    }

    public String getMode() {
        return mode;
    }
}
