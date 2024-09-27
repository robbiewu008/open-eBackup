package openbackup.system.base.common.process;

import openbackup.system.base.common.thread.ThreadPoolTool;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.TimeUnit;

/**
 * 执行工具
 *
 * @author w00493811
 * @since 2021-08-10
 */
@Slf4j
public class ProcessUtil {
    /**
     * 默认等待时间（毫秒）
     */
    private static final long DEFAULT_WAIT_TIME = 1000L;

    /**
     * 错误ProcessResult 最大String长度，防止结果过长导致的占用CPU及内存，引发Full GC
     */
    private static final int MAX_LOG_ERROR_SIZE = 1000;

    /**
     * 默认构造函数
     */
    private ProcessUtil() {
    }

    /**
     * 执行命令（超时(分钟））
     *
     * @param command 命令
     * @param timeoutMinutes 超时(分钟）
     * @return 执行结果
     * @throws ProcessException 处理异常
     */
    public static ProcessResult executeInMinutes(List<String> command, long timeoutMinutes) throws ProcessException {
        return execute(command, timeoutMinutes, TimeUnit.MINUTES);
    }

    /**
     * 执行命令（超时(秒））
     *
     * @param command 命令
     * @param timeoutSeconds 超时(秒）
     * @return 执行结果
     * @throws ProcessException 处理异常
     */
    public static ProcessResult executeInSeconds(List<String> command, long timeoutSeconds) throws ProcessException {
        return execute(command, timeoutSeconds, TimeUnit.SECONDS);
    }

    /**
     * 执行命令,命令参数中有中文可以使用该方法,该方法只支持在unix/liunx下执行
     *
     * @param command 执行命令
     * @param args 命令参数
     * @param timeout 超时
     * @param timeUnit 时间单位
     * @return 执行结果
     * @throws ProcessException 处理异常
     */
    public static ProcessResult execute(String command, List<String> args, long timeout, TimeUnit timeUnit)
        throws ProcessException {
        ProcessBuilder processBuilder = new ProcessBuilder("xargs", "sudo", command);
        processBuilder.environment().put("LANG", "en_US.UTF-8");
        Process process = null;
        ProcessResult processResult = new ProcessResult();
        try {
            process = processBuilder.start();
            inputArgs(args, process);
            logProcessResult(process, processResult, Collections.singletonList(command), timeout, timeUnit);
        } catch (IOException exception) {
            throw new ProcessException(exception, "start execute(command=%s,timeout=%s,timeUnit=%s) failed", command,
                timeout, timeUnit);
        } catch (InterruptedException exception) {
            Thread.currentThread().interrupt();
            throw new ProcessException(exception, "start execute(command=%s,timeout=%s,timeUnit=%s) failed", command,
                timeout, timeUnit);
        } finally {
            if (process != null) {
                try {
                    Thread.sleep(DEFAULT_WAIT_TIME);
                } catch (InterruptedException exception) {
                    log.error("Wait Exception:", exception);
                    Thread.currentThread().interrupt();
                }

                // 最终毁灭这个线程
                process.destroy();
            }
        }
        return processResult;
    }

    private static void inputArgs(List<String> args, Process process) throws IOException {
        try (OutputStream outputStream = process.getOutputStream()) {
            if (CollectionUtils.isNotEmpty(args)) {
                for (String arg : args) {
                    outputStream.write(arg.getBytes(StandardCharsets.UTF_8));
                    outputStream.write(" ".getBytes(StandardCharsets.UTF_8));
                }
            }
        }
    }

    private static void logProcessResult(Process process, ProcessResult processResult, List<String> command,
        long timeout, TimeUnit timeUnit) throws ProcessException, InterruptedException {
        log.info("execute command:{}, timeout:{}, timeUnit:{}", command, timeout, timeUnit);
        log(process.getInputStream(), processResult, false);
        log(process.getErrorStream(), processResult, true);
        // 等待超时
        if (process.waitFor(timeout, timeUnit)) {
            // 返回结束值
            processResult.setExitCode(process.exitValue());
        } else {
            throw new ProcessException("execute(command=%s,timeout=%s,timeUnit=%s) timeout", command, timeout,
                timeUnit);
        }
    }

    /**
     * 执行命令
     *
     * @param command 命令
     * @param timeout 超时
     * @param timeUnit 时间单位
     * @return 执行结果
     * @throws ProcessException 处理异常
     */
    public static ProcessResult execute(List<String> command, long timeout, TimeUnit timeUnit) throws ProcessException {
        log.debug("execute command:{}, timeout:{}, timeUnit:{}", String.join(";", command), timeout, timeUnit);

        // 执行构造器
        ProcessBuilder processBuilder = new ProcessBuilder();
        processBuilder.command(command);

        // 处理
        Process process = null;
        ProcessResult processResult = new ProcessResult();
        try {
            // 开始
            process = processBuilder.start();
            logProcessResult(process, processResult, command, timeout, timeUnit);
        } catch (IOException exception) {
            throw new ProcessException(exception, "start execute(command=%s,timeout=%s,timeUnit=%s) failed",
                String.join(";", command), timeout, timeUnit);
        } catch (InterruptedException exception) {
            Thread.currentThread().interrupt();
            throw new ProcessException(exception, "execute(command=%s,timeout=%s,timeUnit=%s) interrupted",
                String.join(";", command), timeout, timeUnit);
        } finally {
            if (process != null) {
                try {
                    Thread.sleep(DEFAULT_WAIT_TIME);
                } catch (InterruptedException exception) {
                    log.error("Wait Exception:", exception);
                    Thread.currentThread().interrupt();
                }

                // 最终毁灭这个线程
                process.destroy();
            }
        }
        return processResult;
    }

    private static void log(InputStream inputStream, ProcessResult processResult, boolean isError) {
        // 线程池执行
        ThreadPoolTool.getPool().execute(() -> {
            try (BufferedReader bf = new BufferedReader(new InputStreamReader(inputStream, StandardCharsets.UTF_8))) {
                collectErrorOrOutput(bf, processResult, isError);
            } catch (IOException e) {
                log.error("ProcessUtil.log(inputStream:{},processResult:{},isError:{})", inputStream,
                    StringUtils.truncate(processResult.toString(), MAX_LOG_ERROR_SIZE), isError);
            }
        });
    }

    private static void collectErrorOrOutput(BufferedReader bufferedReader, ProcessResult processResult,
        boolean isError) throws IOException {
        String line;
        while ((line = bufferedReader.readLine()) != null) {
            if (isError) {
                processResult.addErrors(line);
            } else {
                processResult.addOutput(line);
            }
        }
    }
}
