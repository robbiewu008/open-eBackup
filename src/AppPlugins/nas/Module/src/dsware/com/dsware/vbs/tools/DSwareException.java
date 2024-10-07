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
package com.dsware.vbs.tools;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * DSwareException
 *
 */
public class DSwareException extends RuntimeException {
    private static Method getErrorMethod;

    private static Method getDescriptionMethod;

    private static Method getErrorCodeMethod;

    private static Method getErrorCodeDescriptionMethod;

    private Error error;

    private String description;

    /**
     * DSwareException
     *
     * @param desc String
     */
    public DSwareException(String desc) {
        this.description = desc;
    }

    /**
     * DSwareException
     *
     * @param e Throwable
     */
    public DSwareException(Throwable e) {
        Throwable throwable = e;
        while (throwable instanceof InvocationTargetException) {
            throwable = ((InvocationTargetException) throwable).getTargetException();
        }
        if (!RuntimeLoad.getDswareClass(DSwareException.class.getSimpleName()).isInstance(throwable)) {
            this.description = throwable.toString();
            return;
        }
        try {
            Object dswareError = getErrorMethod.invoke(throwable);
            int dswareErrorCode = (int) getErrorCodeMethod.invoke(dswareError);
            String dswareDescription = String.valueOf(getErrorCodeDescriptionMethod.invoke(dswareError));
            this.error = new Error(dswareErrorCode, dswareDescription);
            this.description = String.valueOf(getDescriptionMethod.invoke(throwable));
        } catch (IllegalAccessException | InvocationTargetException ex) {
            this.description = "DSwareException invoke failed:" + ex;
        }
    }

    /**
     * loadExceptionMethod
     *
     * @param dswareExceptionClass Class<?>
     * @param dswareErrorCodeClass Class<?>
     * @throws NoSuchMethodException load dsware jar failed
     */
    public static void loadExceptionMethod(Class<?> dswareExceptionClass, Class<?> dswareErrorCodeClass)
        throws NoSuchMethodException {
        getErrorMethod = dswareExceptionClass.getMethod("getError");
        getDescriptionMethod = dswareExceptionClass.getMethod("getDescription");
        getErrorCodeMethod = dswareErrorCodeClass.getMethod("getErrorCode");
        getErrorCodeDescriptionMethod = dswareErrorCodeClass.getMethod("getDescription");
    }

    @Override
    public String getMessage() {
        return toString();
    }

    @Override
    public String toString() {
        StringBuilder buffer = new StringBuilder();
        buffer.append("DSwareException ").append(this.description);
        if (this.error != null) {
            buffer.append("//").append(this.error);
        }
        return buffer.toString();
    }

    /**
     * getError
     *
     * @return Error
     */
    public Error getError() {
        return this.error;
    }

    static class Error {
        private final int errorCode;

        private final String description;

        public Error(int errorCode, String description) {
            this.errorCode = errorCode;
            this.description = description;
        }

        public int getErrorCode() {
            return this.errorCode;
        }

        @Override
        public String toString() {
            return "ErrorCode=" + this.errorCode + "(" + this.description + ")";
        }
    }
}
