package openbackup.data.protection.access.provider.sdk.enums;

import java.util.Arrays;

/**
 * 存储库协议枚举类
 * <p>
 * A8000本地存储均已NATIVE_<协议名>来定义
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/8
 **/
public enum RepositoryProtocolEnum {
    CIFS(0),
    NFS(1),
    S3(2),
    BLOCK(3),
    LOCAL_DIR(4),
    /**
     * A8000 NFS存储
     */
    NATIVE_NFS(5),
    /**
     * A8000 CIFS存储
     */
    NATIVE_CIFS(6),
    /**
     * 磁带库存储
     */
    TAPE(7)
    ;

    private final int protocol;

    RepositoryProtocolEnum(int protocol) {
        this.protocol = protocol;
    }

    public int getProtocol() {
        return protocol;
    }

    /**
     * 根据协议类型获取存储库协议的枚举类
     *
     * @param protocol 协议类型
     * @return 存储库协议枚举类 {@code RepositoryProtocolEnum}
     */
    public static RepositoryProtocolEnum getByProtocol(int protocol) {
        return Arrays.stream(RepositoryProtocolEnum.values())
                .filter(location -> location.protocol == protocol)
                .findFirst()
                .orElseThrow(IllegalArgumentException::new);
    }
}
