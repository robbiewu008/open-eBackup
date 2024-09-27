package openbackup.access.framework.resource.mock;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.AbstractResourceChecker;
import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Map;

/**
 * The MockResourceChecker
 *
 * @author g30003063
 * @since 2022/5/30
 */
@Component
public class MockResourceChecker extends AbstractResourceChecker<AppEnvResponse> {
    private final UnifiedResourceConnectionChecker connectionChecker;

    private final UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker;

    public MockResourceChecker(final ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        final UnifiedResourceConnectionChecker connectionChecker,
        final UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker) {
        super(environmentRetrievalsService);
        this.connectionChecker = connectionChecker;
        this.clusterIntegrityChecker = clusterIntegrityChecker;
    }

    @Override
    public CheckResult<AppEnvResponse> generateCheckResult(final ProtectedResource protectedResource) {
        CheckResult<Object> connectionCheckResult = connectionChecker.generateCheckResult(protectedResource);
        CheckResult<AppEnvResponse> integrityCheckResult = clusterIntegrityChecker.generateCheckResult(
            protectedResource);
        integrityCheckResult.setResults(connectionCheckResult.getResults());
        return integrityCheckResult;
    }

    @Override
    public List<ActionResult> collectActionResults(final List<CheckReport<AppEnvResponse>> checkReports,
        Map<String, Object> context) {
        checkReports.forEach(this::checkConnectionReport);
        List<ActionResult> integrityActionResults = clusterIntegrityChecker.collectActionResults(checkReports, context);
        integrityActionResults.forEach(this::checkIntegrity);
        return Collections.emptyList();
    }

    private void checkIntegrity(ActionResult actionResult) {
        if (actionResult.getCode() != 0) {
            throw new LegoCheckedException(actionResult.getCode(), actionResult.getMessage());
        }
    }

    private void checkConnectionReport(CheckReport<AppEnvResponse> checkReport) {
        for (CheckResult<AppEnvResponse> result : checkReport.getResults()) {
            if (result.getResults().getCode() == 0) {
                return;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "File to check connection.");
    }

    @Override
    public boolean applicable(final ProtectedResource object) {
        return "mock_sub_type".equals(object.getSubType());
    }
}
