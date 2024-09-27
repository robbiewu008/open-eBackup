package openbackup.data.protection.access.provider.sdk.anti.ransomware;

import java.util.List;
import java.util.Map;

/**
 * 副本勒索服务
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/8/16
 */
public interface CopyRansomwareService {
    /**
     * 检查防勒索感染副本操作是否有效
     *
     * @param copyId 副本ID
     * @param operation 副本操作
     */
    void checkCopyOperationValid(String copyId, String operation);

    /**
     * 检查防勒索感染副本操作是否有效,并返回结果
     *
     * @param copyIds 副本列表
     * @param operation 操作
     * @return 副本操作是否有效，副本是否支持该操作
     */
    Map<String, Boolean> checkCopyOperationValidWithMap(List<String> copyIds, String operation);


    /**
     * 检查副本是否感染,并返回结果
     *
     * @param copyIds 副本列表
     * @return 副本操作是否有效，副本是否支持该操作
     */
    Map<String, Boolean> checkCopyInfectedWithMap(List<String> copyIds);
}
