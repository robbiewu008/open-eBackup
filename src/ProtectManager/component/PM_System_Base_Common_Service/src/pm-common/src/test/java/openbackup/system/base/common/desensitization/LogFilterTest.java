package openbackup.system.base.common.desensitization;

import static org.assertj.core.api.BDDAssertions.then;

import ch.qos.logback.classic.spi.LoggingEvent;
import ch.qos.logback.core.spi.FilterReply;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.desensitization.LogFilter;

import org.junit.Test;

/**
 * 日志过滤器单元测试集合
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/3/22
 **/
@Slf4j
public class LogFilterTest {
    final LogFilter logFilter = new LogFilter();
    @Test
    public void should_accept_when_test_filter_given_log_not_contains_sensitive_word() {
        // Given
        String msg = "send schedule message, params={\"uuid\":\"8be715c4-e15c-4a6d-b642-256cbb0e3eb1\",\"timestamp\":1647401522034,\"msg_id\":\"08a8fba6-d7b4-4eee-9cfe-782acc08b203\",\"request_id\":\"0c00cddb-53fb-42a3-af26-b044ac9bba07\",\"default_publish_topic\":\"Sanning_environment_v2\"}";
        // When and Then
        then(logFilter.decide(this.buildLoggingEvent(msg))).isEqualTo(FilterReply.ACCEPT);
    }

