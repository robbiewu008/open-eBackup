package openbackup.system.base.common.model.repository.enums;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonEnumDefaultValue;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.annotation.JsonValue;

import lombok.AllArgsConstructor;
import lombok.Getter;

import java.util.HashMap;
import java.util.Map;

/**
 * 存储池运行状态
 *
 * @author w00493811
 * @since 2020-12-08
 */
@Getter
@AllArgsConstructor
public enum StoragePoolRunningStatus {
    /**
     * 预拷贝
     */
    PRE_COPY("14"),
    /**
     * 重构
     */
    RECONSTRUCTION("16"),
    /**
     * 在线
     */
    ONLINE("27"),
    /**
     * 离线
     */
    @JsonEnumDefaultValue OFFLINE("28"),
    /**
     * 部分在线
     */
    PARTIALLY_ONLINE("30"),
    /**
     * 正在均衡
     */
    BALANCING("32"),
    /**
     * 初始化中
     */
    INITIALIZING("53"),
    /**
     * 删除中
     */
    DELETING("106"),
    /**
     * 故障
     */
    FAULTY("1"),
    /**
     * 写保护
     */
    WRITE_PROTECTED("2"),
    /**
     * 停止
     */
    STOPPED("3"),
    /**
     * 故障且写保护
     */
    FAULTY_AND_WRITE_PROTECTED("4"),
    /**
     * 数据迁移
     */
    MIGRATING_DATA("5"),
    /**
     * 降级
     */
    DEGRADED("7"),
    /**
     * 数据重构
     */
    REBUILDING_DATA("8"),
    /**
     * 删除中
     */
    PACIFIC_DELETING("9"),
    /**
     * 删除失败
     */
    DELETION_FAILED("10");

    /**
     * 常量图
     */
    private static final Map<String, StoragePoolRunningStatus> VALUE_ENUM_MAP = new HashMap<>(
        StoragePoolRunningStatus.values().length);

    static {
        for (StoragePoolRunningStatus each : StoragePoolRunningStatus.values()) {
            VALUE_ENUM_MAP.put(each.getRunningStatus(), each);
        }
    }

    /**
     * 在线状态
     */
    private static final int ONLINE_STATUS = 0;

    /**
     * 部分在线
     */
    private static final int PARTIALLY_ONLINE_STATUS = 1;

    /**
     * 离线状态
     */
    private static final int OFFLINE_STATUS = -1;

    /**
     * 存储池运行状态
     */
    private final String runningStatus;

    @JsonValue
    @Override
    public String toString() {
        return getRunningStatus();
    }

    /**
     * 和其他存储池运行状态（字符串）对比
     *
     * @param otherStoragePoolRunningStatus 其他存储池运行状态
     * @return 是否相同
     */
    public boolean equalsStatus(String otherStoragePoolRunningStatus) {
        return getRunningStatus().equals(otherStoragePoolRunningStatus);
    }

    /**
     * 获取在线状态
     *
     * @return 在线状态
     */
    public int getOnlineStatus() {
        if (getRunningStatus().equals(ONLINE.getRunningStatus())) {
            return ONLINE_STATUS;
        } else if (getRunningStatus().equals(PARTIALLY_ONLINE.getRunningStatus())) {
            return PARTIALLY_ONLINE_STATUS;
        } else {
            return OFFLINE_STATUS;
        }
    }

    /**
     * 创建
     *
     * @param runningStatus 存储池运行状态
     * @return 存储池运行状态
     */
    @JsonCreator(mode = JsonCreator.Mode.PROPERTIES)
    public static StoragePoolRunningStatus forValues(@JsonProperty("RUNNINGSTATUS") String runningStatus) {
        return VALUE_ENUM_MAP.get(runningStatus);
    }
}