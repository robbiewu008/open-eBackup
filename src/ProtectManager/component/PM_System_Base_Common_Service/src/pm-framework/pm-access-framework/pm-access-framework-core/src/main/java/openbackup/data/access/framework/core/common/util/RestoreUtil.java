package openbackup.data.access.framework.core.common.util;

import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;

import java.util.Objects;

/**
 * 功能描述
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/8/16
 */
public class RestoreUtil {
    /**
     * 转换恢复值为 副本操作的恢复值
     *
     * @param restoreType 恢复类型
     * @param targetLocation 目标位置
     * @return 副本操作的恢复值
     */
    public static String getRestoreOperation(String restoreType, RestoreLocationEnum targetLocation) {
        if (Objects.equals(restoreType, RestoreTypeEnum.IR.getType())) {
            return "LIVE_RESTORE";
        } else if (Objects.equals(restoreType, RestoreTypeEnum.FLR.getType())) {
            return "FLR_RESTORE";
        } else {
            if (Objects.equals(targetLocation, RestoreLocationEnum.NEW)) {
                return "NEW_LOCATION_RESTORE";
            } else {
                return "OLD_LOCATION_RESTORE";
            }
        }
    }
}
