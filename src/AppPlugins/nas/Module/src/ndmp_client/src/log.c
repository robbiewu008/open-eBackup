/*                               -*- Mode: C -*- 
 * log.c
 * 
 * Description     : Log request handlers.
 * 
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

/*
 * The majority of this code was taken from the NDMP SDK client toolkit.
 */

#include "ndmp_common.h"
#include "ndmpc.h"
#include "queue.h"
#include "securec.h"
#include "log/Log.h"

extern struct cmd_line_opts opts;
/*
 * logLog
 *   log_log request message handler.
 *
 * Parameters:
 *   connection (input) - NDMP connection.
 *   msginfo    (input) - message.
 *
 * Returns:
 *   void
 */
void
logLog(NdmpConnection	connection,
       void*			body)
{
    MsgQueue*    backend_queue = (MsgQueue*)ndmpGetClientData(connection);
    MsgData     backend_msg;

    ndmp_log_log_request*	request =
        (ndmp_log_log_request*)body;

    (void)sprintf_s(backend_msg.text, sizeof(backend_msg.text), "%s", request->entry);
    backend_msg.message = _NDMP_LOG_LOG;
    backend_msg.connection  = connection;
    enqueue (backend_queue, &backend_msg);
}

/*
 * logDebug
 *   log_debug request message handler.
 *
 * Parameters:
 *   connection (input) - NDMP connection.
 *   msginfo    (input) - message.
 *
 * Returns:
 *   void
 */
void
logDebug(NdmpConnection	connection,
		 void*			body)
{
	ndmp_log_debug_request*	request =
		(ndmp_log_debug_request *)body;

    DBGLOG("Debug: %s.", request->message);
}

/*
 * logFile
 *   log_file request message handler.
 *
 * Parameters:
 *   connection (input) - NDMP connection.
 *   msginfo    (input) - message.
 *
 * Returns:
 *   void
 */
void
logFile(NdmpConnection	connection,
		void*			body)
{
	ndmp_log_file_request_v4*	request =
		(ndmp_log_file_request_v4 *)body;

    DBGLOG("File recovered: %s, status: %u.", request->name, request->recovery_status);
}

void
logMessage(NdmpConnection	connection,
		void*			body)
{
    MsgQueue*    backend_queue = (MsgQueue*)ndmpGetClientData(connection);
    MsgData     backend_msg;
    
    ndmp_log_message_request_v4*	request =
		(ndmp_log_message_request_v4 *)body;

    (void)sprintf_s(backend_msg.text, sizeof(backend_msg.text), "%s", request->entry);
    backend_msg.message = NDMP_LOG_MESSAGE;
    backend_msg.connection  = connection;
    enqueue (backend_queue, &backend_msg);

	return;
}

/*
 * End of file	:	log.c
 */
