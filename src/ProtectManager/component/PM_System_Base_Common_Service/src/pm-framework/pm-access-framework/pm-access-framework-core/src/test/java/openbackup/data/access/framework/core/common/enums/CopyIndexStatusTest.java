package openbackup.data.access.framework.core.common.enums;


import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;

import org.junit.Assert;
import org.junit.Test;

/**
 * CopyIndexStatus LLT
 *
 * @author z30027603
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-07-26
 */
public class CopyIndexStatusTest {
    @Test
    public void get_status_success() {
        Assert.assertEquals(CopyIndexStatus.UNINDEXED.getStatus(), "Unindexed");
        Assert.assertEquals(CopyIndexStatus.INDEXING.getStatus(), "Indexing");
        Assert.assertEquals(CopyIndexStatus.INDEXED.getStatus(), "Indexed");
        Assert.assertEquals(CopyIndexStatus.INDEX_FAIL.getStatus(), "Index_fail");
        Assert.assertEquals(CopyIndexStatus.INDEX_DELETING.getStatus(), "Index_deleting");
        Assert.assertEquals(CopyIndexStatus.INDEX_DELETE_FAIL.getStatus(), "Index_delete_fail");
        Assert.assertEquals(CopyIndexStatus.INDEX_RESPONSE_ERROR_LABEL.getStatus(), "index_response_error_label");
        Assert.assertEquals(CopyIndexStatus.INDEX_COPY_STATUS_ERROR_LABEL.getStatus(), "index_copy_status_error_label");
    }
}
