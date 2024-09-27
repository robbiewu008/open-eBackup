package openbackup.data.access.framework.core.common.enums.v2.filter;

/**
 * FilterTypeEnum
 *
 * @description: 资源过滤器中的资源类型，用于跟框架层使用
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/11/30
 **/
public enum FilterTypeEnum {
    /**
     * 虚拟机
     */
    VM("VM"),
    /**
     * 磁盘
     */
    DISK("Disk"),
    /**
     * 文件
     */
    FILE("File"),
    /**
     * 目录
     */
    DIR("Dir")
    ;

    FilterTypeEnum(String type) {
        this.type = type;
    }

    private String type;

    public String getType() {
        return type;
    }
}
