package openbackup.data.access.framework.protection.mocks;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;

import org.assertj.core.util.Lists;

/**
 * 副本校验任务数据构造器
 *
 * @author: y00559272
 * @version: [OceanProtect X8000 1.2.1]
 * @since: 2022/8/6
 **/
public class CopyVerifyTaskMocker {
    public static CopyVerifyTask mockTask(String mockRequestId, String copyId) {
        CopyVerifyTask copyVerifyTask = new CopyVerifyTask();
        copyVerifyTask.setCopyId(copyId);
        copyVerifyTask.setRequestId(mockRequestId);
        copyVerifyTask.setTaskId(mockRequestId);
        copyVerifyTask.setAgents(Lists.newArrayList(new Endpoint("1", "9.9.9.9", 9999)));
        return copyVerifyTask;
    }
}
