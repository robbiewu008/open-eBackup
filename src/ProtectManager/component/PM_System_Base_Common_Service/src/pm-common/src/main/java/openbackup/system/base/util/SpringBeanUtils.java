package openbackup.system.base.util;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.BeansException;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.stereotype.Component;

import java.util.Map;

/**
 * 获取SpringBean的工具类
 *
 * @author w00448845
 * @version [CDM Integrated machine]
 * @since 2019-11-05
 */
@Slf4j
@Component
public class SpringBeanUtils implements ApplicationContextAware {
    private static ApplicationContext applicationContext;

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
        if (SpringBeanUtils.applicationContext == null) {
            SpringBeanUtils.applicationContext = applicationContext;
        }
        log.info("ApplicationContext init success");
    }

    /**
     * 获取上下文
     *
     * @return ApplicationContext
     */
    public static ApplicationContext getApplicationContext() {
        return applicationContext;
    }

    /**
     * 获取bean
     *
     * @param name 名称
     * @return Object
     */
    public static Object getBean(String name) {
        return getApplicationContext().getBean(name);
    }

    /**
     * 获取bean
     *
     * @param clazz clazz
     * @param <T>   <T>
     * @return T
     */
    public static <T> T getBean(Class<T> clazz) {
        return getApplicationContext().getBean(clazz);
    }

    /**
     * 获取bean
     *
     * @param name  name
     * @param clazz clazz
     * @param <T>   <T>
     * @return T
     */
    public static <T> T getBean(String name, Class<T> clazz) {
        return getApplicationContext().getBean(name, clazz);
    }

    /**
     * 获取class的所有子类的bean
     *
     * @param clazz bean的class对象
     *
     * @return Map<String, T> 指定Class的所有Bean实例map，key为bean名称，value为bean实例
     */
    public static <T> Map<String, T> getBeansByClass(Class<T> clazz) {
        if (clazz == null) {
            throw new IllegalArgumentException("class is empty");
        }
        // 根据接口类型返回相应的所有bean
        return applicationContext.getBeansOfType(clazz);
    }
}
