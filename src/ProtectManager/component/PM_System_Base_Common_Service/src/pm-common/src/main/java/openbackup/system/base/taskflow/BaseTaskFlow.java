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
package openbackup.system.base.taskflow;

import static jodd.util.ThreadUtil.sleep;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.schedule.BaseTimedTask;
import openbackup.system.base.util.OrderNoUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Queue;

/**
 * 任务流
 *
 * @author w00607005
 * @since 2023-05-18
 */
@Slf4j
public abstract class BaseTaskFlow<T extends BaseContext> {
    /**
     * 0入度
     */
    private static final int IN_DEGREE_ZERO = 0;

    /**
     * 重试周期3分钟
     */
    private static final long RETRY_PERIOD = 3 * 60 * 1000L;

    /**
     * 任务依赖关系
     */
    private final Map<String, List<String>> graph = new HashMap<>();

    /**
     * 任务入度
     */
    private final Map<String, Integer> inDegree = new HashMap<>();

    /**
     * 任务列表
     */
    private final Map<String, BaseTask> taskMap = new HashMap<>();

    /**
     * 任务流所涉及定时器
     */
    private final List<BaseTimedTask> timedTasks = new ArrayList<>();

    /**
     * 任务流上下文
     */
    private final T context;

    /**
     * 构造方法
     *
     * @param context 任务流上下文
     */
    protected BaseTaskFlow(T context) {
        this.context = context;
    }

    /**
     * 添加任务列表
     *
     * @param clazz clazz
     * @param retryCount 重试次数
     * @return 任务id
     */
    public String addTask(Class<? extends BaseTask> clazz, int retryCount) {
        return addTask(clazz, null, retryCount);
    }

    /**
     * 添加任务列表
     *
     * @param clazz clazz
     * @param taskId 任务id
     * @param retryCount 重试次数
     * @return 任务id
     */
    public String addTask(Class<? extends BaseTask> clazz, String taskId, int retryCount) {
        try {
            Constructor<?> constructor = clazz.getDeclaredConstructor(String.class, int.class);
            String id = taskId;
            if (StringUtils.isEmpty(id)) {
                id = OrderNoUtil.getTaskNo();
            }
            Object obj = constructor.newInstance(id, retryCount);
            if (obj instanceof BaseTask) {
                BaseTask task = (BaseTask) obj;
                addTask(task);
                return id;
            } else {
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "create task error");
            }
        } catch (InvocationTargetException | InstantiationException | IllegalAccessException
            | NoSuchMethodException e) {
            log.error("add task error.", e);
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "create task error");
        }
    }

    /**
     * 添加任务列表
     *
     * @param task 子任务
     */
    public void addTask(BaseTask task) {
        String taskId = task.getTaskId();
        taskMap.put(taskId, task);
        graph.put(taskId, new ArrayList<>());
        inDegree.put(taskId, IsmNumberConstant.ZERO);
        timedTasks.addAll(task.getTimedTasks());
    }

    /**
     * 添加任务依赖关系（画有向边）
     *
     * @param idFrom 依赖任务id
     * @param idTo 被依赖任务id
     */
    public void addDependency(String idFrom, String idTo) {
        graph.get(idFrom).add(idTo);
        inDegree.put(idTo, inDegree.getOrDefault(idTo, IsmNumberConstant.ZERO) + IsmNumberConstant.ONE);
    }

    /**
     * 执行任务
     *
     * @return 任务流上下文
     */
    public boolean execute() {
        Queue<String> queue = new LinkedList<>();
        // 添加入度为0任务至队列中
        for (String taskId : taskMap.keySet()) {
            if (inDegree.get(taskId) == IN_DEGREE_ZERO) {
                queue.offer(taskId);
            }
        }

        while (!queue.isEmpty()) {
            String taskId = queue.poll();
            BaseTask task = taskMap.get(taskId);
            boolean isSuccess = false;
            try {
                // 执行任务
                isSuccess = task.execute(context);
                // 更新节点入度
                refreshInDegree(queue, taskId);
            } catch (Exception e) {
                // 重试失败，递归回滚
                if (!retry(task)) {
                    rollback(task);
                    stopTimedTask();
                    return false;
                }
            }
            // 子任务失败，重试回滚
            if (!isSuccess) {
                if (!retry(task)) {
                    rollback(task);
                    stopTimedTask();
                    return false;
                }
            }
        }
        stopTimedTask();
        return true;
    }

    /**
     * 获取job状态
     *
     * @return job状态
     */
    public int getJobStatus() {
        BaseContext baseContext = new BaseContext();
        if (context instanceof BaseContext) {
            baseContext = context;
        }
        return baseContext.getStatus();
    }

    private void refreshInDegree(Queue<String> queue, String taskId) {
        for (String nextTaskId : graph.get(taskId)) {
            // 被依赖任务入度减一
            inDegree.put(nextTaskId, inDegree.get(nextTaskId) - IsmNumberConstant.ONE);
            // 入度为0加入执行队列
            if (inDegree.get(nextTaskId) == IN_DEGREE_ZERO) {
                queue.offer(nextTaskId);
            }
        }
    }

    /**
     * 停用任务流中所有定时器，可以重复调用，建议如下场景使用：
     * 1、任务执行成功
     * 2、发生异常
     * 3、任务回退
     */
    public void stopTimedTask() {
        for (BaseTimedTask timedTask : timedTasks) {
            timedTask.stop();
        }
    }

    /**
     * 任务重试
     *
     * @param task 任务详情
     * @return 重试结果
     */
    public boolean retry(BaseTask task) {
        int retryCount = task.getRetryCount();
        // 重试
        while (retryCount > IsmNumberConstant.ZERO) {
            retryCount--;
            try {
                boolean isSuccess = task.execute(context);
                if (isSuccess) {
                    return true;
                }
            } catch (Exception ex) {
                // 重试次数用尽，执行失败
                if (retryCount == IsmNumberConstant.ZERO) {
                    return false;
                }
            }
            sleep(RETRY_PERIOD);
        }
        // 情形一：不重试，直接回滚
        // 情形二：重试次数耗尽
        return false;
    }

    /**
     * 任务回退，某个子任务回滚失败仍然继续向上回滚，自定义回滚方式请覆写该方法
     *
     * @param task 任务详情
     * @return 回退结果
     */
    public boolean rollback(BaseTask task) {
        boolean isSuccess = false;
        try {
            // 回退当前子任务
            isSuccess = task.rollback(context);
            // 递归回退上一步子任务
            for (String prevTaskId : graph.keySet()) {
                if (graph.get(prevTaskId).contains(task.getTaskId())) {
                    BaseTask preTask = taskMap.get(prevTaskId);
                    isSuccess = rollback(preTask);
                }
            }
        } catch (Exception e) {
            log.warn("rollback failed, happened in the base task flow", e);
        }

        return isSuccess;
    }
}
