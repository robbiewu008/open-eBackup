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
package openbackup.system.base.config;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.client.SimpleClientHttpRequestFactory;

import java.io.IOException;
import java.net.HttpURLConnection;

import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLSocketFactory;

/**
 * 自定义InternalStorageHttpRequestFactory
 *
 * @author y30046482
 * @since 2023-12-23
 */
public class InternalStorageHttpRequestFactory extends SimpleClientHttpRequestFactory {
    private static final Logger logger = LoggerFactory.getLogger(InternalStorageHttpRequestFactory.class);

    private final SSLSocketFactory internalTrustingSslSocketFactory;

    public InternalStorageHttpRequestFactory(SSLSocketFactory internalTrustingSslSocketFactory) {
        this.internalTrustingSslSocketFactory = internalTrustingSslSocketFactory;
    }

    @Override
    protected void prepareConnection(HttpURLConnection connection, String httpMethod) throws IOException {
        if (connection instanceof HttpsURLConnection) {
            prepareHttpsConnection((HttpsURLConnection) connection);
        }
        super.prepareConnection(connection, httpMethod);
    }

    private void prepareHttpsConnection(HttpsURLConnection connection) {
        HostnameVerifier hnv = (hostname, session) -> true;
        connection.setHostnameVerifier(hnv);
        connection.setSSLSocketFactory(internalTrustingSslSocketFactory);
    }
}

