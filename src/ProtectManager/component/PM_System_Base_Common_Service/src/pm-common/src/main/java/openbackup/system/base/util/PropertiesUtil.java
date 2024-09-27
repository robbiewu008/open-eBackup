/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.StringUtils;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.util.Properties;

/**
 * 配置文件操作相关工具类
 *
 * @author w00607005
 * @since 2023-05-16
 */
@Slf4j
public class PropertiesUtil {
    /**
     * 获取配置文件转化为Properties
     *
     * @param filePath 配置文件绝对路径
     * @return Properties
     */
    public static Properties getProperties(String filePath) {
        Properties properties = new Properties();
        try (FileInputStream inputStream = new FileInputStream(filePath)) {
            properties.load(inputStream);
        } catch (IOException e) {
            log.error("read file error");
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Read file io ex");
        }
        return properties;
    }

    /**
     * 获取文件内容转化为string
     *
     * @param filePath 文件路径
     * @return 字符串
     */
    public static String fileToString(String filePath) {
        String str = StringUtils.EMPTY;
        try {
            str = FileUtils.readFileToString(new File(filePath), StandardCharsets.UTF_8);
        } catch (IOException e) {
            log.debug("file to string occurred io exception, {}", e.getMessage());
        }
        return str;
    }

    /**
     * 根据key查询configMap中的value
     *
     * @param mountPath 挂载路径
     * @param fileName 挂载文件名
     * @param key 键值
     * @return value值
     */
    public static String getConfValue(String mountPath, String fileName, String key) {
        String filePath = mountPath + File.separator + fileName;
        Properties properties = getProperties(filePath);
        return properties.getProperty(key);
    }
}
