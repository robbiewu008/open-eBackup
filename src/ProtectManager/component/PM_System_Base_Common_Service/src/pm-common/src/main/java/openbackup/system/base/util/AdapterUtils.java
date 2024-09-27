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
package openbackup.system.base.util;

import openbackup.system.base.common.utils.VerifyUtil;

import com.google.common.collect.ImmutableMap;

import lombok.extern.slf4j.Slf4j;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Enumeration;
import java.util.List;
import java.util.Locale;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import java.util.stream.Collectors;

/**
 * AdapterUtils
 *
 * @author m90005650
 * @version [BCManager eReplication V200R001C10, 2016年12月26日]
 * @since 2019-10-31
 */
@Slf4j
public class AdapterUtils {
    private static final String RESOURCE_FILE_ERROR_MESSAGE = "parse classpath resource file failed.";

    private static final String SLASH = "/";

    private static final int FOUR = 4;

    private static final int SIX = 6;

    private static final ImmutableMap<String, String> DEVICE_MAP = ImmutableMap.of("CyberEnginePacific",
        "OceanStor Pacific", "CyberEngineDoradoV6", "OceanStor Dorado", "CyberEngineOceanProtect", "OceanProtect");

    private static final String ALARM_ENGLISH_I18N_CONFIG_FILE = "jar:file:/app/app.jar!/BOOT-INF/classes"
        + "/conf/alarmI18nE/";

    private static final String ALARM_CHINESE_I18N_CONFIG_FILE = "jar:file:/app/app.jar!/BOOT-INF/classes"
        + "/conf/alarmI18nZ/";

    private AdapterUtils() {
    }

    /**
     * 获取Class Loader
     *
     * @param className className
     * @return ClassLoader loader
     */
    public static ClassLoader getClassLoader(Class className) {
        ClassLoader classLoader = null;
        if (className != null) {
            classLoader = className.getClassLoader();
        }
        if (classLoader == null) {
            classLoader = AdapterUtils.class.getClassLoader();
        }
        if (classLoader == null) {
            classLoader = AdapterUtils.class.getClassLoader();
        }
        return classLoader;
    }

    /**
     * 获取一个classpath下符合通配符的所有资源文件
     * 此方法有限制。详见实现调用
     *
     * @param packageFileName 必须是
     * “xxx/xxx/xxx/*.”后缀名的形式，内部使用classloader实现，前面不能有"/"
     * @return List URL [返回类型说明]
     */
    public static List<URL> getAllClassPathEntries(String packageFileName) {
        int index = packageFileName.lastIndexOf(SLASH);
        String classpath = "";
        String fileName = "";
        if (index >= 0) {
            classpath = packageFileName.substring(0, index);
            fileName = packageFileName.substring(index + 1);
        }
        return getAllClassPathEntries(classpath, fileName);
    }

    /**
     * 获取fault文件定义
     *
     * @return list
     */
    public static List<URL> getAllClassPathAlarmDefineUrls() {
        List<URL> listReturn = new ArrayList<>();
        try {
            File workDir = new File("");
            List<File> fileList = getFiles(
                workDir.getCanonicalPath() + File.separator + "conf" + File.separator + "fault" + File.separator);
            for (File file : fileList) {
                URL url = file.toURL();
                listReturn.add(url);
            }
        } catch (FileNotFoundException e) {
            log.error("wrong with read file, can not open file");
        } catch (IOException e1) {
            log.error("wrong with read file");
        }
        return listReturn;
    }

    /**
     * 获取告警国际化文件定义 英文
     *
     * @return list
     */
    public static List<URL> getAllClassPathEntriesAlarmEn() {
        return getI18nConfigFile(ALARM_ENGLISH_I18N_CONFIG_FILE + "AlarmCommonEn.json");
    }

    /**
     * 获取dorado底座告警国际化文件定义 英文
     *
     * @return list
     */
    public static List<URL> getDoradoAlarmClassPathEntriesAlarmEn() {
        return getI18nConfigFile(ALARM_ENGLISH_I18N_CONFIG_FILE + "DoradoAlarmEn.json");
    }

