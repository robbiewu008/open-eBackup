package openbackup.system.base.common.enums;

/**
 * 组件名枚举
 *
 * @author w00607005
 * @since 2023-08-04
 */
public enum ComponentEnum {
    GAUSSDB("gaussdb");

    private final String name;

    ComponentEnum(String name) {
        this.name = name;
    }

    /**
     * 获取组件名称
     *
     * @return 组件名
     */
    public String getName() {
        return name;
    }
}
