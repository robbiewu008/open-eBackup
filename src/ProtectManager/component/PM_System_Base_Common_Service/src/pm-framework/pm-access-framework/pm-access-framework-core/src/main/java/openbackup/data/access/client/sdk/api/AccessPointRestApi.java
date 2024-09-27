package openbackup.data.access.client.sdk.api;

import openbackup.system.base.sdk.copy.model.CopyReplicationImportParam;
import openbackup.system.base.security.exterattack.ExterAttack;

import feign.Headers;
import feign.Param;
import feign.RequestLine;

import org.springframework.web.bind.annotation.RequestBody;

import java.net.URI;

/**
 * Access Point Rest Api
 *
 * @author l00272247
 * @since 2020-12-18
 */
public interface AccessPointRestApi {
    /**
     * import copy
     *
     * @param uri uri
     * @param token token
     * @param importParam import param
     * @return result
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters/copies/import")
    @Headers("x-auth-token: {token}")
    boolean importCopy(URI uri, @Param("token") String token, @RequestBody CopyReplicationImportParam importParam);
}
