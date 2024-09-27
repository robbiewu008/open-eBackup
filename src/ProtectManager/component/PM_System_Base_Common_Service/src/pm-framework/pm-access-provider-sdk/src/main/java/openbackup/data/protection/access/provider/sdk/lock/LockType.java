package openbackup.data.protection.access.provider.sdk.lock;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 资源锁类型
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/26
 **/
public enum LockType {
    READ("r"),
    WRITE("w");

    private final String type;

    LockType(String type) {
        this.type = type;
    }

    /**
     * 获取资源锁类型
     *
     * @return 资源锁类型
     */
    @JsonValue
    public String getType() {
        return type;
    }
}
