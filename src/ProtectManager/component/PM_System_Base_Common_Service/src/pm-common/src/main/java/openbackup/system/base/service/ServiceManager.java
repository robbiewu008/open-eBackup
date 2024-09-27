package openbackup.system.base.service;

import openbackup.system.base.common.annotation.Adapter;

import org.springframework.beans.BeansException;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.stereotype.Service;

import java.util.HashMap;
import java.util.Map;

/**
 * 外部认证类
 *
 * @author t00482481
 * @since 2020-08-10
 */
@Service
public class ServiceManager implements InitializingBean, ApplicationContextAware {
    private final Map<String, Object> serviceImplMap = new HashMap<>();

    private ApplicationContext applicationContext;

    /**
     * 根据枚举获取对应的实现类
     *
     * @param abstractClass 抽象类
     * @param serviceName   service名称
     * @return 实现类
     */
    public <T> T getService(Class<T> abstractClass, String serviceName) {
        return (T) serviceImplMap.get(serviceName + "_" + abstractClass.getSimpleName());
    }

    @Override
    public void afterPropertiesSet() {
        Map<String, Object> beanMap = applicationContext.getBeansWithAnnotation(Adapter.class);
        // 遍历该接口的所有实现，将其放入map中
        for (Object serviceImpl : beanMap.values()) {
            Adapter adapter = serviceImpl.getClass().getAnnotation(Adapter.class);
            for (String name : adapter.names()) {
                serviceImplMap.put(name + "_" + adapter.abstractClass(), serviceImpl);
            }
        }
    }

    @Override
    public void setApplicationContext(ApplicationContext applicationContext) throws BeansException {
        this.applicationContext = applicationContext;
    }
}
