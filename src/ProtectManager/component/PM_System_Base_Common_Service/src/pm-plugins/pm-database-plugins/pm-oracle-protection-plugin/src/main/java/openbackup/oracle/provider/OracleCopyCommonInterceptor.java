/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.oracle.provider;

import openbackup.data.protection.access.provider.sdk.copy.CopyCommonInterceptor;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableList;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Optional;

/**
 * oracle 副本拦截器
 *
 * @author w30044259
 * @since 2024-05-07
 */
@Component
@Slf4j
public class OracleCopyCommonInterceptor implements CopyCommonInterceptor {
    private static final List<String> ORACLE_SUB_TYPE_LIST = ImmutableList.of(ResourceSubTypeEnum.ORACLE.getType(),
            ResourceSubTypeEnum.ORACLE_CLUSTER.getType(), ResourceSubTypeEnum.ORACLE_CLUSTER_ENV.getType(),
            ResourceSubTypeEnum.ORACLE_CLUSTER_INSTANCE.getType());

    @Override
    public boolean applicable(String object) {
        return ORACLE_SUB_TYPE_LIST.contains(object);
    }

    @Override
    public void backupBuildCopyPostprocess(CopyInfo copyInfo) {
        JSONObject resourceProperties = JSONObject.fromObject(copyInfo.getResourceProperties());
        copyInfo.setIsStorageSnapshot(Optional.ofNullable(resourceProperties.getJSONObject("ext_parameters"))
            .orElseGet(JSONObject::new)
            .getBoolean("storage_snapshot_flag", false));
    }
}
