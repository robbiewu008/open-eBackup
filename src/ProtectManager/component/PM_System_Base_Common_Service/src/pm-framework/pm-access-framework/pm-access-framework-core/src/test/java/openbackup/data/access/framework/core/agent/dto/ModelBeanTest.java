package openbackup.data.access.framework.core.agent.dto;

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentLogLevelInfo;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CleanAgentLogReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CollectAgentLogRsp;
import openbackup.data.access.client.sdk.api.framework.agent.dto.GetAgentConfigRsp;
import openbackup.data.access.client.sdk.api.framework.agent.dto.GetAgentLogCollectStatusRsp;
import openbackup.data.access.client.sdk.api.framework.agent.dto.UpdateAgentLevelReq;

import org.junit.jupiter.api.Test;

/**
 * Bean测试用例
 *
 * @author y00357914
 * @since 2021-11-09
 */
public class ModelBeanTest {
    @Test
    public void test_agent_log_level_info_success() {
        EqualsVerifier.simple().forClass(AgentLogLevelInfo.class).verify();
        EqualsVerifier.simple().forClass(AgentLogLevelInfo.class).verify();
    }

    @Test
    public void test_collect_agent_log_rsp_success() {
        EqualsVerifier.simple().forClass(CollectAgentLogRsp.class).verify();
        EqualsVerifier.simple().forClass(CollectAgentLogRsp.class).verify();
    }

    @Test
    public void test_get_agent_config_rsp_success() {
        EqualsVerifier.simple().forClass(GetAgentConfigRsp.class).verify();
        EqualsVerifier.simple().forClass(GetAgentConfigRsp.class).verify();
    }

    @Test
    public void test_get_agent_collect_status_rsp_success() {
        EqualsVerifier.simple().forClass(GetAgentLogCollectStatusRsp.class).verify();
        EqualsVerifier.simple().forClass(GetAgentLogCollectStatusRsp.class).verify();
    }

    @Test
    public void test_update_agent_config_req_success() {
        EqualsVerifier.simple().forClass(UpdateAgentLevelReq.class).verify();
        EqualsVerifier.simple().forClass(UpdateAgentLevelReq.class).verify();
    }

    @Test
    public void test_clean_agent_log_req_success() {
        EqualsVerifier.simple().forClass(CleanAgentLogReq.class).verify();
        EqualsVerifier.simple().forClass(CleanAgentLogReq.class).verify();
    }
}
