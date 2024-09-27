package openbackup.system.base.task;

import ch.qos.logback.classic.Level;
import ch.qos.logback.classic.Logger;
import ch.qos.logback.classic.LoggerContext;
import openbackup.system.base.task.ChangeLogLevelTask;

import org.apache.commons.io.FileUtils;
import org.junit.*;
import org.mockito.InjectMocks;
import org.mockito.MockitoAnnotations;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.IOException;


public class ChangeLogLevelTaskTest {

    @InjectMocks
    private ChangeLogLevelTask changeLogLevelTask;

    @BeforeClass
    public static void initTest() {
        // 创建loglevel
        File loglevelFile = new File("src/test/task/loglevel");
        try {
            if (!loglevelFile.getParentFile().exists()) {
                loglevelFile.getParentFile().mkdirs();
            }
            if (!loglevelFile.exists()) {
                loglevelFile.createNewFile();
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    @AfterClass
    public static void removeEnvironment() {
        try {
            FileUtils.deleteDirectory(new File("src/test/task/"));
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
    }

    @Test
    public void change_log_level_to_debug_success() {
        File loglevelFile = new File("src/test/task/loglevel");
        try {
            FileUtils.writeStringToFile(loglevelFile, "DEBUG");
        } catch (IOException e) {
            e.printStackTrace();
        }
        LoggerContext loggerContext = (LoggerContext) LoggerFactory.getILoggerFactory();
        Logger logger = loggerContext.getLogger("root");
        logger.setLevel(Level.INFO);
        changeLogLevelTask.task();
        Assert.assertEquals(logger.getLevel(), Level.INFO);
    }

    @Test
    public void change_log_level_to_info_success() {
        File loglevelFile = new File("src/test/task/loglevel");
        try {
            FileUtils.writeStringToFile(loglevelFile, "INFO");
        } catch (IOException e) {
            e.printStackTrace();
        }
        LoggerContext loggerContext = (LoggerContext) LoggerFactory.getILoggerFactory();
        Logger logger = loggerContext.getLogger("root");
        logger.setLevel(Level.INFO);
        changeLogLevelTask.task();
        Assert.assertEquals(logger.getLevel(), Level.INFO);
    }

    @Test
    public void change_log_level_to_warn_success() {
        File loglevelFile = new File("src/test/task/loglevel");
        try {
            FileUtils.writeStringToFile(loglevelFile, "WARN");
        } catch (IOException e) {
            e.printStackTrace();
        }
        LoggerContext loggerContext = (LoggerContext) LoggerFactory.getILoggerFactory();
        Logger logger = loggerContext.getLogger("root");
        logger.setLevel(Level.WARN);
        changeLogLevelTask.task();
        Assert.assertEquals(logger.getLevel(), Level.WARN);
    }

    @Test
    public void change_log_level_to_error_success() {
        File loglevelFile = new File("src/test/task/loglevel");
        try {
            FileUtils.writeStringToFile(loglevelFile, "ERROR");
        } catch (IOException e) {
            e.printStackTrace();
        }
        LoggerContext loggerContext = (LoggerContext) LoggerFactory.getILoggerFactory();
        Logger logger = loggerContext.getLogger("root");
        logger.setLevel(Level.ERROR);
        changeLogLevelTask.task();
        Assert.assertEquals(logger.getLevel(), Level.ERROR);
    }

    @Test
    public void change_log_level_to_error_fail() {
        File loglevelFile = new File("src/test/task/loglevel");
        try {
            FileUtils.writeStringToFile(loglevelFile, "ERRORS");
        } catch (IOException e) {
            e.printStackTrace();
        }
        LoggerContext loggerContext = (LoggerContext) LoggerFactory.getILoggerFactory();
        Logger logger = loggerContext.getLogger("root");
        logger.setLevel(Level.INFO);
        changeLogLevelTask.task();
        Assert.assertNotEquals(logger.getLevel(), Level.ERROR);
    }
}
