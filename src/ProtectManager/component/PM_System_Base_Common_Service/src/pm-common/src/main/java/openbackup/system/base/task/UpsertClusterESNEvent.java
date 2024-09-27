/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.task;

import lombok.Getter;
import lombok.Setter;

import org.springframework.context.ApplicationEvent;

/**
 * 初始化ESN事件
 *
 * @author w30042425
 * @since 2023-09-26
 */
@Getter
@Setter
public class UpsertClusterESNEvent extends ApplicationEvent {
    private String esn;

    /**
     * 构造方法
     *
     * @param source source
     * @param esn esn值
     */
    public UpsertClusterESNEvent(Object source, String esn) {
        super(source);
        this.esn = esn;
    }
}
