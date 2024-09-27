/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.taskflow;

import static jodd.util.ThreadUtil.sleep;

import openbackup.system.base.schedule.BaseTimedTask;

import java.util.ArrayList;
import java.util.List;

/**
 * 子任务基类
 *
 * @author w00607005
 * @since 2023-05-18
 */
public abstract class BaseTask {
    /**
     * 任务Id，自行定义，一般为子任务的执行顺序，同一个任务流中多个子任务不可以重复
     */
    private final String taskId;

    /**
     * 重试次数
     */
    private int retryCount;

    /**
     * 构造方法
     *
     * @param taskId 任务Id
     * @param retryCount 重试次数
     */
    public BaseTask(String taskId, int retryCount) {
        this.taskId = taskId;
        this.retryCount = retryCount;
    }

    /**
     * 获取任务ID
     *
     * @return 任务ID
     */
    public String getTaskId() {
        return taskId;
    }

    /**
     * 获取重试次数
     *
     * @return 重试次数
     */
    public int getRetryCount() {
        return retryCount;
    }

    /**
     * 主动更新重试次数
     *
     * @param retryCount 重试次数
     */
    public void updateRetryCount(int retryCount) {
        this.retryCount = retryCount;
    }

    /**
     * 任务执行逻辑
     *
     * @param context 任务流上下文
     * @param <T> 泛型
     * @return 任务流上下文
     */
    protected abstract <T> boolean execute(T context);

    /**
     * 任务回滚逻辑
     *
     * @param context 任务流上下文
     * @param <T> 泛型
     * @return 任务流上下文
     */
    protected abstract <T> boolean rollback(T context);

    /**
     * 定时等待，执行时间 = cycleNum * period，单位：ms
     *
     * @param timedTask 定时器
     * @param cycleNum 循环次数
     * @param period 线程阻塞时间
     */
    public void timedWait(BaseTimedTask timedTask, int cycleNum, long period) {
        int num = cycleNum;
        timedTask.start();
        // 一定要不要直接使用常量，因为重试也需要用
        while (num > 0) {
            num--;
            if (!timedTask.isRunning()) {
                break;
            }
            sleep(period);
        }
        timedTask.stop();
    }

    /**
     * 获取子任务的所有定时器
     *
     * @return 定时器列表
     */
    public List<BaseTimedTask> getTimedTasks() {
        return new ArrayList<>();
    }
}
