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
