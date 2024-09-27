package openbackup.data.protection.access.provider.sdk.livemount;

import openbackup.data.protection.access.provider.sdk.base.v2.BaseTask;

import java.util.Map;

/**
 * Live Mount Cancel Task
 *
 * @author l00272247
 * @since 2022-01-14
 */
public class LiveMountCancelTask extends BaseTask {
    private Map<String, Object> advanceParams;

    public Map<String, Object> getAdvanceParams() {
        return advanceParams;
    }

    public void setAdvanceParams(Map<String, Object> advanceParams) {
        this.advanceParams = advanceParams;
    }
}
