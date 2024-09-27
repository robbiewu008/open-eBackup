package openbackup.system.base.common.utils;

import openbackup.system.base.config.SystemConfig;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.text.Normalizer;
import java.text.Normalizer.Form;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;

/**
 * lego.properties文件配置
 *
 * @author l90005176
 * @version [OceanStor BCManager V200R001C00, 2016年3月21日]
 * @since 2019-11-01
 */
public class LegoConfig {
    private static final Logger logger = LoggerFactory.getLogger(LegoConfig.class);

    // 默认开启证书告警开关
    private static final String CERT_ALARM_SWITCH_OFF = "off";

    // 证书告警开关，在LegoRuntime/conf/lego.properties中配置
    private static final String CERT_ALARM_SWITCH = "CertificateAlarmSwith";

    /**
     * 通过外部配置获得的参数配置
     */
    private static final String SYSTEM_PROPERTIES = "conf/lego.properties";

    private static final String NODE_CONFIG = "conf/node.properties";

    private static final Map<String, LegoConfig> INSTANCES = new HashMap<>();

    private static final long GET_REGULAR = 10 * 60L;

    private static final long TEN_THOUSAND_MILLISECONDS = 1000L;

    static {
        Thread regularGetLegoConfigThread =
                new Thread() {
                    @Override
                    public void run() {
                        setName("regularGetLegoConfigThread");
                        while (true) {
                            try {
                                sleep(GET_REGULAR * TEN_THOUSAND_MILLISECONDS);
                                updateInstance();
                            } catch (Exception e) {
                                logger.error("Failed to get regular legoConfig.", ExceptionUtil.getErrorMessage(e));
                            }
                        }
                    }
                };
        regularGetLegoConfigThread.setUncaughtExceptionHandler(
                new Thread.UncaughtExceptionHandler() {
                    /**
                     * uncaughtException
                     *
                     * @param t1 t1
                     * @param e1 e1
                     */
                    public void uncaughtException(Thread t1, Throwable e1) {
                        logger.info("regularGetLegoConfigThread uncaughtException");
                    }
                });
        regularGetLegoConfigThread.setName("regularGetLegoConfigThread");
        regularGetLegoConfigThread.start();
        logger.info("regular get legoConfig thread already start");
    }

    private final String path;

    private Properties properties;

    private LegoConfig(String path) {
        this.path = path;
        readConfigProperties();
    }

    /**
     * 获取lego.properties配置的对象
     *
     * @return LegoConfig
     */
    public static LegoConfig getInstance() {
        return getInstance(SYSTEM_PROPERTIES);
    }

    /**
     * 读取配置文件信息
     */
    @ExterAttack
    private void readConfigProperties() {
        File file = new File(path);
        if (file.exists()) {
            FileInputStream fileInputStream = null;
            InputStreamReader inputReader = null;
            try {
                fileInputStream = new FileInputStream(file);
                inputReader = new InputStreamReader(fileInputStream, Charset.defaultCharset());
                Properties props = new Properties();
                props.load(inputReader);
                setProperties(props);
            } catch (FileNotFoundException e) {
                logger.error("read lego.properties fail, not found.");
            } catch (IOException e) {
                logger.error("read lego.properties fail.", ExceptionUtil.getErrorMessage(e));
            } finally {
                CommonUtil.close(inputReader);
                CommonUtil.close(fileInputStream);
            }
        }
    }

    /**
     * 提供外部重新读取配置文件的方法
     */
    public synchronized void reloadConfigProperties() {
        readConfigProperties();
    }

    /**
     * 获取配置实例
     *
     * @param path 配置路径
     * @return 配置实例
     */
    public static synchronized LegoConfig getInstance(String path) {
        LegoConfig config = INSTANCES.get(path);
        if (config == null) {
            config = new LegoConfig(path);
            INSTANCES.put(path, config);
        }
        return config;
    }

    /**
     * 获取node.properties配置
     *
     * @return node.properties配置
     */
    public static LegoConfig getNodeConfig() {
        return getInstance(NODE_CONFIG);
    }

    /**
     * 更新conf/lego.properties配置信息
     */
    public static synchronized void updateInstance() {
        LegoConfig config = INSTANCES.get(SYSTEM_PROPERTIES);
        if (config != null) {
            config.reloadConfigProperties();
            INSTANCES.put(SYSTEM_PROPERTIES, config);
        }
    }

    /**
     * getKeyValue
     *
     * @param key 键
     * @return String 值
     */
    public String getKeyValue(String key) {
        String value = "";
        if (properties != null) {
            value = properties.getProperty(key);
        }

        return value;
    }

    /**
     * 获取配置值，如果没有找到配置值，则返回默认值
     *
     * @param key key
     * @param defaultValue defaultValue
     * @return value
     */
    public String getKeyValue(String key, String defaultValue) {
        String value = getKeyValue(key);
        if (value != null) {
            value = Normalizer.normalize(value, Form.NFKC);
        } else {
            value = defaultValue;
        }
        return value;
    }

    /**
     * 获取配置值，如果没有找到配置值，或配置的值不为数字，则返回默认值
     *
     * @param key key
     * @param defaultValue default value
     * @return value
     */
    public long getNumber(String key, long defaultValue) {
        String value = getKeyValue(key, String.valueOf(defaultValue));
        logger.info("get value from config file with key, key: {}, value: {} ", key, value);
        value = value != null ? value : "";
        value = value.replaceAll("^\\s*|\\s*$", "");
        value = value.matches("^\\d+$") ? value : String.valueOf(defaultValue);
        long num;
        try {
            num = Long.parseLong(value);
        } catch (NumberFormatException e) {
            logger.error("number format error for " + key + ", will use default value: " + defaultValue);
            num = defaultValue;
        }
        return num;
    }

    /**
     * 设置properties
     *
     * @param properties 对properties进行赋值
     */
    private void setProperties(Properties properties) {
        this.properties = properties;
    }

    /**
     * isCertAlarmSwitch
     *
     * @return boolean isCertAlarmopend
     */
    public boolean isCertAlarmSwitch() {
        boolean isCertAlarmSwitch = true;
        String alarmSwitch = SystemConfig.getInstance().getConfig(CERT_ALARM_SWITCH);
        logger.debug("cert alarm switch: {}", alarmSwitch);
        if (CERT_ALARM_SWITCH_OFF.equalsIgnoreCase(alarmSwitch)) {
            isCertAlarmSwitch = false;
        }
        return isCertAlarmSwitch;
    }

    /**
     * Get the configuration value, return the default value if no configuration value is found
     * 获取配置值，如果没有找到配置值，则返回默认值
     *
     * @param key Configure item name
     * @param isTrue default value
     * @return 配置value
     */
    public Boolean getBoolean(String key, Boolean isTrue) {
        String value = getKeyValue(key, String.valueOf(isTrue));
        if (!VerifyUtil.isEmpty(value)) {
            return Boolean.parseBoolean(value);
        }
        return isTrue;
    }
}
