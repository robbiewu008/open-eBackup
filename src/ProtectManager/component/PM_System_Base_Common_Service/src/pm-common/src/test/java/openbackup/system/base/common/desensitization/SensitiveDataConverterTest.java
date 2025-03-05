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
package openbackup.system.base.common.desensitization;

import jodd.util.StringUtil;
import lombok.extern.slf4j.Slf4j;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * 功能描述
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {SensitiveDataConverter.class})
@Slf4j
public class SensitiveDataConverterTest {
    @Autowired
    private SensitiveDataConverter converter;

    @Test
    public void invokeMsg() {
        String test2
            = "Backoff FixedBackOff{interval=0, currentAttempts=10, maxAttempts=9} exhausted for ConsumerRecord(topic = ScanRequest, partition = 2, leaderEpoch = 0, offset = 3390, CreateTime = 1648473552937, serialized key size = -1, serialized value size = 1412, headers = RecordHeaders(headers = [], isReadOnly = false), key = null, value = {\"request_id\":\"aeb1fafa-ce1d-44df-a545-18ca4d502c60\",\"default_publish_topic\":\"ScanRequest\",\"response_topic\":\"ScanResponse\",\"snap_info\":{\"snap_id\":\"9481070a-c66c-47a0-b09d-eafedf5d3cb9\",\"snap_type\":\"Backup\",\"timestamp\":\"1648473464000000\",\"resource_id\":\"50245adf-5a07-bfa1-1f33-0785f0a9bcf6\",\"resource_name\":\"livemount_9\",\"resource_type\":\"vim.virtualmachine\",\"chain_id\":\"3b8b153c-9f0a-4a12-bcfc-7fbab8db2404\",\"gn\":14188,\"snap_metadata\":\"{\\\"disk_info\\\":[{\\\"DSNAME\\\":\\\"datastore2\\\",\\\"DSMOREF\\\":\\\"datastore-48\\\",\\\"BUSNUMBER\\\":\\\"SCSI(0:0)\\\",\\\"GUID\\\":\\\"6000c298-11d0-6af8-cfaa-10ab9e4f1c5a\\\",\\\"NAME\\\":\\\"Hard disk 1\\\",\\\"SIZE\\\":\\\"1048576\\\",\\\"DISKDEVICENAME\\\":\\\"9447610785871768757\\\",\\\"DISKSNAPSHOTDEVICENAME\\\":\\\"640673573888334465\\\"}],\\\"vmx_datastore\\\":{\\\"uuid\\\":\\\"datastore-48:616e2ccd-989d7aa0-9ed5-ec388f6b5948\\\",\\\"name\\\":\\\"datastore2\\\",\\\"mo_id\\\":\\\"datastore-48\\\"},\\\"net_work\\\":[\\\"Network adapter 1\\\"],\\\"runtime\\\":{\\\"host\\\":{\\\"name\\\":\\\"8.40.120.97\\\",\\\"uuid\\\":\\\"29c73c52-1dd2-11b2-b041-0018e1c5d866\\\",\\\"version\\\":\\\"6.7.0\\\",\\\"mo_id\\\":\\\"host-15\\\"}},\\\"hardware\\\":{\\\"num_cpu\\\":1,\\\"num_cores_per_socket\\\":1,\\\"memory\\\":1024,\\\"controller\\\":[\\\"IDE\\\",\\\"SCSI\\\"]}}\",\"user_id\":\"\"},\"storage_info\":{\"ip\":\"127.0.0.1\",\"port\":\"8088\",\"password\":\"AAAAAgAAAAAAAAAAAAAAAQAAAAkR+hRHFyKZYHF2fIJmto9RROJY2Xpqj5uu+wsfAAAAAAAAAAAAAAAAAAAAHssBH7Uiy9m9hSTGuadh4tAD0gS4iPWUFuYXpFkhPg==\",\"username\":\"admin\",\"storage_type\":\"DORADO\",\"protocol\":\"NAS\"}})";
        log.error("{}", test2);
        log.info(
            "Processing [GenericMessage [payload={\"password\":******,\"sk\":\"Yjw_yests\",\"ask\":\"Yjw_yests\"}, headers={kafka_offset=27, kafka_consumer=org.apache.kafka.clients.consumer.KafkaConsumer@2e2bb636, kafka_timestampType=CREATE_TIME, kafka_receivedPartitionId=3, kafka_receivedTopic=TEST_AAAA, kafka_receivedTimestamp=1649393057986, kafka_acknowledgment=Acknowledgment for ConsumerRecord(topic = TEST_AAAA, partition = 3, leaderEpoch = 0, offset = 27, CreateTime = 1649393057986, serialized key size = -1, serialized value size = 44, headers = RecordHeaders(headers = [], isReadOnly = false), key = null, value = {\"password\":******,\"name\":\"Yjw_yests\"}), kafka_groupId=DEE_Filesystem_Scanner}]]");
        log.info("safe:dasdas, iv:dsadas; ive:dsdsad; mk:dsadsa, mkdir:test, salt:dsadsa; Salty: dasdsad");
        log.error("Decode DeviceManagerResponse OK, but result is failed, errorCode:{}, description:{}, dec :dasdasd");
        log.info("get secret information from infra begin");
        log.info(
            "password: 123456, username: test, key: yaydasda, pass:1233, sk:adasdas, pwd:212, PWd: dasda, ask: dasdasd,Pass:456;cert=fffff");
        converter.invokeMsg("errorCode:dasdasd");
        String ans = converter.invokeMsg(
            "password: 123456, username: test, key: yaydasda, pass:1233, sk:adasdas, pwd:212, PWd: dasda, ask: dasdasd,Pass:456;cert=fffff");
        log.info("{}", ans);
        String expect
            = "password:******";
        log.info("{}", expect);
        log.info(
            "salt:123, private:test, verfiycode:12323, email:132130@qq.com, phone:1234234,rand:10, safe:ewe, user_info:huawei, PKCS1:dsadsa, base64:dsadsa, AES128:43243243; AES256:342432, RSA:4555, SHA1:23423, SHA256:423423,SHA384:423423, SHA512:rwerwerwe, algorithm:rewrwerwer, AccountNumber:143124, bank:qeqwe, cvv:qeqwe, checkno:qdasdas,mima:1232, CardPinNumber:134132, IDNumber:12312312");
        Assert.assertTrue(StringUtil.equals(ans, expect));
    }
}