    /**
     * 获取dorado底座告警国际化文件定义 中文
     *
     * @return list
     */
    public static List<URL> getDoradoAlarmClassPathEntriesAlarmZn() {
        return getI18nConfigFile(ALARM_CHINESE_I18N_CONFIG_FILE + "DoradoAlarmZn.json");
    }

    /**
     * 获取Pacific底座告警国际化文件定义 英文
     *
     * @return list
     */
    public static List<URL> getPacificAlarmClassPathEntriesAlarmEn() {
        return getI18nConfigFile(ALARM_ENGLISH_I18N_CONFIG_FILE + "PacificAlarmEn.json");
    }

    /**
     * 获取Pacific底座告警国际化文件定义 中文
     *
     * @return list
     */
    public static List<URL> getPacificAlarmClassPathEntriesAlarmZn() {
        return getI18nConfigFile(ALARM_CHINESE_I18N_CONFIG_FILE + "PacificAlarmZn.json");
    }

    /**
     * 获取告警对象国际化文件定义 英文
     *
     * @return list
     */
    public static List<URL> getAllOperationTargetClassPathEntriesAlarmEn() {
        return getI18nConfigFile(ALARM_ENGLISH_I18N_CONFIG_FILE + "OperationTargetEn.json");
    }

    /**
     * 获取爱数告警对象国际化文件定义 英文
     *
     * @return list
     */
    public static List<URL> getAnyBackUpClassPathEntriesAlarmEn() {
        return getI18nConfigFile(ALARM_ENGLISH_I18N_CONFIG_FILE + "AbAlarmEn.json");
    }

    /**
     * 获取告警国际化文件定义 中文
     *
     * @return list
     */
    public static List<URL> getAllClassPathEntriesAlarmZh() {
        return getI18nConfigFile(ALARM_CHINESE_I18N_CONFIG_FILE + "AlarmCommonZh.json");
    }

    /**
     * 获取告警参数二次国际化信息 英文
     *
     * @return list
     */
    public static List<URL> getSecondaryParamInternalEn() {
        return getI18nConfigFile(ALARM_ENGLISH_I18N_CONFIG_FILE + "SecondaryParamEn.json");
    }

    /**
     * 获取告警参数二次国际化信息 中文
     *
     * @return list
     */
    public static List<URL> getSecondaryParamInternalZh() {
        return getI18nConfigFile(ALARM_CHINESE_I18N_CONFIG_FILE + "SecondaryParamZn.json");
    }

    /**
     * 获取告警对象国际化文件定义 中文
     *
     * @return list
     */
    public static List<URL> getAllOperationTargetClassPathEntriesAlarmZh() {
        return getI18nConfigFile(ALARM_CHINESE_I18N_CONFIG_FILE + "OperationTargetZn.json");
    }

    /**
     * 获取爱数告警对象国际化文件定义 中文
     *
     * @return list
     */
    public static List<URL> getAnyBackUpClassPathEntriesAlarmZh() {
        return getI18nConfigFile(ALARM_CHINESE_I18N_CONFIG_FILE + "AbAlarmZn.json");
    }

    /**
     * 获取explore label中文
     *
     * @return list
     */
    public static List<URL> getLabelExploreZh() {
        return getI18nConfigFile(ALARM_CHINESE_I18N_CONFIG_FILE + "explore.json");
    }

    /**
     * 获取common label中文
     *
     * @return list
     */
    public static List<URL> getLabelCommonZh() {
        return getI18nConfigFile(ALARM_CHINESE_I18N_CONFIG_FILE + "common.json");
    }

    /**
     * 获取protection label中文
     *
     * @return list
     */
    public static List<URL> getLabelProtectionZh() {
        return getI18nConfigFile(ALARM_CHINESE_I18N_CONFIG_FILE + "protection.json");
    }

    /**
     * 获取insight label中文
     *
     * @return list
     */
    public static List<URL> getLabelInsightZh() {
        return getI18nConfigFile(ALARM_CHINESE_I18N_CONFIG_FILE + "insight.json");
    }