    @Test
    public void should_deny_when_test_filter_given_log_contains_sensitive_word() {
        // When and Then
        then(logFilter.decide(this.buildLoggingEvent("get aka testak fdfak1 ussss test"))).isEqualTo(FilterReply.ACCEPT);
        then(logFilter.decide(this.buildLoggingEvent("get aka testaiv fdivk1 ussss test"))).isEqualTo(FilterReply.ACCEPT);
        then(logFilter.decide(this.buildLoggingEvent("get aka mkak fdfamk sdsmksss test"))).isEqualTo(FilterReply.ACCEPT);
        then(logFilter.decide(this.buildLoggingEvent("i don't have password"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("i don't have pass!"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("i don't have,pwd"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("i don't have!key"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("i don't have # crypto."))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("i don't have. have [session] test again"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("token: 12324343 is create"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("fingerprint[0e99r8ejr82fds] is success"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("generate auth info [huawei@123]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("enc[aaaaa] and dec is useraaaa"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("tgt is userabca"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("get iqn from xxx, value is abc1"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("initiator"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("init secret xxxxx success"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check cert failed, cert name is [dsdsdsds]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("use salt, value is xxxxxx"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("this is a private info, value is not support "))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("user login success, verfiycode is abcdsds"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("user email is abc@aacfuf.com"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("my phone number is 136404038543"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("have a rand value"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check safe failed"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("user_info is user info"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [PKCS1]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [base64]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [AES128]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [AES256]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [RSA]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [SHA1]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [SHA256]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [SHA384]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [SHA512]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [algorithm]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [AccountNumber]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [bank]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [cvv]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [checkno]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [mima]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [CardPinNumber]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("check text failed, value is [IDNumber]"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("get ak from db, is 9d8f9d822hjh"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("get !ak from db, is 9d8f9d822hjh"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("get %ak@ from db, is 9d8f9d822hjh"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("get iv from db, is 9d8f9d822hjh"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("get !iv from db, is 9d8f9d822hjh"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("get %iv@ from db, is 9d8f9d822hjh"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("get mk from db, is 9d8f9d822hjh"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("get !mk from db, is 9d8f9d822hjh"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("get %mk@ from db, is 9d8f9d822hjh"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("get sk from db, is 9d8f9d822hjh"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("get !sk from db, is 9d8f9d822hjh"))).isEqualTo(FilterReply.DENY);
        then(logFilter.decide(this.buildLoggingEvent("get %sk@ from db, is 9d8f9d822hjh"))).isEqualTo(FilterReply.DENY);
        String testStr = "{\"msg_id\":\"b816c82f-0193-44ce-b24c-69ebc91590a5\",\"request_id\":\"205f0043-2cdf-490c-8c2a-9321b97f8882\",\"default_publish_topic\":\"protection.replication\",\"response_topic\":\"\",\"related_request_id\":\"20d81d01-267c-4e7f-ab37-e6535bbc28b7\",\"resource_id\":\"25957563-c987-3a04-a9fa-903f21b68713\",\"execute_type\":\"AUTOMATIC\",\"sla\":{\"name\":\"NasShare_backup_copy\",\"type\":1,\"application\":\"NasShare\",\"created_time\":\"2022-04-12T10:32:03.275510\",\"uuid\":\"4ef68227-4b4c-43ef-81bc-2e91c0fb3989\",\"is_global\":false,\"policy_list\":[{\"uuid\":\"b7881b98-e03f-403a-8fe7-b83bd0900661\",\"name\":\"permanent_increment\",\"action\":\"permanent_increment\",\"ext_parameters\":{\"auto_retry\":true,\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5,\"qos_id\":\"\",\"auto_index\":false},\"retention\":{\"retention_type\":1,\"duration_unit\":null,\"retention_duration\":null},\"schedule\":{\"trigger\":1,\"interval\":10,\"interval_unit\":\"m\",\"start_time\":\"2022-04-12T00:00:00\",\"window_start\":\"00:00:00\",\"window_end\":\"00:00:00\",\"days_of_month\":null,\"days_of_year\":null,\"trigger_action\":null,\"days_of_week\":null},\"type\":\"backup\"},{\"uuid\":\"a296cf01-2634-4a9e-9305-882cea8a09e0\",\"name\":\"策略0\",\"action\":\"replication\",\"ext_parameters\":{\"qos_id\":\"\",\"external_system_id\":\"4\"},\"retention\":{\"retention_type\":1,\"duration_unit\":null,\"retention_duration\":null},\"schedule\":{\"trigger\":2,\"interval\":20,\"interval_unit\":\"m\",\"start_time\":\"2022-04-12T00:31:20\",\"window_start\":null,\"window_end\":null,\"days_of_month\":null,\"days_of_year\":null,\"trigger_action\":null,\"days_of_week\":null},\"type\":\"replication\"},{\"uuid\":\"aebc1ccc-3c5c-4bcd-9ab2-173c885088a3\",\"name\":\"策略1\",\"action\":\"replication\",\"ext_parameters\":{\"qos_id\":\"\",\"external_system_id\":\"5\"},\"retention\":{\"retention_type\":1,\"duration_unit\":null,\"retention_duration\":null},\"schedule\":{\"trigger\":2,\"interval\":0,\"interval_unit\":\"h\",\"start_time\":\"2022-04-13T00:08:13\",\"window_start\":null,\"window_end\":null,\"days_of_month\":null,\"days_of_year\":null,\"trigger_action\":null,\"days_of_week\":null},\"type\":\"replication\"}],\"resource_count\":null,\"archival_count\":null,\"replication_count\":null},\"policy\":{\"uuid\":\"a296cf01-2634-4a9e-9305-882cea8a09e0\",\"name\":\"策略0\",\"action\":\"replication\",\"ext_parameters\":{\"qos_id\":\"\",\"external_system_id\":\"4\"},\"retention\":{\"retention_type\":1,\"duration_unit\":null,\"retention_duration\":null},\"schedule\":{\"trigger\":2,\"interval\":20,\"interval_unit\":\"m\",\"start_time\":\"2022-04-12T00:31:20\",\"window_start\":null,\"window_end\":null,\"days_of_month\":null,\"days_of_year\":null,\"trigger_action\":null,\"days_of_week\":null},\"type\":\"replication\"},\"resource\":{\"name\":\"ljh_test_cifs_1\",\"path\":\"ljh_test_cifs_1\",\"root_uuid\":\"12345678922987654321\",\"parent_name\":\"V5_98_90\",\"parent_uuid\":\"12345678922987654321\",\"children_uuids\":null,\"type\":\"Storage\",\"sub_type\":\"NasShare\",\"uuid\":\"25957563-c987-3a04-a9fa-903f21b68713\",\"created_time\":\"2022-04-08T18:26:29.979000\",\"ext_parameters\":{\"proxy_host_mode\":0,\"agents\":null,\"small_file_aggregation\":1,\"permissions_and_attributes\":0},\"authorized_user\":null,\"user_id\":\"5988d9b358c54989b6f698c7e3ffacf8\",\"version\":null,\"sla_id\":\"4ef68227-4b4c-43ef-81bc-2e91c0fb3989\",\"sla_name\":\"NasShare_backup_copy\",\"sla_status\":true,\"sla_compliance\":true,\"protection_status\":1,\"environment_uuid\":\"12345678922987654321\",\"environment_name\":\"V5_98_90\",\"environment_endpoint\":\"8.40.98.90\",\"environment_os_type\":null,\"environment_type\":\"StorageEquipment\",\"environment_sub_type\":\"OceanStorV5\",\"environment_is_cluster\":\"False\",\"environment_os_name\":null,\"extendInfo\":{\"authMode\":\"0\",\"passWord\":\"Admin@123\",\"shareMode\":\"0\",\"encryption\":\"0\",\"ip\":\"192.168.98.90\",\"kerberosId\":\"\",\"filters\":\"[]\",\"userName\":\"ljh_test\",\"isAutoScan\":\"1\"}},\"current_operate_user_id\":\"5988d9b358c54989b6f698c7e3ffacf8\",\"resource_obj\":{\"name\":\"ljh_test_cifs_1\",\"path\":\"ljh_test_cifs_1\",\"root_uuid\":\"12345678922987654321\",\"parent_name\":\"V5_98_90\",\"parent_uuid\":\"12345678922987654321\",\"children_uuids\":null,\"type\":\"Storage\",\"sub_type\":\"NasShare\",\"uuid\":\"25957563-c987-3a04-a9fa-903f21b68713\",\"created_time\":\"2022-04-08T18:26:29.979000\",\"ext_parameters\":{\"proxy_host_mode\":0,\"agents\":null,\"small_file_aggregation\":1,\"permissions_and_attributes\":0},\"authorized_user\":null,\"user_id\":\"5988d9b358c54989b6f698c7e3ffacf8\",\"version\":null,\"sla_id\":\"4ef68227-4b4c-43ef-81bc-2e91c0fb3989\",\"sla_name\":\"NasShare_backup_copy\",\"sla_status\":true,\"sla_compliance\":true,\"protection_status\":1,\"environment_uuid\":\"12345678922987654321\",\"environment_name\":\"V5_98_90\",\"environment_endpoint\":\"8.40.98.90\",\"environment_os_type\":null,\"environment_type\":\"StorageEquipment\",\"environment_sub_type\":\"OceanStorV5\",\"environment_is_cluster\":\"False\",\"environment_os_name\":null},\"protected_obj\":{\"latest_time\":\"2022-04-14T04:00:02.790295\",\"earliest_time\":\"2022-04-09T09:38:15.122802\",\"next_time\":null,\"sla_id\":\"4ef68227-4b4c-43ef-81bc-2e91c0fb3989\",\"sla_name\":\"NasShare_backup_copy\",\"resource_id\":\"25957563-c987-3a04-a9fa-903f21b68713\",\"name\":\"ljh_test_cifs_1\",\"path\":\"ljh_test_cifs_1\",\"env_id\":\"12345678922987654321\",\"env_type\":null,\"type\":\"Storage\",\"sub_type\":\"NasShare\",\"status\":1,\"ext_parameters\":{\"proxy_host_mode\":0,\"agents\":null,\"small_file_aggregation\":1,\"permissions_and_attributes\":0}},\"job_id\":\"205f0043-2cdf-490c-8c2a-9321b97f8882\"}";
        then(logFilter.decide(this.buildLoggingEvent(testStr))).isEqualTo(FilterReply.DENY);
    }

    private LoggingEvent buildLoggingEvent(String msg){
        final LoggingEvent loggingEvent = new LoggingEvent();
        loggingEvent.setMessage(msg);
        return loggingEvent;
    }
}