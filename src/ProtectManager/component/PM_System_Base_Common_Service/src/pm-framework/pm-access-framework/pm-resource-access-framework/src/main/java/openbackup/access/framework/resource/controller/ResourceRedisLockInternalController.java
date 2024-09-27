/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.access.framework.resource.controller;

import openbackup.access.framework.resource.service.ResourceRedisLockService;
import openbackup.system.base.sdk.resource.model.ResourceLockRequest;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import javax.validation.Valid;

/**
 * 资源Redis冗余锁内部接口
 *
 * @author w30044259
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-08-04
 */
@Slf4j
@RestController
@RequestMapping("/v1/internal/resource/redis")
@Validated
public class ResourceRedisLockInternalController {
    @Autowired
    private ResourceRedisLockService resourceRedisLockService;

    /**
     * Redis 加锁，并同步资源锁
     *
     * @param request request
     * @return 加锁结果
     */
    @ExterAttack
    @PutMapping("/lock")
    public boolean lock(@RequestBody @Valid ResourceLockRequest request) {
        log.info("start redis resource lock ,lock_id :{}", request.getLockId());
        return resourceRedisLockService.lockAndMultiClusterSync(request.getLockId(), request.getResources());
    }

    /**
     * Redis 解锁，并同步资源锁
     *
     * @param request request
     * @return 解锁结果
     */
    @ExterAttack
    @DeleteMapping("/lock")
    public boolean unlock(@RequestBody @Valid ResourceLockRequest request) {
        log.info("start redis resource unlock ,lock_id :{}", request.getLockId());
        return resourceRedisLockService.unlockAndMultiClusterSync(request.getLockId());
    }
}
