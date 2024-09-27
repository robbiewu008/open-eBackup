/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.stereotype.Component;

import java.util.Collection;
import java.util.Objects;

/**
 * DataMover provider registry
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-22
 */
@Component
public class ProviderRegistry {
    private final ApplicationContext context;

    /**
     * constructor
     *
     * @param context context
     */
    public ProviderRegistry(@Autowired ApplicationContext context) {
        this.context = context;
    }

    /**
     * Find data mover provider by resource type and provider class
     *
     * @param providerClazz Provider clazz
     * @param identification identification
     * @param <E> template E
     * @param <T> template T
     * @return provider
     */
    public <E, T extends Applicable<E>> T findProvider(Class<T> providerClazz, E identification) {
        return findProvider(providerClazz, identification,
            new LegoCheckedException(CommonErrorCode.OPERATION_FAILED,
                "not found applicable provider for resource type: " + identification));
    }

    /**
     * Find data mover provider by resource type and provider class
     *
     * @param providerClazz Provider clazz
     * @param identification identification
     * @param exception exception
     * @param <E> template E
     * @param <T> template T
     * @return provider
     */
    public <E, T extends Applicable<E>> T findProvider(Class<T> providerClazz, E identification,
        LegoCheckedException exception) {
        Collection<T> beans = findProviders(providerClazz);
        for (T bean : beans) {
            if (bean.applicable(identification)) {
                return bean;
            }
        }
        if (exception != null) {
            throw exception;
        }
        return null;
    }

    /**
     * Find data mover provider by resource type and provider class
     *
     * @param providerClazz Provider clazz
     * @param identification identification
     * @param defaultProvider default provider
     * @param <E> template E
     * @param <T> template T
     * @return provider
     */
    public <E, T extends Applicable<E>> T findProviderOrDefault(
            Class<T> providerClazz, E identification, T defaultProvider) {
        LegoCheckedException exception = null;
        T provider = findProvider(providerClazz, identification, exception);
        if (provider != null) {
            return provider;
        }
        return Objects.requireNonNull(defaultProvider);
    }

    /**
     * find providers by type
     *
     * @param providerClazz provider class
     * @param <E> template e
     * @param <T> template t
     * @return provider list
     */
    public <E, T extends Applicable<E>> Collection<T> findProviders(Class<T> providerClazz) {
        return context.getBeansOfType(providerClazz).values();
    }
}
