package openbackup.tdsql.resources.access.util;

import openbackup.tdsql.resources.access.dto.instance.DataNode;

import org.apache.commons.collections4.Equator;

import java.util.Objects;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-05-25
 */
public class DataNodeEquator implements Equator<DataNode> {
    @Override
    public boolean equate(DataNode dataNode1, DataNode dataNode2) {
        if (dataNode1 == null && dataNode2 == null) {
            return true;
        }

        if (dataNode1 == null || dataNode2 == null) {
            return false;
        }

        if (dataNode1 == dataNode2) {
            return true;
        }

        return Objects.equals(dataNode1.getIp(), dataNode2.getIp())
            && Objects.equals(dataNode1.getPort(), dataNode2.getPort());
    }

    @Override
    public int hash(DataNode dataNode) {
        return Objects.hash(dataNode.getIp(), dataNode.getPort(), dataNode.getIsMaster());
    }
}