    /**
     * 获取params label中文
     *
     * @return list
     */
    public static List<URL> getLabelParamsZh() {
        return getI18nConfigFile(ALARM_CHINESE_I18N_CONFIG_FILE + "params.json");
    }

    /**
     * 获取search label中文
     *
     * @return list
     */
    public static List<URL> getLabelSearchZh() {
        return getI18nConfigFile(ALARM_CHINESE_I18N_CONFIG_FILE + "search.json");
    }

    /**
     * 获取system label中文
     *
     * @return list
     */
    public static List<URL> getLabelSystemZh() {
        return getI18nConfigFile(ALARM_CHINESE_I18N_CONFIG_FILE + "system.json");
    }

    /**
     * 获取explore label英文
     *
     * @return list
     */
    public static List<URL> getLabelExploreEn() {
        return getI18nConfigFile(ALARM_ENGLISH_I18N_CONFIG_FILE + "explore.json");
    }

    /**
     * 获取common label英文
     *
     * @return list
     */
    public static List<URL> getLabelCommonEn() {
        return getI18nConfigFile(ALARM_ENGLISH_I18N_CONFIG_FILE + "common.json");
    }

    /**
     * 获取protection label英文
     *
     * @return list
     */
    public static List<URL> getLabelProtectionEn() {
        return getI18nConfigFile(ALARM_ENGLISH_I18N_CONFIG_FILE + "protection.json");
    }

    /**
     * 获取insight label英文
     *
     * @return list
     */
    public static List<URL> getLabelInsightEn() {
        return getI18nConfigFile(ALARM_ENGLISH_I18N_CONFIG_FILE + "insight.json");
    }

    /**
     * 获取params label英文
     *
     * @return list
     */
    public static List<URL> getLabelParamsEn() {
        return getI18nConfigFile(ALARM_ENGLISH_I18N_CONFIG_FILE + "params.json");
    }

    /**
     * 获取search label英文
     *
     * @return list
     */
    public static List<URL> getLabelSearchEn() {
        return getI18nConfigFile(ALARM_ENGLISH_I18N_CONFIG_FILE + "search.json");
    }

    /**
     * 获取system label英文
     *
     * @return list
     */
    public static List<URL> getLabelSystemEn() {
        return getI18nConfigFile(ALARM_ENGLISH_I18N_CONFIG_FILE + "system.json");
    }

    private static List<URL> getI18nConfigFile(String path) {
        try {
            return Collections.singletonList(new URL(path));
        } catch (MalformedURLException e) {
            log.error("parse classpath resource file failed. Path: {}", path, e);
            return Collections.emptyList();
        }
    }

    /**
     * 目前此方法有2个限制：
     * 1. 只读取了一个classpath下的目录。
     * 所以必须保证传入的 classpath路径在系统中唯一。若系统中有2个相同的@param classpath。此方法可能导致不一样的结果
     * 2.classpath中的路径不包括jar包中的.
     *
     * @param classpath 某一个classpath
     * @param fileName 需为 *.properties类似的形式
     * @return List URL [返回类型说明]
     */
    private static List<URL> getAllClassPathEntries(String classpath, String fileName) {
        File classpathFile = null;
        String subFileName = fileName.substring(1);
        try {
            List<URL> list = new ArrayList<>();
            ClassLoader classLoader = AdapterUtils.getClassLoader(AdapterUtils.class);
            if (classLoader == null) {
                log.error("get AdapterUtil null.");
                return list;
            }
            Enumeration<URL> emnum = classLoader.getResources(classpath);
            while ((emnum != null) && emnum.hasMoreElements()) {
                URL url = emnum.nextElement();
                if (url == null) {
                    log.error("The url is null");
                    return Collections.emptyList();
                }
                String urlPath = url.getPath();
                if (urlPath.contains(".jar!/")) {
                    int index = urlPath.indexOf(".jar!/");
                    String jarPath = url.getPath().substring(0, index + FOUR);
                    getResourceFromJar(list, jarPath, subFileName);
                } else {
                    classpathFile = new File(url.toURI());
                    list.addAll(getSubUrl(classpathFile, subFileName));
                }
            }
            return list;
        } catch (URISyntaxException e) {
            log.error("read classpath directory failed.", e);
            return Collections.emptyList();
        } catch (IOException e) {
            log.error(RESOURCE_FILE_ERROR_MESSAGE, e);
            return Collections.emptyList();
        }
    }

