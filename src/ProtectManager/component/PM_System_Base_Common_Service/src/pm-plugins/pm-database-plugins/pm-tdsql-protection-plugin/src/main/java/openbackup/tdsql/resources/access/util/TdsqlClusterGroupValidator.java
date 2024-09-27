/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.util;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.instance.DataNode;
import openbackup.tdsql.resources.access.dto.instance.TdsqlGroup;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;

import java.util.List;

/**
 * TdsqlGroupValidator参数校验
 *
 * @author z00445440
 * @since 2023-11-14
 */
@Slf4j
public class TdsqlClusterGroupValidator {
    /**
     * 创建/更新TDSQL分布式实例时，校验参数
     *
     * @param resource 实例
     */
    public static void checkTdsqlGroupParams(ProtectedResource resource) {
        log.info("begin to checkTdsqlGroup params");
        TdsqlGroup tdsqlGroup = JsonUtil.read(
            resource.getExtendInfoByKey(TdsqlConstant.CLUSTER_GROUP_INFO), TdsqlGroup.class);
        if (tdsqlGroup.getGroup() == null) {
            log.warn("tdsql group info is null");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "tdsql group info is null");
        }
        checkSetParams(tdsqlGroup.getGroup().getSetIds());
        checkDataNodeParams(tdsqlGroup.getGroup().getDataNodes());
    }

    private static void checkSetParams(List<String> setIds) {
        if (setIds == null || setIds.size() == 0) {
            log.warn("tdsql group setIds size is zero");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "tdsql group setIds size is zero");
        }
    }

    private static void checkDataNodeParams(List<DataNode> dataNodes) {
        if (dataNodes == null || dataNodes.size() == 0) {
            log.warn("tdsql group dataNodes size is zero");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "tdsql group dataNodes size is zero");
        }
        dataNodes.forEach(dataNode -> {
            if (!StringUtils.isNotBlank(dataNode.getIp()) || !StringUtils.isNotBlank(dataNode.getParentUuid())) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "tdsql group dataNode param is empty");
            }
        });
    }
}
