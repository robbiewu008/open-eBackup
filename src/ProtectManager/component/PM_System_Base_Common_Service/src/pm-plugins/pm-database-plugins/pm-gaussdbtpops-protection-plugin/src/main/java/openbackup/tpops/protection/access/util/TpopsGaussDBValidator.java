package openbackup.tpops.protection.access.util;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;
import openbackup.tpops.protection.access.constant.TpopsGaussDBErrorCode;

import lombok.extern.slf4j.Slf4j;

import java.util.List;

/**
 * 功能描述: 校验 GaussDb各种参数的基本格式等信息
 *
 * @author x30021699
 * @since 2023-05-09
 */
@Slf4j
public class TpopsGaussDBValidator {
    /**
     * 检查已经接入的 GaussDb cluster是否已经超过规格
     *
     * @param existingEnvironments 已接入的GaussDb环境列表
     */
    public static void checkGaussDbCount(List<ProtectedEnvironment> existingEnvironments) {
        if (existingEnvironments.size() >= TpopsGaussDBConstant.GAUSSDB_CLUSTER_MAX_COUNT) {
            throw new LegoCheckedException(TpopsGaussDBErrorCode.GAUSSDB_RESOURCE_REACHED_THE_UPPER_LIMIT,
                new String[] {String.valueOf(TpopsGaussDBConstant.GAUSSDB_CLUSTER_MAX_COUNT)});
        }
    }
}