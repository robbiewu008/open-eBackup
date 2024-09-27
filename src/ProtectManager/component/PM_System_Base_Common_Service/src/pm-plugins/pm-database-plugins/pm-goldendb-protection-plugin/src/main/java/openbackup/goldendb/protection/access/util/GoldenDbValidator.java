package openbackup.goldendb.protection.access.util;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.MapUtils;

import java.util.Map;

/**
 * 功能描述 GoldenDbValidator参数校验
 *
 * @author s30036254
 * @since 2023-02-06
 */
@Slf4j
public class GoldenDbValidator {
    private GoldenDbValidator() {
    }

    /**
     * 创建/更新GoldenDb集群时，校验参数
     *
     * @param protectedEnvironment 集群环境
     */
    public static void checkGoldenDb(ProtectedEnvironment protectedEnvironment) {
        verifyEnvName(protectedEnvironment.getName());
        Map<String, String> extendInfo = protectedEnvironment.getExtendInfo();
        if (MapUtils.isEmpty(extendInfo)) {
            log.error("GoldenDb cluster extendInfo is null.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "GoldenDb cluster extendInfo is null.");
        }
    }

    private static void verifyEnvName(String name) {
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(name);
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(name);
    }
}
