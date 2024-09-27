/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.goldendb.protection.access.util;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.goldendb.protection.access.constant.GoldenDbConstant;
import openbackup.goldendb.protection.access.dto.instance.GoldenInstance;
import openbackup.system.base.common.utils.json.JsonUtil;

import lombok.extern.slf4j.Slf4j;

/**
 * 功能描述 GoldenDbValidator参数校验
 *
 * @author s30036254
 * @since 2023-03-20
 */
@Slf4j
public class GoldenDbInstanceValidator {
    private GoldenDbInstanceValidator() {
    }

    /**
     * 创建/更新GoldenDb实例时，校验参数
     *
     * @param resource 实例
     */
    public static void checkGoldenDbInstance(ProtectedResource resource) {
        log.info("begin to checkGoldenDbInstance params");
        GoldenInstance instance = getInstance(resource);
        checkGtmNode(instance);
    }

    private static void checkGtmNode(GoldenInstance instance) {
        if (instance.getGtm() == null || instance.getGtm().size() == 0) {
            // 根据业务需求，现在不需要gtm节点也可以进行注册
            log.warn("gtmNode size is zero");
        }
    }

    private static GoldenInstance getInstance(ProtectedResource resource) {
        String instanceJson = resource.getExtendInfoByKey(GoldenDbConstant.CLUSTER_INFO);
        return JsonUtil.read(instanceJson, GoldenInstance.class);
    }
}
