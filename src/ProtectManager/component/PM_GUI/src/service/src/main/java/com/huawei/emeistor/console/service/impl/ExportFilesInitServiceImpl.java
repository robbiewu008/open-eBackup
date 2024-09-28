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
package com.huawei.emeistor.console.service.impl;

import com.huawei.emeistor.console.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.redisson.api.RList;
import org.redisson.api.RedissonClient;
import org.springframework.boot.CommandLineRunner;
import org.springframework.stereotype.Service;

/**
 * clear read_export_file_list
 *
 */
@Slf4j
@Service
public class ExportFilesInitServiceImpl implements CommandLineRunner {
    /**
     * 读取锁
     */
    private static final String READ_LOCK_PREFIX = "read_export_file_list";

    private final RedissonClient redissonClient;

    /**
     * 构造方法
     *
     * @param redissonClient redissonClient
     */
    public ExportFilesInitServiceImpl(RedissonClient redissonClient) {
        this.redissonClient = redissonClient;
    }

    /**
     * 启动服务后，清理redis中可能残留（pod重启）的read_export_file_list
     *
     * @param args 启动参数
     */
    @Override
    @ExterAttack
    public void run(String... args) {
        RList<Object> readFileIdList = redissonClient.getList(READ_LOCK_PREFIX);
        readFileIdList.clear();
        log.info("clear read_export_file_list success.");
    }
}
