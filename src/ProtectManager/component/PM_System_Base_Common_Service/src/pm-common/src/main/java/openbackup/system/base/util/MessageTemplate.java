package openbackup.system.base.util;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.kafka.support.SendResult;
import org.springframework.stereotype.Component;
import org.springframework.util.concurrent.ListenableFuture;

import java.util.UUID;

/**
 * Message Template
 *
 * @param <K> template K
 * @author l00272247
 * @since 2020-09-20
 */
@Component
@Slf4j
public class MessageTemplate<K> {
    /**
     * REQUEST ID
     */
    public static final String REQUEST_ID = "request_id";
    private static final int RETRY_COUNT = 3;

    private static final long TWO_MINUTES = 2 * 60 * 1000L;


    @Autowired
    private KafkaTemplate<K, String> kafkaTemplate;

    @Autowired
    private MessageCallback<K, String> callback;

    /**
     * send method
     *
     * @param topic topic
     * @param data data
     * @return future
     */
    public ListenableFuture<SendResult<K, String>> send(String topic, JSONObject data) {
        if (!data.containsKey(REQUEST_ID)) {
            data.set(REQUEST_ID, UUID.randomUUID().toString());
        }
        return send(topic, data.toString(), 1);
    }

    /**
     * send method
     *
     * @param topic topic
     * @param data data
     * @return future
     */
    public ListenableFuture<SendResult<K, String>> send(String topic, String data) {
        return send(topic, data, 0);
    }

    private ListenableFuture<SendResult<K, String>> send(String topic, String data, int stack) {
        JSONObject json = JSONObject.fromObject(data);
        String requestId = json.getString(REQUEST_ID);
        String caller = ExceptionUtil.getCaller(stack + IsmNumberConstant.TWO);
        log.info("send message. topic: {}, requestId: {}, caller: {}", topic, requestId, caller);
        kafkaTemplate.setProducerListener(callback);
        int count = 1;
        while (true) {
            try {
                return kafkaTemplate.send(topic, data);
            } catch (Exception e) {
                log.error("Send message failed, failedCount: {}.", count, ExceptionUtil.getErrorMessage(e));
            }
            count++;
            if (count > RETRY_COUNT) {
                break;
            }
            try {
                Thread.sleep(TWO_MINUTES);
            } catch (InterruptedException e) {
                log.error("Thread sleep failed.", ExceptionUtil.getErrorMessage(e));
            }
        }
        log.error("Send message failed after retry, topic: {}", topic);
        throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Send message failed.");
    }
}
