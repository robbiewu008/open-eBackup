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
package openbackup.system.base.service;

import openbackup.system.base.common.constants.CacheConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.DataMoverCheckedException;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;
import openbackup.system.base.util.ZKDistributeLock;

import com.fasterxml.jackson.core.type.TypeReference;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.function.Supplier;

import javax.annotation.PostConstruct;

/**
 * CacheService
 *
 */
@Component
@Slf4j
public class CacheService {
    private final Map<String, Supplier<Object>> functionMap = new HashMap<>();

    private final RedissonClient redissonClient;

    private final InfrastructureRestApi infrastructureRestApi;

    /**
     * constructor
     *
     * @param redissonClient redissonClient
     * @param infrastructureRestApi infrastructureRestApi
     */
    public CacheService(RedissonClient redissonClient, InfrastructureRestApi infrastructureRestApi) {
        this.redissonClient = redissonClient;
        this.infrastructureRestApi = infrastructureRestApi;
    }

    /**
     * 初始化缓存刷新字段与方法
     */
    @PostConstruct
    public void init() {
        functionMap.put(CacheConstant.PROTECT_ENGINE_POD_INFO_CACHE, this::getProtectEnginePodInfo);
    }

    /**
     * 刷新缓存 应该可以写成自动过期的形式，但是改单没时间了
     */
    @Scheduled(initialDelay = CacheConstant.INITIAL_DELAY, fixedRate = CacheConstant.PERIOD)
    @ZKDistributeLock(lockName = CacheConstant.CACHE_REDIS_UPDATE, tryLockTime = 0)
    public void fresh() {
        RMap<String, String> cacheMap = redissonClient.getMap(CacheConstant.CACHE_REDIS_KEY, StringCodec.INSTANCE);
        for (Map.Entry<String, Supplier<Object>> entry : functionMap.entrySet()) {
            log.debug("start fresh cache,cache key:{}", entry.getKey());
            cacheMap.put(entry.getKey(), JsonUtil.json(entry.getValue().get()));
        }
    }

    /**
     * 获取redis里的缓存值
     *
     * @param key redis key
     * @param typeReference class
     * @param <T> T
     * @return value
     */
    public <T> Optional<T> get(String key, TypeReference<T> typeReference) {
        if (!functionMap.containsKey(key)) {
            return Optional.empty();
        }
        RMap<String, String> cacheMap = redissonClient.getMap(CacheConstant.CACHE_REDIS_KEY, StringCodec.INSTANCE);
        if (cacheMap.containsKey(key)) {
            String vlaueString = cacheMap.get(key);
            try {
                return Optional.ofNullable(JsonUtil.read(vlaueString, typeReference));
            } catch (DataMoverCheckedException e) {
                log.error("type convert filed", e);
                return getValueFromSupplier(key, cacheMap, typeReference);
            }
        }
        return getValueFromSupplier(key, cacheMap, typeReference);
    }

    private <T> Optional<T> getValueFromSupplier(String key, RMap<String, String> cacheMap,
        TypeReference<T> typeReference) {
        Object value = functionMap.get(key).get();
        if (value == null) {
            return Optional.empty();
        }
        cacheMap.put(key, JsonUtil.json(value));
        return Optional.ofNullable(JsonUtil.read(value, typeReference));
    }

    private List<NodePodInfo> getProtectEnginePodInfo() {
        InfraResponseWithError<List<NodePodInfo>> response = infrastructureRestApi.getInfraPodInfo("protectengine");
        if (response == null || VerifyUtil.isEmpty(response.getData())) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "can not get protectengine pod-node relation");
        }
        return response.getData();
    }
}
