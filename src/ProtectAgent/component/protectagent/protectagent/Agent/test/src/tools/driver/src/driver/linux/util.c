/*
 * Copyright (c) 2014 Huawei Technologies Co.,Ltd. All rights reserved.
 *
 * File name   : util.c
 * Author      : WANG Chao/w00238084
 * Date        : 2014/3/3
 * Version     : 1.0
 *
 * Description : socket及报文收发函数定义，读写配置文件函数定义
 *
 */

#include <linux/bio.h>
#include <linux/module.h>

#include "util.h"
#include "ctl_cmd.h"


/**
 * Description : 创建socket
 *
 * Parameters  : family  
 *               type    
 *               protocol
 *               sock - 返回创建好的socket结构指针
 * Return      : 成功 - 0
 *               失败 - <0错误码
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/3/3
 */
int im_socket_create(int family, int type,
                        int protocol,struct socket **sock)
{
    int ret = 0;

    ret = sock_create_kern(family, type, protocol, sock);

    if (0 == ret)
    {
        (*sock)->sk->sk_rcvtimeo = IM_SOCK_TIMEOUT_RECV;
        (*sock)->sk->sk_sndtimeo = IM_SOCK_TIMEOUT_SEND;
        (*sock)->sk->sk_reuse = 1;
        return ret;
    }
    else if (-EAFNOSUPPORT == ret)
    {
        IOMIRROR_ERR("sock_create_kern failed, invalid family=%d, err=%d.",
                    family, ret);
    }
    else if (-EINVAL == ret)
    {
        IOMIRROR_ERR("sock_create_kern failed, invalid type=%d, err=%d.",
                    type, ret);
    }
    else
    {
        IOMIRROR_ERR("sock_create_kern failed, errno=%d.", ret);
    }

    return ret;
}


/**
 * Description : 关闭socket
 *
 * Parameters  : sock - 待关闭socket的指针
 * Return      : void
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/3/3
 */
void im_socket_close(struct socket *sock)
{
    sock_release(sock);
}


/**
 * Description : bind
 *
 * Parameters  : sock   
 *               address
 *               addrlen
 * Return      : 成功 - 0
 *               失败 - <0错误码
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/3/3
 */
int im_socket_bind(struct socket *sock,
                        struct sockaddr *address, int addrlen)
{
    int ret = 0;

    ret = sock->ops->bind(sock, address, addrlen);

    if (0 == ret)
    {
        return 0;
    }
    else if (-EINVAL == ret)
    {
        IOMIRROR_ERR("bin failed, invalid addrlen=%d, err=%d.", addrlen, ret);
    }
    else if (-EAFNOSUPPORT == ret)
    {
        IOMIRROR_ERR("bin failed, EAFNOSUPPORT, err=%d.", ret);
    }
    else
    {
        IOMIRROR_ERR("bind failed, errno=%d.", ret);
    }

    return ret;
}


/**
 * Description : listen
 *
 * Parameters  : sock   
 *               backlog
 * Return      : 成功 - 0
 *               失败 - <0错误码
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/3/3
 */
int im_socket_listen(struct socket *sock, int backlog)
{
    int ret = 0;

    ret = sock->ops->listen(sock, backlog);

    if (0 == ret)
    {
        return 0;
    }
    else if (-EINVAL == ret)
    {
        IOMIRROR_ERR("listen failed, EINVAL, err=%d.", ret);
    }
    else if (-EOPNOTSUPP == ret)
    {
        IOMIRROR_ERR("listen failed, EOPNOTSUPP, err=%d.", ret);
    }
    else
    {
        IOMIRROR_ERR("listen failed, err=%d.", ret);
    }

    return ret;
}


/**
 * Description : accept
 *
 * Parameters  : sock   
 *               newsock
 *               flags  
 * Return      : 成功 - 0
 *               失败 - <0错误码
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/3/3
 */
int im_socket_accept(struct socket *sock,
                        struct socket **newsock, int flags)
{
    int ret = 0;
    struct sock* sk = sock->sk;

    ret = sock_create_lite(sk->sk_family, sk->sk_type, sk->sk_protocol, newsock);
    if (ret < 0)
    {
        IOMIRROR_ERR("sock_create_lite failed, err=%d.", ret);
        return ret;
    }

