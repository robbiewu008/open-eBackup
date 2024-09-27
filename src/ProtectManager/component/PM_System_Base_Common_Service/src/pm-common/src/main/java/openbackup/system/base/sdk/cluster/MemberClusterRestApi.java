/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.cluster;

import openbackup.system.base.sdk.cluster.request.CertReplaceRequest;
import openbackup.system.base.sdk.cluster.request.HaCertRequest;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestHeader;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RequestPart;
import org.springframework.web.multipart.MultipartFile;

import java.net.URI;
import java.util.List;

/**
 * MemberClusterRestApi
 *
 * @author y30046482
 * @since 2023-05-27
 */
public interface MemberClusterRestApi {
    /**
     * 调用成员集群添加主集群证书
     *
     * @param uri uri
     * @param token token
     * @param name name
     * @param type type
     * @param ca ca
     */
    @ExterAttack
    @PostMapping(value = "/v1/certs/components", consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    void addMainClusterCertFile(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @RequestParam("name") String name, @RequestParam("type") String type, @RequestPart("ca") MultipartFile ca);

    /**
     * 添加外部证书，不同步，不操作数据库
     *
     * @param uri uri
     * @param token token
     * @param name name
     * @param type type
     * @param ca ca
     * @return Void
     */
    @ExterAttack
    @PostMapping(value = "/v1/certs/components?sync=false", consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    Void registerExternalCertNotSync(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @RequestParam("name") String name, @RequestParam("type") String type, @RequestPart("ca") MultipartFile ca);

    /**
     * 导入吊销列表
     *
     * @param uri uri
     * @param token token
     * @param crl crl
     * @param componentId componentId
     * @return 吊销列表ID
     */
    @ExterAttack
    @PostMapping(value = "/v1/certs/components/{componentId}/crls/action/import?sync=false",
        consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    String importCrlNotSync(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @RequestPart(value = "crl") MultipartFile crl, @PathVariable(value = "componentId") String componentId);

    /**
     * 删除吊销列表
     *
     * @param uri uri
     * @param token token
     * @param crlId crlId
     * @param componentId componentId
     * @return Void
     */
    @ExterAttack
    @DeleteMapping(value = "/v1/certs/components/{componentId}/crls/{crlId}?sync=false")
    Void deleteCrlNotSync(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @PathVariable(value = "componentId") String componentId, @PathVariable(value = "crlId") String crlId);

    /**
     * 导入私钥
     *
     * @param uri uri
     * @param token token
     * @param keyPass keyPass
     * @param componentId componentId
     * @param keyFile keyFile
     * @return Void
     */
    @ExterAttack
    @PostMapping(value = "/v1/certs/components/{componentId}/key/action/import",
        consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    Void importPrivateKey(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @PathVariable("componentId") String componentId,
        @RequestHeader(value = "keyPass", required = false) String keyPass,
        @RequestPart("keyFile") MultipartFile keyFile);

    /**
     * 删除私钥
     *
     * @param uri uri
     * @param token token
     * @param componentId componentId
     * @return Void
     */
    @ExterAttack
    @DeleteMapping(value = "/v1/certs/components/{componentId}/key")
    Void deletePrivateKey(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @PathVariable("componentId") String componentId);

    /**
     * 替换证书，不同步，不操作数据库
     *
     * @param uri uri
     * @param token token
     * @param componentId componentId
     * @param serverPass serverPass
     * @param files files
     * @return Void
     */
    @ExterAttack
    @PostMapping(value = "/v1/certs/components/{componentId}/action/import?sync=false",
        consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    Void replaceCertNotSync(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @PathVariable(value = "componentId") String componentId,
        @RequestHeader(value = "serverPass", required = false) String serverPass,
        @RequestBody CertReplaceRequest files);

    /**
     * 更新ha证书
     *
     * @param uri uri
     * @param token token
     * @param componentId componentId
     * @param files ha证书列表
     * @return Void
     */
    @ExterAttack
    @PostMapping(value = "/v1/certs/components/{componentId}/ha/action/update",
        consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    Void updateHaCert(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @PathVariable(value = "componentId") String componentId, @RequestBody HaCertRequest files);

    /**
     * 证书生效并重启服务
     *
     * @param uri uri
     * @param token token
     * @param certType 证书类型
     * @return Void
     */
    @ExterAttack
    @PostMapping(value = "/v1/certs/action/reboot")
    Void reboot(URI uri, @RequestHeader(name = "x-auth-token") String token, @RequestParam("certType") int certType);

    /**
     * 删除外部证书，不同步，不操作数据库
     *
     * @param uri uri
     * @param token token
     * @param componentId componentId
     * @return Void
     */
    @ExterAttack
    @DeleteMapping(value = "/v1/certs/components/{componentId}?sync=false")
    Void deleteExternalCertNotSync(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @PathVariable(value = "componentId") String componentId);

    /**
     * 调用成员集群验证license
     *
     * @param uri uri
     * @param token token
     * @param function function
     * @param resourceType resourceType
     */
    @ExterAttack
    @GetMapping(value = "/v1/license/function")
    void licenseFunction(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @RequestParam("function") String function, @RequestParam("resourceType") String resourceType);

    /**
     * 同步HA证书
     *
     * @param uri uri
     * @param token token
     * @param certFiles 证书
     */
    @ExterAttack
    @PostMapping(value = "/v1/clusters/ha/cert/sync", consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    void syncHaCerts(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @RequestPart("certFiles") List<MultipartFile> certFiles);

    /**
     * 同步秘钥证书文件
     *
     * @param uri target url addr
     * @param token token
     * @param kmcCertFiles 来自主节点的被压缩的pm和infra的证书文件
     */
    @ExterAttack
    @PostMapping(value = "/v1/clusters/backup/sync-kmc-cert", consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    void syncKmcCertFiles(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @RequestPart("kmcCertFiles") List<MultipartFile> kmcCertFiles);

    /**
     * 回退秘钥证书文件
     *
     * @param uri target url addr
     * @param token token
     */
    @ExterAttack
    @PostMapping(value = "/v1/clusters/backup/rollback-kmc-cert")
    void rollbackKmcCertFiles(URI uri, @RequestHeader(name = "x-auth-token") String token);

    /**
     * 暂存重新生成的内部证书 只支持内部通信证书和内部数据库证书两种类型
     *
     * @param uri uri
     * @param token token
     * @param componentId componentId
     * @param serverPass serverPass
     * @param files files
     * @return Void
     */
    @ExterAttack
    @PostMapping(value = "/v1/certs/components/{componentId}/action/sync",
        consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    Void saveRegenerateInternalCert(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @PathVariable(value = "componentId") String componentId,
        @RequestHeader(value = "serverPass", required = false) String serverPass,
        @RequestBody CertReplaceRequest files);

    /**
     * 调用成员集群进行管理数据备份直接恢复
     *
     * @param uri uri
     * @param token token
     * @param file 验签文件
     * @param password 密码
     */
    @ExterAttack
    @PostMapping(value = "v1/sysbackup/recovery/upload", consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    void uploadRecovery(URI uri, @RequestHeader(name = "x-auth-token") String token,
        @RequestPart("file") MultipartFile file, @RequestParam("password") String password);
}