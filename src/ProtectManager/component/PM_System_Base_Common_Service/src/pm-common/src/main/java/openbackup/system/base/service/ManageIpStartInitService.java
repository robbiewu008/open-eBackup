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

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.redis.RedisSetService;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.CommandLineRunner;
import org.springframework.stereotype.Component;

import java.util.Set;

/**
 * manage ip init service, will write current ip to redis depends on have manage ip or not
 *
 */
@Component
@Slf4j
public class ManageIpStartInitService implements CommandLineRunner {
    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private NetworkService networkService;

    @Autowired
    private RedisSetService redisSetService;

    /**
     * 初始化时 如果当前节点有管理ip 则加入到高优先级列表 否则加入低优先级
     *
     * @param args args
     * @throws Exception Exception
     */
    @Override
    public void run(String... args) throws Exception {
        if (!deployTypeService.isXSeries()) {
            return;
        }
        String currentIp = System.getenv("POD_IP");
        if (VerifyUtil.isEmpty(currentIp)) {
            log.error("Fail to get current ip, will not add to any redis set");
            return;
        }
        // 如果当前node有管理ip 则应该加入到管理ip列表
        // 为了保证方法纯粹性高优先级不在此处维护 如果没有则需要从对应列表移除
        if (networkService.isNodeHaveManageIp(true)) {
            log.info("current node:{} have manage ip, will add to manage ip list.", currentIp);
            addToPriorityList(Constants.NODE_WITH_MANAGE_IP_CACHE_KEY, currentIp);
        } else {
            log.info("current node:{} not have manage ip, will remove from manage ip list.", currentIp);
            removeFromPriorityList(Constants.NODE_WITH_MANAGE_IP_CACHE_KEY, currentIp);
        }
    }

    /**
     * 添加ip到对应的redis缓存中
     *
     * @param cacheKey redis缓存 高优先级 低优先级
     * @param currentIp 需要添加的ip
     */
    public void addToPriorityList(String cacheKey, String currentIp) {
        redisSetService.addToSet(cacheKey, currentIp);
    }

    /**
     * 将当前的ip从redis列表中移除
     *
     * @param cacheKey redis缓存key
     * @param currentIp 需要删除的ip
     */
    public void removeFromPriorityList(String cacheKey, String currentIp) {
        redisSetService.removeFromSet(cacheKey, currentIp);
    }

    /**
     * 将当前ip列表添加到redis缓存中
     *
     * @param ipSet IP列表
     * @param cacheKey 缓存key
     */
    public void setToCache(Set<String> ipSet, String cacheKey) {
        redisSetService.mergeSets(cacheKey, ipSet);
    }
}
