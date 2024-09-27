/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.obs.plugin.provider;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.obs.plugin.common.constants.EnvironmentConstant;
import openbackup.obs.plugin.entity.BucketInfoEntity;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.fasterxml.jackson.core.type.TypeReference;
import com.google.common.collect.ImmutableMap;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Objects;

/**
 * OBS对象集合Provider
 *
 * @author c30035089
 * @since 2023-11-15
 */
@Component
@Slf4j
public class ObjectSetProvider implements ResourceProvider {
    private static final int MAX_PREFIX_LEN = 1024;

    private static final int MAX_PREFIX_NUM = 256;

    private static final int MAX_BUCKET_NUM = 100;

    /**
     * 允许创建OBS对象集合的最大数量
     */
    private static final int OBJECT_SET_MAX_COUNT = 256;

    private final ResourceService resourceService;

    public ObjectSetProvider(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && Objects.equals(object.getSubType(), ResourceSubTypeEnum.OBJECT_SET.getType());
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        beforeUpdate(resource);
        checkOBSCount(resource);
        // 如果对象集合的扩展信息中storageType为空，设置值
        if (StringUtils.isEmpty(resource.getExtendInfo().get(EnvironmentConstant.KEY_STORAGE_TYPE))) {
            resource.getExtendInfo().put(EnvironmentConstant.KEY_STORAGE_TYPE,
                resource.getEnvironment().getExtendInfoByKey(EnvironmentConstant.KEY_STORAGE_TYPE));
        }
    }

    @Override
    public boolean isSupportIndex() {
        return true;
    }

    private void checkPrefix(ProtectedResource resource) {
        String bucketList = resource.getExtendInfoByKey(EnvironmentConstant.KEY_BUCKETLIST);
        if (StringUtils.isNotEmpty(bucketList)) {
            List<BucketInfoEntity> buckets = JsonUtil.read(bucketList, new TypeReference<List<BucketInfoEntity>>() {});
            if (buckets.size() > MAX_BUCKET_NUM) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                    "The OBS set buckets exceeds the maximum num");
            }
            for (BucketInfoEntity bucket : buckets) {
                if (CollectionUtils.isEmpty(bucket.getPrefix())) {
                    continue;
                }
                if (bucket.getPrefix().size() > MAX_PREFIX_NUM) {
                    throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                        "The OBS set prefix exceeds the maximum num");
                }
                bucket.getPrefix().forEach(e -> {
                    if (e.length() > MAX_PREFIX_LEN) {
                        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                            "The OBS set prefix exceeds the maximum length");
                    }
                });
            }
        }
    }

    private void checkOBSCount(ProtectedResource resource) {
        PageListResponse<ProtectedResource> resources = resourceService.query(LegoNumberConstant.ZERO,
            LegoNumberConstant.ONE,
            ImmutableMap.of("rootUuid", resource.getRootUuid(), "subType", ResourceSubTypeEnum.OBJECT_SET.getType()));
        if (resources.getTotalCount() >= OBJECT_SET_MAX_COUNT) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_NUM_EXCEED_LIMIT,
                new String[] {String.valueOf(OBJECT_SET_MAX_COUNT)}, "The number of object set exceeds the maximum.");
        }
    }

    private void checkName(String name) {
        if (!EnvironmentConstant.NAME_PATTERN.matcher(name).matches()) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "OBS set name is not pattern");
        }
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        checkName(resource.getName());
        checkPrefix(resource);
        // 复制任务不允许path为空
        ProtectedEnvironment environment = resource.getEnvironment();
        resource.setPath(environment.getName());
        PageListResponse<ProtectedResource> environments = resourceService.query(LegoNumberConstant.ZERO,
            LegoNumberConstant.ONE, Collections.singletonMap("uuid", resource.getRootUuid()));
        checkEnvironment(environments);
    }

    private void checkEnvironment(PageListResponse<ProtectedResource> environments) {
        if (environments.getRecords().size() == 0) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "The object storage is not exist.");
        }

        ProtectedResource record = environments.getRecords().stream().findFirst().get();
        if (!(record instanceof ProtectedEnvironment)) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "The object storage is illegal.");
        }

        ProtectedEnvironment protectedEnvironment = (ProtectedEnvironment) record;
        if (!String.valueOf(LinkStatusEnum.ONLINE.getStatus())
            .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(protectedEnvironment))) {
            log.error("object storage check, the object storage is offline, uuid:{}", protectedEnvironment.getUuid());
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE,
                "The object storage is offline.");
        }
    }
}
