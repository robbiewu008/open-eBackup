/* 
 * Copyright (c) 1999 Network Appliance, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */ 

#include <malloc.h>
#include "queue.h"
#include "ndmp_common.h"
#include "securec.h"
#include "log/Log.h"
#include "comm.h"


void ReleaseFhAddNodes(ndmp_node_v3 *nodes, uint64_t nodeNum)
{
    for (int i = 0; i < nodeNum; ++i) {
        MODULE_FREE(nodes[i].stats.stats_val);
    }
    MODULE_FREE(nodes);
}

/* enqueue():
 *   Description:
 *      Queues up particular NDMP client requests for processing at
 *      the top level.  If head is NULL the list is assumed to be
 *      empty and head is changed to point to the new data element.
 *      
 *   Inputs:
 *      MsgQueue *head  - head of the message queue to put the message on
 *      MsgData  *data  - message data to put on the queue.
 *
 *   Outputs:
 *      MsgQueue *head  - set to the head of a new queue if the queue was
 *                        previously empty (head == NULL).
 *
 *   Returns:
 *      void
 */
extern void setSendAbortValue(bool value);

int BuildParamForFhAddNode(ndmp_fh_add_node_request_v4 *dstReq, ndmp_fh_add_node_request_v4 *srcReq)
{
    int len = srcReq->nodes.nodes_len;
    int ret = 0;
    uint64_t sucNum = 0;
    if (ret = memcpy_s(dstReq, sizeof(ndmp_fh_add_node_request_v4), srcReq, sizeof(ndmp_fh_add_node_request_v4)) != 0) {
        ERRLOG("memcpy failed!");
        return -1;
    }
 
    ndmp_node_v3 *dstNodes = NULL;
    ndmp_node_v3 *srcNodes = srcReq->nodes.nodes_val;
    dstNodes = (ndmp_node_v3 *)malloc(sizeof(ndmp_node_v3) * len);
    CHECK_NULL_POINTER_RETURN(dstNodes, -1);
 
    ret = memcpy_s(dstNodes, sizeof(ndmp_node_v3) * len, srcNodes, sizeof(ndmp_node_v3) * len);
    CHECK_RESULT_GOTO(ret, 0, ERR);

    for (int i = 0; i < len; i++) {
        dstNodes[i].node = srcNodes[i].node;
        dstNodes[i].fh_info = srcNodes[i].fh_info;
        dstNodes[i].stats.stats_len = srcNodes[i].stats.stats_len;
        dstNodes[i].stats.stats_val = (ndmp_file_stat_v3 *)malloc(sizeof(ndmp_file_stat_v3));
        CHECK_NULL_POINTER_GOTO(dstNodes[i].stats.stats_val, ERR);
 
        ret = memcpy_s(&(dstNodes[i].stats.stats_val[0]), sizeof(ndmp_file_stat_v3), &(srcNodes[i].stats.stats_val[0]),
            sizeof(ndmp_file_stat_v3));
        ++sucNum;
        CHECK_RESULT_GOTO(ret, 0, ERR); 
    }
 
    dstReq->nodes.nodes_val = dstNodes;
    return 0;
 
ERR:
    ReleaseFhAddNodes(dstNodes, sucNum);
    return ret;
}

void ReleaseParamForFhAddDir(ndmp_dir_v3 *dirs, uint64_t dirNum)
{
    if (dirs == NULL) {
        return;
    }
    for (int i = 0; i < dirNum; ++i) {
        MODULE_FREE(dirs[i].names.names_val[0].ndmp_file_name_v3_u.unix_name);
        MODULE_FREE(dirs[i].names.names_val);
    }
    MODULE_FREE(dirs);
}

int BuildParamForFhAddDir(ndmp_fh_add_dir_request_v4 *dstReq, ndmp_fh_add_dir_request_v4 *srcReq)
{
    int len = srcReq->dirs.dirs_len;
    uint64_t sucNum = 0;
 
    ndmp_dir_v3 *srcDirs = srcReq->dirs.dirs_val;
    int ret = memcpy_s(dstReq, sizeof(ndmp_fh_add_dir_request_v4), srcReq, sizeof(ndmp_fh_add_dir_request_v4));
    CHECK_RESULT_RETURN(ret, 0, "memcpy failed!");
 
    ndmp_dir_v3 *dirs = (ndmp_dir_v3 *)malloc(sizeof(ndmp_dir_v3) *  len);
    CHECK_NULL_POINTER_RETURN(dstReq->dirs.dirs_val, -1);
 
    ret = memcpy_s(dirs, sizeof(ndmp_dir_v3) * len, srcDirs, sizeof(ndmp_dir_v3) * len);
    CHECK_RESULT_GOTO(ret, 0, ERR);
 
    for (int i = 0; i < len; i++) {
        dirs[i].node = srcDirs[i].node;
        dirs[i].parent = srcDirs[i].parent;
        dirs[i].names.names_len = srcDirs[i].names.names_len;
        int nums = srcDirs[i].names.names_len;
        dirs[i].names.names_val = (ndmp_file_name_v3 *)malloc(sizeof(ndmp_file_name_v3) * nums);
        CHECK_NULL_POINTER_GOTO(dirs[i].names.names_val, ERR);

        int nameLen = strlen(srcDirs[i].names.names_val[0].ndmp_file_name_v3_u.unix_name);
        char *unix_name = (char *)malloc(nameLen + 1);
        CHECK_NULL_POINTER_GOTO(unix_name, ERR);
        ++sucNum;
 
        (void)memset_s(unix_name, nameLen + 1, 0, nameLen + 1);
 
        ret = memcpy_s(&dirs[i].names.names_val[0], sizeof(ndmp_file_name_v3), &srcDirs[i].names.names_val[0],
                        sizeof(ndmp_file_name_v3)); 
        CHECK_RESULT_GOTO(ret, 0, ERR);
 
        ret = strcpy_s(unix_name, nameLen + 1, srcDirs[i].names.names_val[0].ndmp_file_name_v3_u.unix_name);
        CHECK_RESULT_GOTO(ret, 0, ERR);
    
        dirs[i].names.names_val[0].ndmp_file_name_v3_u.unix_name = unix_name;
        DBGLOG("parse one file dst name:%s, src name:%s, unix name:%s, strlen(%d), arr:%p",
                dirs[i].names.names_val[0].ndmp_file_name_v3_u.unix_name,
                srcDirs[i].names.names_val[0].ndmp_file_name_v3_u.unix_name, unix_name, nameLen,
                dirs[i].names.names_val[0].ndmp_file_name_v3_u.unix_name);
    }
 
    dstReq->dirs.dirs_val = dirs;
    return 0;
 
ERR:
    ReleaseParamForFhAddDir(dirs, sucNum);
    return -1;
}
 
