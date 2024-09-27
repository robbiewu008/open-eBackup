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
package com.huawei.emeistor.console.config.lock;

import com.huawei.emeistor.console.config.lock.entity.LockEntity;
import com.huawei.emeistor.console.config.lock.mapper.LockMapper;
import com.huawei.emeistor.console.util.ExceptionUtil;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.conditions.update.LambdaUpdateWrapper;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.time.DateUtils;
import org.springframework.dao.CannotSerializeTransactionException;
import org.springframework.dao.DataIntegrityViolationException;
import org.springframework.dao.DuplicateKeyException;
import org.springframework.jdbc.BadSqlGrammarException;
import org.springframework.jdbc.UncategorizedSQLException;
import org.springframework.stereotype.Component;
import org.springframework.transaction.support.TransactionTemplate;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Collections;
import java.util.Date;
import java.util.Set;
import java.util.UUID;
import java.util.WeakHashMap;

/**
 * 基于数据库的分布式锁服务
 *
 * @author w30042425
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-06-10
 */
@Slf4j
@Component
public class SQLLockService extends ServiceImpl<LockMapper, LockEntity> {
    /**
     * 默认持有锁的时间1小时（适配上传agent包等耗时操作）
     */
    private static final int DEFAULT_LOCK_TIME = 1;

    private static final String HOSTNAME = initHostname();

    /**
     * 锁名称注册
     */
    private final Set<String> lockRecords = Collections.synchronizedSet(Collections.newSetFromMap(new WeakHashMap<>()));

    private final LockMapper lockMapper;

    private final TransactionTemplate transactionTemplate;

    /**
     * 构造方法
     *
     * @param lockMapper lockMapper
     * @param transactionTemplate 事务Template
     */
    public SQLLockService(LockMapper lockMapper, TransactionTemplate transactionTemplate) {
        this.lockMapper = lockMapper;
        this.transactionTemplate = transactionTemplate;
    }

    /**
     * 创建/获得锁
     *
     * @param key 锁的key
     * @return 是否获取成功
     */
    public boolean createLock(String key) {
        if (!lockRecords.contains(key)) {
            if (insertRecord(key)) {
                lockRecords.add(key);
                return true;
            }
            lockRecords.add(key);
        }
        return updateRecord(key);
    }

    /**
     * 释放锁
     *
     * @param key 锁的key
     */
    public void unLock(String key) {
        this.transactionTemplate.executeWithoutResult(transactionStatus -> {
            LambdaUpdateWrapper<LockEntity> wrapper = new LambdaUpdateWrapper<LockEntity>().set(
                LockEntity::getUnlockTime, new Date()).eq(LockEntity::getKey, key);
            this.update(wrapper);
        });
    }

    /**
     * 插入一条锁记录
     *
     * @param key 锁的key
     * @return 是否插入成功
     */
    public boolean insertRecord(String key) {
        try {
            return Boolean.TRUE.equals(transactionTemplate.execute(status -> {
                LockEntity entity = buildDefaultLockEntity(key);
                QueryWrapper<LockEntity> wrapper = new QueryWrapper<>();
                wrapper.eq("key", key);
                int count = lockMapper.selectCount(wrapper).intValue();
                if (count > 0) {
                    return updateRecord(key);
                } else {
                    return lockMapper.insert(entity) > 0;
                }
            }));
        } catch (DuplicateKeyException | CannotSerializeTransactionException e) {
            return false;
        } catch (DataIntegrityViolationException | BadSqlGrammarException | UncategorizedSQLException e) {
            log.warn("Unexpected exception", ExceptionUtil.getErrorMessage(e));
            return false;
        }
    }

    /**
     * 更新锁记录
     *
     * @param key 锁name
     * @return 是否更新成功
     */
    public boolean updateRecord(String key) {
        return Boolean.TRUE.equals(this.transactionTemplate.execute(status -> {
            Date now = new Date();
            Date unLockTime = DateUtils.addHours(now, DEFAULT_LOCK_TIME);
            LambdaUpdateWrapper<LockEntity> updateWrapper = new LambdaUpdateWrapper<LockEntity>().set(
                    LockEntity::getLockTime, now)
                .set(LockEntity::getUnlockTime, unLockTime)
                .eq(LockEntity::getKey, key)
                .le(LockEntity::getUnlockTime, now);
            return this.update(updateWrapper);
        }));
    }

    private LockEntity buildDefaultLockEntity(String key) {
        Date now = new Date();
        Date date = DateUtils.addHours(now, DEFAULT_LOCK_TIME);
        return LockEntity.builder()
            .id(UUID.randomUUID().toString().replaceAll("-", ""))
            .lockTime(now).unlockTime(date).key(key).owner(HOSTNAME)
            .build();
    }

    private static String initHostname() {
        try {
            return InetAddress.getLocalHost().getHostName();
        } catch (UnknownHostException e) {
            return "unknown";
        }
    }
}
