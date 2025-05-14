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
package com.huawei.oceanprotect.system.base.initialize;

import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.RedisTimeoutException;

import java.util.concurrent.TimeUnit;

/**
 * 监控是否正在进行初始化或网络配置修改
 *
 * @author l00347293
 * @since 2021-04-12
 */
@Slf4j
public class CheckStatusThread extends Thread {
    // 等待时间15s
    private static final long DEFAULT_WAIT_SECOND = 15 * 1000L;

    /**
     * 线程是否退出
     */
    private volatile boolean isExist = false;

    private RedissonClient redissonClient;

    private String statusFlag;

    /**
     * 线程传参
     *
     * @param redissonClient 更新redis数据
     * @param statusFlag redis Map的key
     */
    public CheckStatusThread(RedissonClient redissonClient, String statusFlag) {
        super.setName("CheckStatusThread");
        this.redissonClient = redissonClient;
        this.statusFlag = statusFlag;
    }

    /**
     * 初始化执行
     */
    @Override
    @ExterAttack
    public void run() {
        RMap redisMap = redissonClient.getMap(InitConfigConstant.INIT_RUNNING_FLAG);
        try {
            while (!isExist) {
                setCheckStatus(redisMap);
                Thread.sleep(DEFAULT_WAIT_SECOND);
            }
        } catch (InterruptedException exception) {
            log.error("monitor thread failed: ", exception);
        }
    }

    /**
     * 设置退出标识
     *
     * @param isExist 退出线程的标识
     */
    public void setExitFlag(boolean isExist) {
        this.isExist = isExist;
    }

    private void setCheckStatus(RMap redisMap) {
        try {
            log.info("current status is: {}", redisMap.get(statusFlag));
            redisMap.put(statusFlag, InitConfigConstant.RUNNING_STATUS_VALUE_FLAG);
            redisMap.expire(InitConfigConstant.INIT_STATUS_EXPIRE_TIMES, TimeUnit.MINUTES);
        } catch (RedisTimeoutException exception) {
            log.error("connect redis failed: ", exception);
        }
    }
}
