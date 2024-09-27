package openbackup.system.base.common.aspect;

import java.util.ArrayList;

/**
 * 用于标识那些写一个表达式（如0!archivenetwork）就对应多个占位符的特殊表达式，每一个String对应一个占位符
 *
 * @author zwx1016945
 * @since 2021-03-24
 */
public class OperationLoggingList extends ArrayList<String> {
}
