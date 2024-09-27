package openbackup.system.base.schedule;

import java.util.Timer;
import java.util.TimerTask;

/**
 * 逻辑定时任务基类
 *
 * @author w00607005
 * @since 2023-05-17
 */
public abstract class BaseTimedTask {
    /**
     * Timer对象
     */
    private final Timer timer;

    /**
     * 延迟时间，单位：ms
     */
    private final long delay;

    /**
     * 间隔时间，单位：ms
     */
    private final long period;

    /**
     * 定时任务运行状态
     */
    private boolean isRunning = false;

    /**
     * 构造方法
     *
     * @param delay 延迟时间，单位：ms
     * @param period 间隔时间，单位：ms
     */
    public BaseTimedTask(long delay, long period) {
        this.timer = new Timer();
        this.delay = delay;
        this.period = period;
    }

    public boolean isRunning() {
        return isRunning;
    }

    /**
     * 启动定时器
     */
    public void start() {
        timer.schedule(new Task(), delay, period);
        isRunning = true;
    }

    /**
     * 取消定时器
     */
    public void stop() {
        timer.cancel();
        isRunning = false;
    }

    /**
     * 定时任务
     */
    protected abstract void runTask();

    private class Task extends TimerTask {
        @Override
        public void run() {
            runTask();
        }
    }
}
