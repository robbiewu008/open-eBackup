/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.task;

import org.springframework.context.ApplicationEvent;

/**
 * 初始化完成Event
 *
 * @author w30042425
 * @since 2023-08-16
 */
public class InitializedEvent extends ApplicationEvent {
    public InitializedEvent(Object source) {
        super(source);
    }
}
