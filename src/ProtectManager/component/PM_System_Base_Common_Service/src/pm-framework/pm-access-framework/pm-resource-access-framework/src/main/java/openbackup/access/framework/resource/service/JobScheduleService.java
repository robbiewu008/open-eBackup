/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.access.framework.resource.service;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.schedule.ScheduleRestApi;

import org.springframework.stereotype.Component;

import java.util.Optional;
import java.util.function.Consumer;

/**
 * Job Service
 *
 * @author l00272247
 * @since 2021-12-08
 */
@Component
public class JobScheduleService {
    private final ScheduleRestApi scheduleRestApi;

    /**
     * constructor
     *
     * @param scheduleRestApi scheduleRestApi
     */
    public JobScheduleService(ScheduleRestApi scheduleRestApi) {
        this.scheduleRestApi = scheduleRestApi;
    }

    /**
     * create job schedule without params
     *
     * @param type job type
     * @param resource resource
     * @param userId user id
     * @param initializations additional initializations
     */
    @SafeVarargs
    public final void createJobSchedule(String type, ProtectedResource resource, String userId,
        Consumer<JSONObject>... initializations) {
        createJobSchedule(type, resource, userId, null, initializations);
    }

    /**
     * create job schedule
     *
     * @param type job type.格式：小写加下划线
     * @param resource resource
     * @param userId user id
     * @param params job params
     * @param initializations additional initializations
     */
    @SafeVarargs
    public final void createJobSchedule(String type, ProtectedResource resource, String userId, JSONObject params,
        Consumer<JSONObject>... initializations) {
        JSONObject task = new JSONObject().set("type", "job_type_" + type)
            .set("sourceId", resource.getUuid())
            .set("sourceName", resource.getName())
            .set("sourceType", resource.getType())
            .set("sourceSubType", resource.getSubType())
            .set("sourceLocation", resource.getPath())
            .set("userId", Optional.ofNullable(userId).orElse(resource.getUserId()));
        for (Consumer<JSONObject> initialization : initializations) {
            initialization.accept(task);
        }
        scheduleRestApi.createImmediateSchedule("job_schedule_" + type, params, task);
    }
}