/*
    备份或者回复过程中接收到的生产端发送的request入队，只有NDMP_FH_ADD_NODE和NDMP_FH_ADD_DIR这俩个请求会将request的内容也存入。
    这两个请求如果解析失败需要报错，因为会影响文件级恢复。
*/
void
enqueue(MsgQueue* head, MsgData* data)
{
    int rc = 0;
    MsgData* newdata;

    newdata = (MsgData*)malloc(sizeof(MsgData));

    if (newdata == NULL)
    {
        exit (1);
    }

    newdata->message = data->message;
    newdata->reason = data->reason;
    strncpy_s(newdata->text, 512, data->text, strlen(data->text));
    newdata->connection = data->connection;
    newdata->next = NULL;

    if (newdata->message == NDMP_FH_ADD_NODE) {
        ndmp_fh_add_node_request_v4 *req = (ndmp_fh_add_node_request_v4 *)data->body;
        ndmp_fh_add_node_request_v4 *newDataReq = (ndmp_fh_add_node_request_v4 *)malloc(sizeof(ndmp_fh_add_node_request_v4));
        CHECK_NULL_POINTER_GOTO(newDataReq, ERR);
 
        rc = BuildParamForFhAddNode(newDataReq, req);
        CHECK_RESULT_GOTO(rc, 0, ERR);
 
        newdata->body = (void *)newDataReq;
    } else if (newdata->message == NDMP_FH_ADD_DIR) {
        ndmp_fh_add_dir_request_v4 *req = (ndmp_fh_add_dir_request_v4 *)data->body;
        ndmp_fh_add_dir_request_v4 *newDataReq = (ndmp_fh_add_dir_request_v4 *)malloc(sizeof(ndmp_fh_add_dir_request_v4));
        CHECK_NULL_POINTER_GOTO(newDataReq, ERR);
 
        rc = BuildParamForFhAddDir(newDataReq, req);
        if (rc != 0) {
            ERRLOG("build param failed! rc:%d", rc);
            MODULE_FREE(newDataReq);
            goto ERR;
        }
 
        newdata->body = (void *)newDataReq;
    }

    if (*head == NULL)
    {
        *head = newdata;
    }
    else
    {
        newdata->next = *head;
        *head = newdata;
    }

    return;
 
ERR:
    MODULE_FREE(newdata);
    setSendAbortValue(false);
    return;
}



/* dequeue():
 *   Description:
 *      Removes a data element from the queue.
 *      
 *   Inputs:
 *      MsgQueue *head  - head of the message queue to take the message from
 *
 *   Outputs:
 *      MsgData  *data  - data element to store information in
 *
 *   Returns:
 *      number of messages that were in the queue BEFORE this one was removed.
 */

int
dequeue(MsgQueue *head, MsgData *data)
{
    int element_count = 0;
    MsgData *cur_data;

    
    for (cur_data = *head;
	 cur_data && cur_data->next && cur_data->next->next;
	 cur_data = cur_data->next, element_count++);


    /* this only occurs when there are no elements in the list */
    if (cur_data == NULL)
	return 0;
    
    /* This only occurs if it's the only element in the list */
    if (cur_data->next == NULL) {
    memcpy_s(data, sizeof(MsgData), cur_data, sizeof(MsgData));
	*head = NULL;
	free(cur_data);
	return 1;
    }

    /* if there are two or more elements, cur_data points to the
     * second to last element.
     */
    if (cur_data->next->next == NULL) {
    memcpy_s(data, sizeof(MsgData), cur_data->next, sizeof(MsgData));
	free(cur_data->next);
	cur_data->next = NULL;
	return element_count+1;
    }

    /* this should never happen */
    return -1;
}


/* peek():
 *   Description:
 *      Same as dequeue but the message is left in the queue.  This is
 *      useful for determing if there are messages waiting to be processed
 *      and how many.
 *      
 *   Inputs:
 *      MsgQueue *head  - head of the message queue to take the message from
 *
 *   Outputs:
 *      MsgData  *data  - data element with head of the message queue
 *
 *   Returns:
 *      number of messages that were in the queue BEFORE this one was removed.
 */

int peek(MsgQueue *head, MsgData *data)
{
    MsgData *cur_data;
    int element_count = 0;

    for (cur_data = *head;
	 cur_data && cur_data->next; 
	 cur_data = cur_data->next, element_count++);

    if (cur_data == NULL)
	return 0;

    if (cur_data ->next == NULL) {
        memcpy_s(data, sizeof(MsgData), cur_data->next, sizeof(MsgData));
        return element_count;
    }

    /* this should never happen */
    return -1;
}




