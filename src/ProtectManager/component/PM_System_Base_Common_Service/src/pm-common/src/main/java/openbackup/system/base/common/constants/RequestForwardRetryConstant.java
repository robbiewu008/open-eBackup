package openbackup.system.base.common.constants;

/**
 * 跨控制器转发http请求相关的常量
 *
 * @author z00850125
 * @since 2024-04-19
 */
public class RequestForwardRetryConstant {
    /**
     * 仅内部用于标识某次转发其他节点进行重试，key本身命名不会持久化，即使后续改名也不涉及演进的兼容性问题
     */
    public static final String HTTP_HEADER_INTERNAL_RETRY = "internal-retry";
}