    ret = sock->ops->accept(sock, *newsock, flags);
    if (ret < 0)
    {
        IOMIRROR_ERR("sock->ops->accept failed, err=%d.", ret);
        sock_release(*newsock);
        *newsock = NULL;
        return ret;
    }

    (*newsock)->ops = sock->ops;
    __module_get((*newsock)->ops->owner);

    return 0;
}


/**
 * Description : connect
 *
 * Parameters  : sock   
 *               address
 *               addrlen
 *               flags  
 * Return      : 成功 - 0
 *               失败 - <0错误码
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/3/3
 */
int im_socket_connect(struct socket *sock,
                        struct sockaddr *address, int addrlen, int flags)
{
    return sock->ops->connect(sock, address, addrlen, flags);
}


/**
 * Description : getsockname
 *
 * Parameters  : sock   
 *               addr   
 *               addrlen
 * Return      : 成功 - 0
 *               失败 - <0错误码
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/3/3
 */
int im_getsockname(struct socket *sock,
                        struct sockaddr *addr, int *addrlen)
{
    return sock->ops->getname(sock, addr, addrlen, 0);
}


/**
 * Description : getpeername
 *
 * Parameters  : sock   
 *               addr   
 *               addrlen
 * Return      : 成功 - 0
 *               失败 - <0错误码
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/3/3
 */
int im_getpeername(struct socket *sock,
                        struct sockaddr *addr, int *addrlen)
{
    return sock->ops->getname(sock, addr, addrlen, 1);
}


/**
 * Description : socket数据收发
 *
 * Parameters  : sock
 *               send
 *               buf 
 *               size
 *               flag
 *               retry - 重试次数
 * Return      : 成功 - 0
 *               失败 - 超时-EAGAIN，其他错误-1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/3/3
 */
inline int im_sock_xmit(struct socket *sock, int send,
                                void *buf, size_t size, int flag, int retry)
{
    struct msghdr msg;
    struct kvec iov;

    int result = 0;

    do {
        iov.iov_base = buf;
        iov.iov_len = size;

        msg.msg_name = NULL;
        msg.msg_namelen = 0;
        msg.msg_control = NULL;
        msg.msg_controllen = 0;
        msg.msg_flags = flag | MSG_NOSIGNAL;

        sock->sk->sk_allocation = GFP_NOIO;

        if (send)
        {
            result = kernel_sendmsg(sock, &msg, &iov, 1, size);
        }
        else
        {
            result = kernel_recvmsg(sock, &msg, &iov, 1, size, 0);
        }

        if (unlikely(result <= 0))
        {
            if (-EAGAIN == result
                || -ETIMEDOUT == result
                || -EWOULDBLOCK == result)
            {
                retry--;
                if (likely(retry > 0))
                {
                    continue;
                }
                else
                {
                    return -EAGAIN;
                }
            }
            else if (0 == result)
            {
                IOMIRROR_ERR("xmit failed, socket closed, retry=%d.", retry);
                return -1;
            }
            else
            {
                IOMIRROR_ERR("xmit failed, err=%d, retry=%d.", result, retry);
                return -1;
            }
        }

        size -= (size_t)result;
        buf += result;
    } while(size > 0);

    return 0;
}


/**
 * Description : 数据报文体发送函数
 *
 * Parameters  : sock 
 *               bvec 
 *               flags
 * Return      : 成功 - 0
 *               失败 - 超时-EAGAIN，其他错误-1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/3/3
 */
static inline int im_sock_send_bvec(struct socket *sock,
                                        struct bio_vec *bvec, int flags)
{
    int result = 0;
    result = im_sock_xmit(sock, 1, page_address(bvec->bv_page) + bvec->bv_offset,
                          bvec->bv_len, flags, IM_SOCK_MAX_RETRY);
    return result;
}

