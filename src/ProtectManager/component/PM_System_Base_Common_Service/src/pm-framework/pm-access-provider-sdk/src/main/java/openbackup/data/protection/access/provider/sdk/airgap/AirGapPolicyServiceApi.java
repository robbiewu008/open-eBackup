package openbackup.data.protection.access.provider.sdk.airgap;

import java.util.List;

/**
 * airGap-policy对外接口
 *
 * @author z30027603
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2022-08-3
 */
public interface AirGapPolicyServiceApi {
    /**
     * query a live mount policy by id
     *
     * @param id id
     * @return boolean 是否存在
     */
    boolean existPolicy(String id);

    /**
     * 策略id列表
     *
     * @return 策略id列表
     */
    List<String> getAirGapPolicyIdList();
}
