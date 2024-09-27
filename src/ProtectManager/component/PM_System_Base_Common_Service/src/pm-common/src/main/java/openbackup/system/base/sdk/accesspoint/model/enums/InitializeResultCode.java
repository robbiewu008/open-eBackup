package openbackup.system.base.sdk.accesspoint.model.enums;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.annotation.JsonValue;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.ToString;

import java.util.HashMap;
import java.util.Map;

/**
 * 初始化动作结果编码
 *
 * @author w00493811
 * @since 2020-12-26
 */
@Getter
@ToString
@AllArgsConstructor
public enum InitializeResultCode {
    /**
     * 成功
     */
    SUCCESS(0),

    /**
     * 失败
     */
    FAILURE(-1),

    /**
     * 查询失败，或者没有任何节点
     */
    ERROR_NO_NODE(10000),

    /**
     * 查询失败，节点数量不等于路径数量
     */
    ERROR_NODE_SIZE_NOT_EQUAL_PATH_SIZE(10001),

    /**
     * 挂载失败
     */
    ERROR_MOUNT_FAILED(10002),

    /**
     * 创建标准备份卷失败
     */
    ERROR_VOLUME_TYPE_STANDARD_BACKUP_FAILED(10003),
    /**
     * 创建元数据卷失败
     */
    ERROR_VOLUME_TYPE_METADATA_BACKUP_FAILED(10004),
    /**
     * 创建云备份索引卷失败
     */
    ERROR_VOLUME_TYPE_COULD_BACKUP_FAILED(10005),
    /**
     * 创建自备份卷失败
     */
    ERROR_VOLUME_TYPE_SELF_BACKUP_FAILED(10006);

    /**
     * 常量图
     */
    private static final Map<String, InitializeResultCode> VALUE_ENUM_MAP = new HashMap<>(
        InitializeResultCode.values().length);

    static {
        for (InitializeResultCode each : InitializeResultCode.values()) {
            VALUE_ENUM_MAP.put(each.toString(), each);
        }
    }

    /**
     * 错误编码
     */
    private final int code;

    /**
     * 是否成功
     *
     * @return 是否成功
     */
    public boolean isOk() {
        return this == SUCCESS;
    }

    /**
     * 字符串表达
     *
     * @return 字符串
     */
    @JsonValue
    @Override
    public String toString() {
        return String.valueOf(getCode());
    }

    /**
     * 创建
     *
     * @param theCode 编码
     * @return 编码
     */
    @JsonCreator(mode = JsonCreator.Mode.PROPERTIES)
    public static InitializeResultCode forValues(@JsonProperty("code") String theCode) {
        return VALUE_ENUM_MAP.get(theCode);
    }
}