void change_to_dpp_data(struct im_cmd_header *cmd_header, DPP_HEADER *header, void *data)
{
    PDPP_DATASET_START p_dataset;
    PDPP_RESYNCSET_START p_resyncset;
	PDPP_VOL_VEC_ENTRY p_vol_vec;
    PDPP_SESSION_LOGIN p_session_login;
	PDPP_SESSION_LOGIN_AUX p_session_login_aux;
    PDPP_DATA p_data;
    PDPP_ATTENTION p_attention;
    PDPP_ATTENTION_PAYLOAD_ACTIVITY p_attention_payload_activity;
    PDPP_FLUSH p_flush;

    header->magic = cpu_to_be32(DPP_MAGIC);
    header->sequence_num = cpu_to_be64(cmd_header->request_id);
    header->cmd_type = cpu_to_be16(cmd_header->cmd_type & (~DPP_CBT_DATASET_FLAG));
    header->flags = cpu_to_be16(DPP_FLAG_SOURCE_IOMIRROR | DPP_FLAG_DEST_SOMA);
    header->body_len = 0;

    if (DPP_TYPE_DATASET_START == cmd_header->cmd_type)
    {
        //header->flags |= DPP_FLAG_STATE_NORMAL;
        header->body_len = cpu_to_be32(sizeof(DPP_DATASET_START));
        p_dataset = data;

        memcpy(p_dataset->vm_id, cmd_header->host_id, VM_ID_LEN);
        p_dataset->dataset_id = cpu_to_be64(cmd_header->request_id);
        p_dataset->dpp_type = cpu_to_be32(DPP_DATASET_TYPE_NORMAL);
        
        IOMIRROR_INFO("Normal DATASet magic 0x%X, cmd_type 0x%X, flag 0x%X, request id 0x%llX, body len 0x%X, reserved 0x%X, dataset id 0x%llX, dpptype %X.", 
                header->magic, header->cmd_type, header->flags, header->sequence_num, 
                header->body_len, header->reserved, p_dataset->dataset_id, p_dataset->dpp_type);
    }
    else if ((DPP_TYPE_DATASET_START | DPP_CBT_DATASET_FLAG) == cmd_header->cmd_type)
    {
        //header->flags |= DPP_FLAG_STATE_CBT;
        header->body_len = cpu_to_be32(sizeof(DPP_DATASET_START));
        p_dataset = data;

        memcpy(p_dataset->vm_id, cmd_header->host_id, VM_ID_LEN);
        p_dataset->dataset_id = cpu_to_be64(cmd_header->request_id);
        p_dataset->dpp_type = cpu_to_be32(DPP_DATASET_TYPE_CBT);

        IOMIRROR_INFO("CBT DATASet magic 0x%X, cmd_type 0x%X, flag 0x%X, request id 0x%llX, body len 0x%X, reserved 0x%X, dataset id 0x%llX, dpptype %X.", 
                header->magic, header->cmd_type, header->flags, header->sequence_num, 
                header->body_len, header->reserved, p_dataset->dataset_id, p_dataset->dpp_type);
    }
    else if (DPP_TYPE_RESYNCSET_START == cmd_header->cmd_type)
    {
        //header->flags |= DPP_FLAG_STATE_RESYNC;
        header->body_len = cpu_to_be32(sizeof(DPP_RESYNCSET_START) + sizeof(DPP_VOL_VEC_ENTRY));
        p_resyncset = data;
        p_vol_vec = (PDPP_VOL_VEC_ENTRY)&p_resyncset[1];

        memcpy(p_resyncset->vm_id, cmd_header->host_id, VM_ID_LEN);
        p_resyncset->resyncset_id = cpu_to_be64(cmd_header->request_id);
        p_resyncset->num_vols = cpu_to_be32(1);

        memcpy(p_vol_vec->vol_id, cmd_header->vol_id, VOL_ID_LEN);
        p_vol_vec->vol_offset = cpu_to_be64(cmd_header->data_offset);
        p_vol_vec->seg_size = cpu_to_be64(cmd_header->data_size_bytes);
        IOMIRROR_INFO("Resync DATASet magic 0x%X, cmd_type 0x%X, flag 0x%X, request id 0x%llX, body len 0x%X, reserved 0x%X, dataset id 0x%llX, "
                       "vol offset %llu, vol size %llu.", 
                header->magic, header->cmd_type, header->flags, header->sequence_num, 
                header->body_len, header->reserved, p_resyncset->resyncset_id, p_vol_vec->vol_offset, p_vol_vec->seg_size);

    }
    else if (DPP_TYPE_SESSION_LOGIN == cmd_header->cmd_type)
    {
        header->body_len = cpu_to_be32(sizeof(DPP_SESSION_LOGIN) + sizeof(DPP_SESSION_LOGIN_AUX));
        p_session_login = data;
        p_session_login_aux = (PDPP_SESSION_LOGIN_AUX)&p_session_login[1];

        p_session_login->version = 1;
        memcpy(p_session_login->vm_id, cmd_header->host_id, VM_ID_LEN);
        p_session_login->aux_login_len = cpu_to_be16(sizeof(DPP_SESSION_LOGIN_AUX));
		p_session_login_aux->max_dataset_size = cpu_to_be64(cmd_header->data_offset);
        p_session_login_aux->dataset_id_sent = cpu_to_be64(cmd_header->request_id);
        p_session_login_aux->dataset_id_done = cpu_to_be64(cmd_header->data_size_bytes);
        memcpy(p_session_login_aux->token_id, cmd_header->oma_id, VM_ID_LEN); // copy tokenID from cmd header omd_id[VM_ID_LEN]
        IOMIRROR_INFO("Session login, dataset_id_set = %llu, dataset_id_done = %llu.", cmd_header->request_id, cmd_header->data_size_bytes);
    }
    else if (DPP_TYPE_DATA == cmd_header->cmd_type)
    {
        header->body_len = cpu_to_be32(sizeof(DPP_DATA) + cmd_header->data_size_bytes);
        p_data = data;

        memcpy(p_data->vol_id, cmd_header->vol_id, VOL_ID_LEN);
        p_data->vol_offset = cpu_to_be64(cmd_header->data_offset * IM_SECTOR_SIZE);
        p_data->data_size = cpu_to_be32(cmd_header->data_size_bytes);
    }
    else if (DPP_TYPE_FLUSH == cmd_header->cmd_type)
    {
        header->body_len = cpu_to_be32(sizeof(DPP_FLUSH));
        p_flush = data;
        memcpy(p_flush->vm_id, cmd_header->host_id, VM_ID_LEN);
        IOMIRROR_INFO("Flush Dataset magic 0x%X, cmd_type 0x%X, flag 0x%X, request id 0x%llX, body len 0x%X", 
                header->magic, header->cmd_type, header->flags, header->sequence_num, header->body_len);
    }
    else if (DPP_TYPE_ATTENTION == cmd_header->cmd_type)
    {
        p_attention = data;
        p_attention->payload_len = 0;
        p_attention->operation = cpu_to_be32(cmd_header->ack_result);

        switch (cmd_header->ack_result)
        {
            case DPP_ATTENTION_OPERATION_ALERT:
                IOMIRROR_WARN("DPP_ATTENTION_OPERATION_ALERT is not support now.");
                break;
            case DPP_ATTENTION_OPERATION_ACTIVITY:
                p_attention_payload_activity = (PDPP_ATTENTION_PAYLOAD_ACTIVITY)&p_attention[1];
                p_attention_payload_activity->cbt_backlog = cpu_to_be64(cmd_header->data_size_bytes);
                p_attention_payload_activity->resync_remaining = cpu_to_be64(cmd_header->data_offset);
                p_attention->payload_len = cpu_to_be32(sizeof(DPP_ATTENTION_PAYLOAD_ACTIVITY));
                break;
            case DPP_ATTENTION_OPERATION_DISCOVERY:
                IOMIRROR_WARN("DPP_ATTENTION_OPERATION_DISCOVERY is not support now.");
                break;
            default:
                IOMIRROR_ERR("unkownn DPP_TYPE_ATTENTION opertion %d.", cmd_header->ack_result);
                break;
        }

        header->body_len = cpu_to_be32(sizeof(DPP_ATTENTION) + be32_to_cpu(p_attention->payload_len));
        memcpy(p_attention->vm_id, cmd_header->host_id, VM_ID_LEN);
    
    }
}