    /**
     * 获取一个classpath下符合通配符的所有资源文件
     * 此方法有限制。详见实现调用
     *
     * @param cl classLoader
     * @param jarfileName jar文件路径
     * @param configName 配置文件名称
     * @return URL [返回类型说明]
     */
    public static URL getUrlResourceFromJar(ClassLoader cl, String jarfileName, String configName) {
        Enumeration<URL> urlsss;
        try {
            urlsss = cl.getResources(configName);
            while (urlsss.hasMoreElements()) {
                URL url = urlsss.nextElement();
                if (url.getPath().contains(jarfileName)) {
                    return url;
                }
            }
        } catch (IOException e) {
            log.error("read default.properties fail.jarfileName: {} , configName: {}", jarfileName, configName);
        }
        return null;
    }

    private static List<File> getFiles(String path) {
        File file = new File(path);
        if (!file.exists()) {
            return Collections.emptyList();
        }
        File[] tempLists = file.listFiles();
        if (VerifyUtil.isEmpty(tempLists)) {
            return Collections.emptyList();
        }
        return Arrays.stream(tempLists).filter(File::isFile).collect(Collectors.toList());
    }

    private static void getResourceFromJar(List<URL> list, String jarPath, String fileName) {
        JarFile jarFile = null;
        String subJarPath = jarPath;
        try {
            if (jarPath.startsWith("file:" + File.separator)) {
                subJarPath = jarPath.substring(SIX);
            }
            String osName = System.getProperty("os.name");
            if (osName == null || !osName.toLowerCase(Locale.ENGLISH).contains("windows")) {
                subJarPath = File.separator + subJarPath;
            }
            log.info("Jar Path:{}, fileName:{}", subJarPath, fileName);
            jarFile = new JarFile(subJarPath);

            String jarName = null;
            int index = jarPath.lastIndexOf(SLASH);
            if (index >= 0) {
                jarName = jarPath.substring(index);
            }
            if (jarName == null) {
                log.error("jarName is null.");
                return;
            }
            Enumeration<JarEntry> es = jarFile.entries();
            ClassLoader classLoader = getClassLoader(AdapterUtils.class);
            if (classLoader == null) {
                log.error("get adapterUtil class loader null.");
                return;
            }
            while (es.hasMoreElements()) {
                JarEntry jarEntry = es.nextElement();
                String resourceName = jarEntry.getName();
                if (resourceName.endsWith(fileName)) {
                    list.add(getUrlResourceFromJar(classLoader, jarName, resourceName));
                }
            }
        } catch (IOException e) {
            log.error("get reource error : ", e);
        } finally {
            if (jarFile != null) {
                try {
                    jarFile.close();
                } catch (IOException e) {
                    log.error("get reource error : ", e);
                }
            }
        }
    }

    private static List<URL> getSubUrl(File parentDir, String fileName) throws MalformedURLException {
        List<URL> results = new ArrayList<>();
        File[] files = parentDir.listFiles();
        if (files != null && files.length > 0) {
            for (File file : files) {
                if (file.isDirectory()) {
                    results.addAll(getSubUrl(file, fileName));
                    continue;
                }
                if (file.getName().endsWith(fileName)) {
                    results.add(file.toURI().toURL());
                    continue;
                }
                if (file.getName().endsWith(".jar")) {
                    getResourceFromJar(results, file.getName(), fileName);
                }
            }
        }
        return results;
    }

    /**
     * 安全一体机设备类型转换为GUI上对应的类型
     *
     * @param subType classLoader
     * @return GUI上对应的类型
     */
    public static String convertSubType(String subType) {
        return DEVICE_MAP.containsKey(subType) ? DEVICE_MAP.get(subType) : subType;
    }
}
