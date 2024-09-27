package openbackup.data.protection.access.provider.sdk.restore;

import lombok.Data;

/**
 * 恢复的一些特性开关
 *
 * @author h30027154
 * @since 2022-07-01
 */
@Data
public class RestoreFeature {
    // 检查目标环境在线
    private boolean shouldCheckEnvironmentIsOnline = true;

    /**
     * 默认特性值
     *
     * @return RestoreFeture
     */
    public static RestoreFeature defaultValue() {
        return new RestoreFeature();
    }
}
