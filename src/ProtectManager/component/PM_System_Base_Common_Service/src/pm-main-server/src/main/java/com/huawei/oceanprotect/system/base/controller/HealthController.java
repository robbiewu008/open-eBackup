/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.controller;

import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.utils.DateFormatUtil;
import openbackup.system.base.pack.lock.zookeeper.zookeeper.impl.ZookeeperServiceImpl;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.permission.Permission;

import org.apache.zookeeper.CreateMode;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.Date;
import java.util.List;

/**
 * 健康检查的接口，用于k8s就绪探针
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.0.RC1]
 * @since 2021-02-03
 */
@RestController
public class HealthController {
    private static final String HEALTH_CHECK_KEY = "health_check_key";

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private InfrastructureRestApi infrastructureRestApi;

    @Autowired
    private KafkaTemplate kafkaTemplate;

    @Autowired
    private ZookeeperServiceImpl zookeeperService;

    /**
     * 健康检查接口
     *
     * @return 服务是否启动
     */
    @GetMapping("/internal/health")
    @ExterAttack
    public String health() {
        redissonClient.getBucket(HEALTH_CHECK_KEY).set(DateFormatUtil.format(Constants.SIMPLE_DATE_FORMAT, new Date()));
        return "ok";
    }

    /**
     * pod状态查询接口
     *
     * @return 服务是否启动
     */
    @GetMapping("/v1/inspect/service/status")
    @ExterAttack
    @Permission(roles = {Constants.Builtin.ROLE_SYS_ADMIN}, enableCheckAuth = false, checkRolePermission = true)
    public InfraResponseWithError<List<NodePodInfo>> podStatus() {
        return infrastructureRestApi.getInfraContainerInfo();
    }

    /**
     * zookeeper服务健康状态
     *
     * @return 服务是否启动
     */
    @GetMapping("/v1/internal/health/zk")
    @ExterAttack
    public String zkHealth() {
        zookeeperService.setString("/test", "test", CreateMode.PERSISTENT);
        return "ok";
    }

    /**
     * kafka服务健康状态
     *
     * @return 服务是否启动
     */
    @GetMapping("/v1/internal/health/kafka")
    @ExterAttack
    public String kafkaHealth() {
        kafkaTemplate.send("testTopic", "test");
        return "ok";
    }
}
