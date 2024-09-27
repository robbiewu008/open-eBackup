package openbackup.data.access.client.sdk.api.framework.dme;

import junit.framework.TestCase;
import nl.jqno.equalsverifier.EqualsVerifier;
import nl.jqno.equalsverifier.Warning;
import openbackup.data.access.client.sdk.api.framework.dme.AvailableTimeRanges;

import org.junit.Test;

/**
 * 功能描述 AvailableTimeRange的测试用例
 *
 * @author s30031954
 * @since 2023-05-25
 */
public class AvailableTimeRangesTest extends TestCase {
    @Test
    public void test_AvailableTimeRanges() {
        EqualsVerifier.simple()
            .forClass(AvailableTimeRanges.class)
            .suppress(Warning.INHERITED_DIRECTLY_FROM_OBJECT)
            .suppress(Warning.ALL_NONFINAL_FIELDS_SHOULD_BE_USED)
            .verify();
        EqualsVerifier.simple()
            .forClass(AvailableTimeRanges.class)
            .suppress(Warning.INHERITED_DIRECTLY_FROM_OBJECT)
            .suppress(Warning.ALL_NONFINAL_FIELDS_SHOULD_BE_USED)
            .usingGetClass()
            .verify();
    }
}