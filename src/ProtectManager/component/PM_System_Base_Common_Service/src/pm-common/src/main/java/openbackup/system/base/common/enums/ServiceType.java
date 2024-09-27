package openbackup.system.base.common.enums;

/**
 * 服务类型
 *
 * @author xwx1016404
 * @since 2021-10-10
 */
public enum ServiceType {
    /**
     * SFTP服务
     */
    SFTP("SFTP_SERVICE"),
    /**
     * 标准备份服务
     */
    STANDARD("STANDARD_SERVICE");

    private final String type;

    /**
     * 默认构造函数
     *
     * @param type 服务类型
     */
    ServiceType(String type) {
        this.type = type;
    }

    /**
     * 获取服务类型
     *
     * @return type
     */
    public String getType() {
        return type;
    }
}