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
package openbackup.access.framework.resource.util;

import static openbackup.system.base.common.utils.JSONObject.RAW_OBJ_MAPPER;

import com.fasterxml.jackson.core.JsonProcessingException;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

/**
 * 资源util
 *
 */
public class ResourceUtil {
    /**
     * 存储类型map key设备类型关键词(resource.extend_info.sub_type)， value规范存储设备类型输出
     */
    private static final Map<String, String> STORAGE_TYPE_MAP = new HashMap<>();

    static {
        STORAGE_TYPE_MAP.put("Pacific", "OceanStor Pacific");
        STORAGE_TYPE_MAP.put("Dorado", "OceanStor Dorado");
        STORAGE_TYPE_MAP.put("OceanProtect", "OceanProtect");
    }

    /**
     * 不更新resource的dependencies的主机信息，将主机信息设为空
     *
     * @param resource 资源
     */
    public static void setUpdateDependencyHostInfoNull(ProtectedResource resource) {
        if (resource == null) {
            return;
        }
        Map<String, List<ProtectedResource>> resourceDependencies = resource.getDependencies();
        if (VerifyUtil.isEmpty(resourceDependencies)) {
            return;
        }
        for (Map.Entry<String, List<ProtectedResource>> entry : resourceDependencies.entrySet()) {
            List<ProtectedResource> valueResources = entry.getValue();
            if (VerifyUtil.isEmpty(valueResources)) {
                return;
            }
            for (int i = 0; i < valueResources.size(); i++) {
                ProtectedResource valueResource = valueResources.get(i);
                if (Objects.equals(valueResource.getType(), ResourceTypeEnum.HOST.getType())) {
                    ProtectedResource updateHostResource = new ProtectedResource();
                    updateHostResource.setUuid(valueResource.getUuid());
                    valueResources.set(i, updateHostResource);
                } else {
                    setUpdateDependencyHostInfoNull(resource);
                }
            }
        }
    }

    /**
     * 对于uuid为空的resource设置sourceType
     *
     * @param resource resource
     * @param sourceType sourceType
     */
    public static void supplySourceTypeWhenUuidNull(ProtectedResource resource, String sourceType) {
        if (Objects.isNull(resource)) {
            return;
        }
        if (Objects.isNull(resource.getUuid()) && Objects.isNull(resource.getSourceType())) {
            resource.setSourceType(sourceType);
        }
        if (resource.getDependencies() == null) {
            return;
        }
        for (Map.Entry<String, List<ProtectedResource>> entry : resource.getDependencies().entrySet()) {
            if (entry.getValue() != null) {
                entry.getValue().forEach(elem -> supplySourceTypeWhenUuidNull(elem, sourceType));
            }
        }
    }

    /**
     * 组合protectResource的属性值，若source属性值为空，则取target
     *
     * @param source source
     * @param target target
     * @return ProtectedResource or ProtectedEnvironment
     */
    public static ProtectedResource combineProtectedResource(ProtectedResource source, ProtectedResource target) {
        Class<?> sourceClass = source.getClass();
        Class<?> targetClass = target.getClass();
        if (sourceClass == ProtectedEnvironment.class || targetClass == ProtectedEnvironment.class) {
            return merge(ProtectedEnvironment.class, source, target, false);
        } else {
            return merge(ProtectedResource.class, source, target, false);
        }
    }

    /**
     * 数据合并
     *
     * @param type type
     * @param basic basic
     * @param delta delta
     * @param isOverwrite isOverwrite 若为true，则delta非空数据会覆盖前者； 若为false，basic不为空的数据不会被覆盖
     * @return result
     */
    public static <T, R extends T> T merge(Class<R> type, T basic, T delta, boolean isOverwrite) {
        if (basic == null) {
            return JSONObject.clone(delta);
        }
        T resource = JSONObject.clone(basic);
        if (delta == null) {
            return resource;
        }
        Map<String, Object> target = toMap(basic);
        Map<String, Object> source = toMap(delta);
        merge(target, source, isOverwrite);
        return toBean(target, type).orElse(null);
    }

    private static void merge(Map<String, Object> target, Map<String, Object> source, boolean isOverwrite) {
        for (Map.Entry<String, Object> entry : source.entrySet()) {
            String key = entry.getKey();
            Object targetValue = target.get(key);
            Object sourceValue = entry.getValue();
            if (targetValue instanceof Map && sourceValue instanceof Map) {
                merge((Map<String, Object>) targetValue, (Map<String, Object>) sourceValue, isOverwrite);
                continue;
            }
            if (!isOverwrite) {
                if (targetValue == null) {
                    target.put(key, sourceValue);
                }
                continue;
            }
            if (sourceValue != null) {
                target.put(key, sourceValue);
            }
        }
    }

    private static Map<String, Object> toMap(Object object) {
        return toBean(object, Map.class).orElse(new HashMap<>());
    }

    private static <T> Optional<T> toBean(Object data, Class<T> type) {
        if (data == null) {
            return Optional.empty();
        }
        try {
            return Optional.ofNullable(RAW_OBJ_MAPPER.readValue(RAW_OBJ_MAPPER.writeValueAsString(data), type));
        } catch (JsonProcessingException e) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "data error", e);
        }
    }

    /**
     * 转换存储设备类型
     *
     * @param originalType 输入
     * @return 类型值
     */
    public static String convertStorageType(String originalType) {
        for (Map.Entry<String, String> entry : STORAGE_TYPE_MAP.entrySet()) {
            if (originalType.contains(entry.getKey())) {
                return entry.getValue();
            }
        }
        return originalType;
    }
}
