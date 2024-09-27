/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * @author z00633516
 * @since 2022-03-17
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
