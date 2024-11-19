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
package openbackup.system.base.task;

import ch.qos.logback.classic.Level;
import ch.qos.logback.classic.Logger;
import ch.qos.logback.classic.LoggerContext;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.utils.ExceptionUtil;

import org.apache.commons.io.FileUtils;
import org.slf4j.ILoggerFactory;
import org.slf4j.LoggerFactory;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.io.File;
import java.io.IOException;
import java.util.Locale;

/**
 * The ChangeLogLevelTask
 *
 */
@EnableScheduling
@Component
@Slf4j
public class ChangeLogLevelTask {
    private static final String DEBUG = "DEBUG";

    private static final String INFO = "INFO";

    private static final String WARN = "WARN";

    private static final String ERROR = "ERROR";

    private static final long THIRTY_SECONDS = 30 * 1000;

    private static final String LOG_LEVEL_FILE_PATH = "/opt/config/loglevel";

    /**
     * 定时任务，检查configmap里的日志级别是否改动，每30s启动
     */
    @Scheduled(fixedRate = THIRTY_SECONDS)
    public void task() {
        String logLevel;
        File logLevelFile = new File(LOG_LEVEL_FILE_PATH);
        if (!logLevelFile.exists()) {
            log.error("Log level file does not exits, logLevelFile: {}.", logLevelFile.toPath());
            return;
        }
        try {
            logLevel = FileUtils.readFileToString(logLevelFile);
            final ILoggerFactory iLoggerFactory = LoggerFactory.getILoggerFactory();
            if (!(iLoggerFactory instanceof LoggerContext)) {
                return;
            }
            LoggerContext loggerContext = (LoggerContext) iLoggerFactory;
            Logger logger = loggerContext.getLogger("root");
            logLevel = logLevel.toUpperCase(Locale.ENGLISH);
            if (!isLevelValid(logLevel)) {
                log.error("log level parameter invalid: {}.", logLevel);
                return;
            }
            if (DEBUG.equals(logLevel) && !Level.DEBUG.equals(logger.getLevel())) {
                logger.setLevel(Level.DEBUG);
                log.debug("Set log level successfully: {}.", logLevel);
                return;
            }
            if (INFO.equals(logLevel) && !Level.INFO.equals(logger.getLevel())) {
                logger.setLevel(Level.INFO);
                log.info("Set log level successfully: {}.", logLevel);
                return;
            }
            if (WARN.equals(logLevel) && !Level.WARN.equals(logger.getLevel())) {
                logger.setLevel(Level.WARN);
                log.warn("Set log level successfully: {}.", logLevel);
                return;
            }
            if (ERROR.equals(logLevel) && !Level.ERROR.equals(logger.getLevel())) {
                logger.setLevel(Level.ERROR);
                log.error("Set log level successfully: {}.", logLevel);
            }
        } catch (IOException e) {
            log.error("Get log level failed", ExceptionUtil.getErrorMessage(e));
        }
    }

    private boolean isLevelValid(String level) {
        return DEBUG.equals(level) || INFO.equals(level) || WARN.equals(level) || ERROR.equals(level);
    }
}
