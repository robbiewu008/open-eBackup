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
package openbackup.system.base.pack.lock;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.conditions.update.LambdaUpdateWrapper;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.cluster.BackupClusterConfigUtil;
import openbackup.system.base.common.constants.StatefulsetConstants;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.pack.lock.entity.LockEntity;
import openbackup.system.base.pack.lock.mapper.LockMapper;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.beans.NodeDetail;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;

import org.apache.commons.lang3.time.DateUtils;
import org.springframework.beans.factory.SmartInitializingSingleton;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.dao.CannotSerializeTransactionException;
import org.springframework.dao.DataIntegrityViolationException;
import org.springframework.dao.DuplicateKeyException;
import org.springframework.jdbc.BadSqlGrammarException;
import org.springframework.jdbc.UncategorizedSQLException;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;
import org.springframework.transaction.NoTransactionException;
import org.springframework.transaction.SavepointManager;
import org.springframework.transaction.TransactionStatus;
import org.springframework.transaction.interceptor.TransactionAspectSupport;

import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Optional;
import java.util.Set;
import java.util.WeakHashMap;
import java.util.stream.Collectors;

/**
 * 基于数据库的分布式锁服务
 *
 */
@Slf4j
@Component
public class SQLLockService extends ServiceImpl<LockMapper, LockEntity> implements SmartInitializingSingleton {
    /**
     * 默认持有锁的时间1小时（适配上传agent包等耗时操作）
     */
    private static final int DEFAULT_LOCK_TIME = 1;

    /**
     * 分批处理时每批次的大小
     */
    private static final int BATCH_SIZE = 100;

    /**
     * 服务名称
     */
    private static final String SERVICE_NAME = "protectmanager-system-base";

    /**
     * 锁名称注册
     */
    private final Set<String> lockRecords = Collections.synchronizedSet(Collections.newSetFromMap(new WeakHashMap<>()));

    private final LockMapper lockMapper;

    @Autowired
    private InfrastructureRestApi infraRestApi;

    @Value("${NODE_NAME}")
    private String nodeName;

    /**
     * 构造方法
     *
     * @param lockMapper lockMapper
     */
    public SQLLockService(LockMapper lockMapper) {
        this.lockMapper = lockMapper;
    }

    /**
     * 每5分钟检查锁的占用情况，owner的节点不存在时释放锁
     */
    @Scheduled(cron = "0 0/5 * * * ?")
    public void unlockNotExistsLock() {
        log.info("start check lock which owner hostname not exist.");
        InfraResponseWithError<List<NodeDetail>> infraNodeInfos = infraRestApi.getInfraNodeInfo();
        InfraResponseWithError<List<NodePodInfo>> systemBaseNodeInfos = infraRestApi
                .getInfraPodInfo(StatefulsetConstants.PMSYSTEMBASE);
        if (!VerifyUtil.isEmpty(infraNodeInfos.getData()) && infraNodeInfos.getData().size() > 0) {
            List<String> allNodes = infraNodeInfos.getData().stream().map(NodeDetail::getHostName)
                    .collect(Collectors.toList());
            List<String> pmNodes = systemBaseNodeInfos.getData().stream().map(NodePodInfo::getNodeName)
                    .collect(Collectors.toList());
            allNodes.removeAll(pmNodes);
            for (String hostName : allNodes) {
                String owner = BackupClusterConfigUtil.getBackupClusterEsn() + "_" + SERVICE_NAME + "_" + hostName;
                LambdaUpdateWrapper<LockEntity> wrapper = new LambdaUpdateWrapper<LockEntity>()
                        .set(LockEntity::getUnlockTime, new Date()).eq(LockEntity::getOwner, owner);
                this.update(wrapper);
            }
        }
    }

    /**
     * 释放锁列表
     *
     * @param keyList 锁key列表
     */
    public void batchUnlockSqlLock(List<String> keyList) {
        String owner = ownerName();
        for (int i = 0; i < keyList.size(); i += BATCH_SIZE) {
            List<String> keys = keyList.subList(i, Math.min(i + BATCH_SIZE, keyList.size()));
            LambdaUpdateWrapper<LockEntity> wrapper = new LambdaUpdateWrapper<LockEntity>()
                    .set(LockEntity::getUnlockTime, new Date())
                    .in(LockEntity::getKey, keys)
                    .eq(LockEntity::getOwner, owner);
            this.update(wrapper);
        }
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
        LambdaUpdateWrapper<LockEntity> wrapper = new LambdaUpdateWrapper<LockEntity>().set(
                LockEntity::getUnlockTime, new Date()).eq(LockEntity::getKey, key);
        this.update(wrapper);
    }

    /**
     * 插入一条锁记录
     *
     * @param key 锁的key
     * @return 是否插入成功
     */
    public boolean insertRecord(String key) {
        Optional<TransactionStatus> statusOptional = getTransactionStatus();
        Optional<Object> savePointOptional = statusOptional.map(SavepointManager::createSavepoint);
        try {
            LockEntity entity = buildDefaultLockEntity(key);
            QueryWrapper<LockEntity> wrapper = new QueryWrapper<>();
            wrapper.eq("key", key);
            int count = lockMapper.selectCount(wrapper).intValue();
            if (count > 0) {
                return updateRecord(key);
            } else {
                return lockMapper.insert(entity) > 0;
            }
        } catch (DuplicateKeyException | CannotSerializeTransactionException e) {
            savePointOptional.ifPresent(savePoint -> statusOptional.get().rollbackToSavepoint(savePoint));
            return false;
        } catch (DataIntegrityViolationException | BadSqlGrammarException | UncategorizedSQLException e) {
            log.warn("Unexpected exception", ExceptionUtil.getErrorMessage(e));
            savePointOptional.ifPresent(savePoint -> statusOptional.get().rollbackToSavepoint(savePoint));
            return false;
        }
    }

    private Optional<TransactionStatus> getTransactionStatus() {
        TransactionStatus transactionStatus;
        try {
            transactionStatus = TransactionAspectSupport.currentTransactionStatus();
        } catch (NoTransactionException e) {
            transactionStatus = null;
        }
        return Optional.ofNullable(transactionStatus);
    }

    /**
     * 更新锁记录
     *
     * @param key 锁name
     * @return 是否更新成功
     */
    public boolean updateRecord(String key) {
        Date now = new Date();
        Date unLockTime = DateUtils.addHours(now, DEFAULT_LOCK_TIME);
        LambdaUpdateWrapper<LockEntity> updateWrapper = new LambdaUpdateWrapper<LockEntity>()
                .set(LockEntity::getLockTime, now).set(LockEntity::getUnlockTime, unLockTime)
                .set(LockEntity::getOwner, ownerName()).eq(LockEntity::getKey, key).le(LockEntity::getUnlockTime, now);
        return this.update(updateWrapper);
    }

    private LockEntity buildDefaultLockEntity(String key) {
        Date now = new Date();
        Date date = DateUtils.addHours(now, DEFAULT_LOCK_TIME);
        return LockEntity.builder()
            .id(UUIDGenerator.getUUID())
            .lockTime(now)
            .unlockTime(date)
            .key(key)
            .owner(ownerName())
            .build();
    }

    private String ownerName() {
        return BackupClusterConfigUtil.getBackupClusterEsn() + "_" + SERVICE_NAME + "_" + nodeName;
    }

    @Override
    public void afterSingletonsInstantiated() {
        lockMapper.unlockAll(new Date(), ownerName());
    }
}
