/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.emeistor.console.contant;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

import java.util.HashMap;
import java.util.Map;

/**
 * 部署类型
 *
 * @author w00504341
 * @since 2023-01-05
 */
public enum DeployType {
    /**
     * A8000
     */
    A8000("a8000", "a8000"),

    /**
     * X8000
     */
    X8000("x8000", "d0"),

    /**
     * X6000
     */
    X6000("x6000", "d1"),

    /**
     * X3000
     */
    X3000("x3000", "d2"),

    /**
     * CLOUDBACKUP
     */
    CLOUDBACKUP("cloudbackup", "d3"),

    /**
     * HYPER_DETECT(主存只部署防勒索)
     */
    HYPER_DETECT("hyperdetect", "d4"),

    /**
     * 一体机
     */
    CYBER_ENGINE("cyberengine", "d5");

    private static final Map<String, DeployType> VALUE_ENUM_MAP = new HashMap<>(DeployType.values().length);

    static {
        for (DeployType deployType : DeployType.values()) {
            VALUE_ENUM_MAP.put(deployType.name, deployType);
            VALUE_ENUM_MAP.put(deployType.boardName, deployType);
        }
    }

    /**
     * 部署类型
     */
    private final String name;

    private final String boardName;

    /**
     * DeployType
     *
     * @param name 部署名
     * @param boardName 白牌化后名称
     */
    DeployType(String name, String boardName) {
        this.name = name;
        this.boardName = boardName;
    }

    /**
     * 根据部署名称获取部署类型，默认A8000
     *
     * @param deployTypeName 部署类型名称
     * @return 部署类型
     */
    @JsonCreator(mode = JsonCreator.Mode.DELEGATING)
    public static DeployType forValues(@JsonProperty("deployType") String deployTypeName) {
        return VALUE_ENUM_MAP.getOrDefault(deployTypeName, DeployType.A8000);
    }

    /**
     * 获取当前的部署类型
     *
     * @return 部署类型
     */
    public static DeployType getCurrentDeployType() {
        return DeployType.forValues(System.getenv("DEPLOY_TYPE"));
    }

    public String getName() {
        return name;
    }
}
