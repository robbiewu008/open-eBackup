package openbackup.system.base.sdk.job.model;

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobGuardStatus;

import com.fasterxml.jackson.core.JsonProcessingException;

import org.junit.Assert;
import org.junit.Test;

import java.util.Locale;

/**
 * Job Guard Status Test
 *
 * @author l00272247
 * @since 2021-03-12
 */
public class JobGuardStatusTest {
    @Test
    public void testEnum() throws JsonProcessingException {
        String origin = "\"running\"";
        JobGuardStatus status = JSONObject.DEFAULT_OBJ_MAPPER.readValue(origin, JobGuardStatus.class);
        Assert.assertEquals(JobGuardStatus.RUNNING, status);
        String value = JSONObject.DEFAULT_OBJ_MAPPER.writeValueAsString(status);
        Assert.assertEquals(origin.toUpperCase(Locale.ENGLISH), value);
    }
}
