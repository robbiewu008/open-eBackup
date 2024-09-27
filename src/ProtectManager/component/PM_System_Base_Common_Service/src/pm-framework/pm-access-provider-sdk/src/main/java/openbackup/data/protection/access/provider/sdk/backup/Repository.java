/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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