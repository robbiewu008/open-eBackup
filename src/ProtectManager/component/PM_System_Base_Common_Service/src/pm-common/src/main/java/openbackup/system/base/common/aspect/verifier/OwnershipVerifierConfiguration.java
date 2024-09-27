/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.common.aspect.verifier;

import openbackup.system.base.sdk.alarm.CommonAlarmService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.filesystem.FileSystemRestApi;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.livemount.LiveMountOwnershipVerifyClientRestApi;
import openbackup.system.base.sdk.resource.ResourceRestApi;
import openbackup.system.base.sdk.sla.SlaRestApi;
import openbackup.system.base.sdk.storage.StorageRestClient;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

/**
 * Ownership Verifier Configuration
 *
 * @author l00272247
 * @since 2020-11-28
 */
@Configuration
public class OwnershipVerifierConfiguration {
    @Autowired
    private ResourceRestApi resourceRestApi;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private LiveMountOwnershipVerifyClientRestApi liveMountOwnershipVerifyClientRestApi;

    @Autowired
    private StorageRestClient storageRestClient;

    @Autowired
    private JobCenterRestApi jobCenterRestApi;

    @Autowired
    private SlaRestApi slaRestApi;

    @Autowired
    private CommonAlarmService commonAlarmService;

    @Autowired
    private FileSystemRestApi fileSystemRestApi;

    /**
     * resource ownership verifier
     *
     * @return resourceOwnershipVerifier
     */
    @Bean("resourceOwnershipVerifier")
    public CommonOwnershipVerifier getResourceOwnershipVerifier() {
        return new CommonOwnershipVerifier("resource", resourceRestApi::verifyResourceOwnership);
    }

    /**
     * copy ownership verifier
     *
     * @return copyOwnershipVerifier
     */
    @Bean("copyOwnershipVerifier")
    public CommonOwnershipVerifier getCopyOwnershipVerifier() {
        return new CommonOwnershipVerifier("copy", copyRestApi::verifyCopyOwnership);
    }

    /**
     * get live mount ownership verifier
     *
     * @return liveMountOwnershipVerifier
     */
    @Bean("liveMountOwnershipVerifier")
    public CommonOwnershipVerifier getLiveMountOwnershipVerifier() {
        return new CommonOwnershipVerifier("live_mount",
            liveMountOwnershipVerifyClientRestApi::verifyLiveMountOwnership);
    }

    /**
     * get live mount policy ownership verifier
     *
     * @return liveMountPolicyOwnershipVerifier
     */
    @Bean("liveMountPolicyOwnershipVerifier")
    public CommonOwnershipVerifier getLiveMountPolicyOwnershipVerifier() {
        return new CommonOwnershipVerifier("live_mount_policy",
            liveMountOwnershipVerifyClientRestApi::verifyLiveMountPolicyOwnership);
    }

    /**
     * storage ownership verifier
     *
     * @return storageOwnershipVerifier
     */
    @Bean("storageOwnershipVerifier")
    public CommonOwnershipVerifier getStorageOwnershipVerifier() {
        return new CommonOwnershipVerifier("storage", storageRestClient::verifyStorageOwnership);
    }

    /**
     * job ownership verifier
     *
     * @return jobOwnershipVerifier
     */
    @Bean("jobOwnershipVerifier")
    public CommonOwnershipVerifier getJobOwnershipVerifier() {
        return new CommonOwnershipVerifier("job", jobCenterRestApi::verifyJobOwnership);
    }

    /**
     * sla ownership verifier
     *
     * @return slaOwnershipVerifier
     */
    @Bean("slaOwnershipVerifier")
    public CommonOwnershipVerifier getSlaOwnershipVerifier() {
        return new CommonOwnershipVerifier("sla", slaRestApi::verifySlaOwnership);
    }

    /**
     * sla ownership verifier
     *
     * @return slaOwnershipVerifier
     */
    @Bean("alarmEntityOwnershipVerifier")
    public CommonOwnershipVerifier getAlarmEntityOwnershipVerifier() {
        return new CommonOwnershipVerifier("alarm_entity", commonAlarmService::verifyAlarmEntityOwnership);
    }

    /**
     * sla ownership verifier
     *
     * @return slaOwnershipVerifier
     */
    @Bean("fileSystemOwnershipVerifier")
    public CommonOwnershipVerifier getFileSystemOwnershipVerifier() {
        return new CommonOwnershipVerifier("file_system", fileSystemRestApi::verifyNasFileSystemMountIdOwnership);
    }
}