/**
 * Description : 报文发送函数
 *
 * Parameters  : sock
 *               cmd - 待发送报文
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/3/3
 */
int im_cmd_send(struct socket *sock, struct im_cmd *cmd)
{
    int i = 0;
    int ret = 0;
    int flags = MSG_MORE;
    struct bio_vec *bvec = NULL;
    char data[256] = {0, };
    DPP_HEADER header;

    if (unlikely(0 == cmd->header.data_size_bytes))
    {
        flags = 0;
    }

    change_to_dpp_data(&(cmd->header), &header, &data);
    ret = im_sock_xmit(sock, 1, &header,
                       sizeof(DPP_HEADER), flags, IM_SOCK_MAX_RETRY);
    if (unlikely(ret < 0))
    {
        IOMIRROR_ERR("im_sock_xmit sned cmd header failed.");
        return -1;
    }

    flags = MSG_MORE;
    if (0 != header.body_len)
    {
        ret = im_sock_xmit(sock, 1, data, (DPP_TYPE_DATA == cmd->header.cmd_type)?
                           sizeof(DPP_DATA) : be32_to_cpu(header.body_len), 0, IM_SOCK_MAX_RETRY);
        if (unlikely(ret < 0))
        {
            IOMIRROR_ERR("im_sock_xmit cmd->buf failed.");
            return -1;
        }
    }

    bvec = cmd->bvl;
    if (likely(NULL != bvec))
    {
        for (i = 0; i < cmd->vcnt; i++)
        {
            if (i == cmd->vcnt - 1)
            {
                flags = 0;
            }

            ret = im_sock_send_bvec(sock, bvec, flags);
            if (unlikely(ret < 0))
            {
                IOMIRROR_ERR("im_sock_send_bvec failed.");
                return -1;
            }

            bvec++;
        }
    }

    return 0;
}


