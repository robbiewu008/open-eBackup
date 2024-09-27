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
package openbackup.data.protection.access.provider.sdk.backup;

import lombok.Data;

import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;

/**
 * Backup Repository
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-17
 */
@Data
public class Repository {
    private String uuid;

    private String type;

    private String name;

    private String protocol;

    private String username;

    private String password;

    private String endpoint;

    private int port;

    /**
     * build uri
     *
     * @return uri
     */
    public URI buildUri() {
        return buildUri("/");
    }

    /**
     * build uri
     *
     * @param path path
     * @return uri
     */
    public URI buildUri(String path) {
        URI uri;
        try {
            uri = new URL("http", endpoint, port, path).toURI();
        } catch (MalformedURLException | URISyntaxException e) {
            throw new IllegalArgumentException("build uri failed", e);
        }
        return uri;
    }
}