/**
 * Description : 报文接收函数，
 *
 * Parameters  : sock
 *               cmd - 待接收报文
 * Return      : 成功 - 0
 *               失败 - 超时-EAGAIN，其他错误-1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/3/3
 */
int im_cmd_recv(struct socket *sock, struct im_cmd *cmd)
{
    int body_len, ret = 0;
    unsigned int flags = 0;
    char data[256] = {0, };
    DPP_HEADER header;
    PDPP_DATASET_DONE p_dataset = (PDPP_DATASET_DONE)data;
    PDPP_CREDIT_BUFFER p_credit_buffer = (PDPP_CREDIT_BUFFER)data;
    PDPP_SESSION_LOGIN p_session_login = (PDPP_SESSION_LOGIN)data;
    PDPP_SESSION_LOGIN_ACK_AUX p_session_login_ack_aux = (PDPP_SESSION_LOGIN_ACK_AUX)(data + sizeof(DPP_SESSION_LOGIN));

    cmd->bvl  = NULL;
    cmd->vcnt = 0;
    cmd->buf  = NULL;
    cmd->private = NULL;

    ret = im_sock_xmit(sock, 0, &header,
                        sizeof(DPP_HEADER), MSG_WAITALL, 1);
    if (unlikely(ret < 0))
    {
        return ret;
    }

    memset(&(cmd->header), 0, sizeof(struct im_cmd_header));
    cmd->header.magic = (be32_to_cpu(header.magic) == DPP_MAGIC)? IM_MAGIC : 0;
    cmd->header.cmd_type = be16_to_cpu(header.cmd_type);
    body_len = be32_to_cpu(header.body_len);
    flags = be16_to_cpu(header.flags);

    if (body_len == 0)
    {
        return 0;
    }
    else if (body_len > sizeof(data))
    {
        IOMIRROR_ERR("lenth of DPP body %d is too large.", body_len);
        return -1;
    }

    ret = im_sock_xmit(sock, 0, &data, body_len, MSG_WAITALL, 1);
    if (unlikely(ret < 0))
    {
        return ret;
    }

    if (DPP_TYPE_DATASET_DONE == cmd->header.cmd_type)
    {
        if ((flags & 0x8000) != 0) {
            IOMIRROR_ERR("the flags of received DPP_TYPE_DATASET_DONE is 0x%x.", flags);
            return -2; // iomirror will flush and enter reconnect state, then delay to send data
        }
        memcpy(cmd->header.host_id, p_dataset->vm_id, VM_ID_LEN);
        cmd->header.request_id = be64_to_cpu(p_dataset->dataset_id);
    }
    else if (DPP_TYPE_RESYNCSET_DONE == cmd->header.cmd_type)
    {
        if ((flags & 0x8000) != 0) {
            IOMIRROR_ERR("the flags of received DPP_TYPE_RESYNCSET_DONE is 0x%x.", flags);
            return -2; // iomirror will flush and enter reconnect state, then delay to send data
        }
        memcpy(cmd->header.host_id, p_dataset->vm_id, VM_ID_LEN);
        cmd->header.request_id = be64_to_cpu(p_dataset->dataset_id);
    }
    else if (DPP_TYPE_CREDIT == cmd->header.cmd_type)
    {
        cmd->header.data_size_bytes = be32_to_cpu(p_credit_buffer->buf_credit);
    }
    else if (DPP_TYPE_SESSION_LOGIN_ACK == cmd->header.cmd_type)
    {
        if (GET_DPP_ACK_ERROR_FLAG(flags) == DPP_ACK_ERROR 
            && GET_DPP_ACK_ERROR_TYPE(flags) == DPP_ACK_TOKEN_ID_ERROR)
        {
            IOMIRROR_ERR("dpp ack error type is 0x%x.", DPP_ACK_TOKEN_ID_ERROR);
            cmd->header.ack_result = DPP_ACK_TOKEN_ID_ERROR;
            return -1;
        }
        memcpy(cmd->header.host_id, p_session_login->vm_id, VM_ID_LEN);
        cmd->header.data_size_bytes = be32_to_cpu(p_session_login_ack_aux->buf_credit);
        cmd->header.data_offset = be64_to_cpu(p_session_login_ack_aux->max_dataset_size); // use data_offset to pass max_dataset_size
        cmd->header.request_id = be64_to_cpu(header.sequence_num);
        IOMIRROR_INFO("got DPP_TYPE_SESSION_LOGIN_ACK cmd, "
              "max_dataset_size=%llu, dataset_id_recv=%llu, dataset_id_done=%llu, buf_credit=%llu.",
              cmd->header.data_offset, be64_to_cpu(p_session_login_ack_aux->dataset_id_sent),
              be64_to_cpu(p_session_login_ack_aux->dataset_id_done), cmd->header.data_size_bytes);

        cmd->buf = vmalloc(DPP_SESSION_LOGIN_ACK_AUX_LEN);
        if (unlikely(NULL == cmd->buf))
        {
            IOMIRROR_ERR("vmalloc for DPP_SESSION_LOGIN_ACK_AUX failed.");
            return -1;
        }
        memcpy(cmd->buf, data + sizeof(DPP_SESSION_LOGIN), DPP_SESSION_LOGIN_ACK_AUX_LEN);
    }

    if (GET_DPP_ACK_ERROR_FLAG(flags) == DPP_ACK_ERROR) {
        cmd->header.ack_result = (char)GET_DPP_ACK_ERROR_TYPE(flags);
        IOMIRROR_ERR("receive dpp protocol failed, error code is 0x%x.", cmd->header.ack_result);
        return -1;
    }

    return 0;
}


/**
 * Description : 读取文件
 *
 * Parameters  : path - 待读取的文件路径
 *               buf - 保存文件内容的buf
 *               size - 读取大小
 * Return      : 成功 - 0
 *               失败 - -1
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
static int im_file_read(const char *path, char *buf, size_t size)
{
    int ret = 0;
    mm_segment_t old_fs;
    struct file *file = NULL;

    file = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(file))
    {
        ret = -1;
        IOMIRROR_ERR("filp_open config file failed.");
        goto out;
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    ret = vfs_read(file, buf, size, &file->f_pos);
    if (size != ret)
    {
        IOMIRROR_ERR("vfs_read file failed, ret=%d.", ret);
        ret = -1;
    }
    set_fs(old_fs);
    filp_close(file, NULL);  
    file = NULL;

out:
    return ret;
}


/**
 * Description : 读取配置文件，
 *               调用者须主动释放返回的im_config_pg指针
 *
 * Parameters  : void
 * Return      : 成功，返回im_config_pg结构体指针
 *               失败，返回NULL
 *
 * Author      : WANG Chao/w00238084
 * Date        : 2014/5/17
 */
struct im_config_pg *im_config_read(void)
{
    int ret = 0;
    size_t size = 0;
    struct im_config_pg conf_tmp;
    struct im_config_pg *conf = NULL;

    ret = im_file_read(IM_CONFIG_PATH, (char *)(&conf_tmp), sizeof(conf_tmp));
    if (ret < 0)
    {
        IOMIRROR_ERR("im_file_read failed.");
        goto out;
    }

    size = sizeof(struct im_config_pg) 
           + sizeof(ProtectVol) * conf_tmp.vol_num;
    conf = (struct im_config_pg *)kzalloc(size, GFP_KERNEL);
    if (NULL == conf)
    {
        IOMIRROR_ERR("kzalloc for conf failed.");
        goto out;
    }

    ret = im_file_read(IM_CONFIG_PATH, (char *)conf, size);
    if (ret < 0)
    {
        IOMIRROR_ERR("im_file_read failed.");
        kfree(conf);
        conf = NULL;
        goto out;
    }

out:
    return conf;
}

/**
 * Description : 根据IP数字解析字符串
 *
 * Parameters  : ipInt -- ip integer
 				ipStr -- ip string
 				ipStrlen -- ip string length
 * Return      : void
 *
 * Author      : wangguitao 00510599
 * Date        : 2019/7/11
 */
void parse_ip_str(uint32_t ipInt, char *ipStr, uint32_t ipStrlen)
{
    if (ipStr == NULL || ipStrlen < IP_STRING_LEN)
    {
        IOMIRROR_ERR("ipStr is null or ipStr len letter than %d.", IP_STRING_LEN);
        return;
    }

    snprintf(ipStr, ipStrlen, "%u.%u.%u.%u", ipInt >> 24, 
                                             (ipInt & 0x00FF0000) >> 16,
                                             (ipInt & 0x0000FF00) >> 8,
                                             (ipInt & 0x000000FF));
    ipStr[ipStrlen - 1] = '\0';
}

inline uint32_t make_log_lower_bound(uint64_t value)
{
    uint32_t ret = 0;
    while (value >= 2)
    {
        ret++;
        value = value >> 1;
    }

    return ret;
}

inline uint32_t make_log_upper_bound(uint64_t value)
{
    uint32_t ret = 0;
    while (value != 0)
    {
        ret++;
        value = value >> 1;
    }

    return ret;
}
// 计算2^x的值 
inline uint64_t make_power_2_value(uint64_t x)
{
    uint64_t ret = 1;
    
    if (x > 64) { // 2^64会超过uint64_t的最大值
        return 0;
    }

    while (x != 0) {
        ret = ret << 1;
        x--;
    }

    return ret;
}
/**
 * Description : 将告警信息添加到链表中
 *
 * Parameters  : p_alarm_list : 所有告警信息的头部，
 *               alarm_item ： 待添加到list的告警信息
 * Return      : 无
 *
 * Author      : z00455045
 * Date        : 2019/12/19
 */
void im_add_alarm_list(PALARM_LIST p_alarm_list, PALARM_ITEM alarm_item)
{
    if (!p_alarm_list || !alarm_item) {
        IOMIRROR_ERR("parameters are error when add alarm into list.");
        return;
    }

    p_alarm_list->alarm_total = (p_alarm_list->alarm_total < 0 ? 0 : p_alarm_list->alarm_total);

    if (p_alarm_list->alarm_total <= 0) {
        p_alarm_list->p_first_alarm_item = alarm_item;
        p_alarm_list->p_last_alarm_item = alarm_item;
        goto ALARM_TOTAL;
    }

    p_alarm_list->p_last_alarm_item->next = alarm_item;
    p_alarm_list->p_last_alarm_item = alarm_item;

ALARM_TOTAL:
    p_alarm_list->alarm_total += 1;
    IOMIRROR_INFO("add alarm info which error code is 0x%llx success", p_alarm_list->p_last_alarm_item->error_code);
}

/**
 * Description : 从list中取出指定数量的告警, 取出之后就从list中删除， 最后返回的个数不大于MAX_ALARM_NUM_ONCE_REPORT
 *
 * Parameters  : p_alarm_list : 告警list header，
 *               p_item_array : 保存报警信息的内存
 *               items_size : 内存大小
 * Return      : > 0 : return the number of items; -1 : failed, parameters is error; 0 : no alarm info in kernel
 *
 * Author      : z00455045
 * Date        : 2019/12/19
 */
int im_get_alarms(PALARM_LIST p_alarm_list, PALARM_ITEM p_item_array, int items_size)
{
    int n = 0;
    PALARM_ITEM q = NULL, p = NULL;
    if (p_item_array == NULL || p_alarm_list == NULL || items_size <= 0 ) {
        return -1;
    }

    if (p_alarm_list->alarm_total == 0) {
        return 0;
    }
    q = p = p_alarm_list->p_first_alarm_item;
    items_size = (items_size > MAX_ALARM_NUM_ONCE_REPORT ? MAX_ALARM_NUM_ONCE_REPORT : items_size);
    while (p != NULL && n < items_size) {
        memcpy(&p_item_array[n], p, sizeof(ALARM_ITEM));
        p = p->next;
        kfree(q); // im_create_and_init_alarm_item()中申请内存的方式要对应
        q = p;
        n++;
        p_alarm_list->alarm_total--;
    }
    p_alarm_list->p_first_alarm_item = p;

    if (p_alarm_list->alarm_total == 0) {
        p_alarm_list->p_last_alarm_item = NULL;
        p_alarm_list->p_first_alarm_item = NULL;
    }

    return n;
}

void im_init_alarm_list(PALARM_LIST p_alarm_list)
{
    if (p_alarm_list == NULL) {
        return;
    }

    p_alarm_list->p_first_alarm_item = NULL;
    p_alarm_list->p_last_alarm_item = NULL;
    p_alarm_list->alarm_total = 0;
}

void im_init_alarm_item(PALARM_ITEM p_alarm_item)
{
    p_alarm_item->error_code = 0;
    p_alarm_item->next = NULL;
    p_alarm_item->info[KERNEL_ALARM_INFO_DESC_LEN - 1] = '\0';
}

PALARM_ITEM im_create_and_init_alarm_item(void)
{
    PALARM_ITEM p = (PALARM_ITEM)kzalloc(sizeof(ALARM_ITEM), GFP_KERNEL);
    if (unlikely(p == NULL)) {
        IOMIRROR_ERR("create alarm item failed due to vmalloc failed.");
        return NULL;
    }
    im_init_alarm_item(p);
    return p;
}
/**
 * Description : 停止保护时，可能仍然有一些告警在内存中，停止保护前需要先释放这些内存再
 *
 * Parameters  : p_alarm_list : 告警list header，
 * 
 * Return      : 无
 *
 * Author      : z00455045
 * Date        : 2019/12/23
 */
void im_pg_free_alarms(PALARM_LIST p_alarm_list)
{
    PALARM_ITEM p = NULL, q = NULL;
    if (!p_alarm_list) {
        return;
    }

    p = q = p_alarm_list->p_first_alarm_item;
    while (p != NULL) {
        p = p->next;
        if (q) {
            kfree(q);
        }
        q = p;
    }

    im_init_alarm_list(p_alarm_list);
}

/**
 * Description : 判断输入的token是否全为0， 全为0表示无效（upgrade情况）
 *
 * Parameters  : token：字符数组， token_len: 数组长度
 *             
 * Return      : true : token无效， 
 *
 * Author      : z00455045
 * Date        : 2020/3/17
 */
inline bool im_is_token_valid(const char* token, const int token_len)
{
    const char uuid_zeros[TOKEN_ID_LEN] = {0};

    if (token == NULL || token_len != TOKEN_ID_LEN) {
        return false;
    }

    if (memcmp(token, uuid_zeros, TOKEN_ID_LEN) != 0) {
        return true;
    }

    return false;
}