/*                               -*- Mode: C -*- 
 * notify.c
 * 
 * Description     : Notify request handlers.
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
 * The majority of this code is taken from the NDMP SDK client toolkit.
 */

#include "ndmp_common.h"
#include "queue.h"
#include "securec.h"

extern struct cmd_line_opts		opts;

/*
 * notifyConnected
 *   notify_connected request message handler.
 *
 * Parameters:
 *   connection (input) - NDMP connection.
 *   msginfo    (input) - message.
 *
 * Returns:
 *   void
 */
void
notifyConnected(NdmpConnection connection, void *body)
{
    ndmp_notify_connected_request*	request =
	    (ndmp_notify_connected_request *)body;
    MsgQueue	*backend_queue = (MsgQueue*)ndmpGetClientData(connection);
    MsgData		backend_msg;

	backend_msg.message = NDMP_NOTIFY_CONNECTION_STATUS;
	backend_msg.reason  = request->reason;
    
	backend_msg.connection  = connection;
	enqueue(backend_queue, &backend_msg);

	switch (request->reason)
	{
	    case NDMP_CONNECTED:
		(void)strcpy_s(backend_msg.text, sizeof(backend_msg.text), "CONNECT: Connection established.");
		break;

	    case NDMP_SHUTDOWN:
		(void)strcpy_s(backend_msg.text, sizeof(backend_msg.text), "CONNECT: Connection shutdown.");
		break;

	    case NDMP_REFUSED:
		(void)strcpy_s(backend_msg.text, sizeof(backend_msg.text), "CONNECT: Connection refused.");
		break;

	    default:
		(void)strcpy_s(backend_msg.text, sizeof(backend_msg.text), "CONNECT: Unknown error.");
		break;
	}

	backend_msg.message = NDMP_NOTIFY_CONNECTION_STATUS;
	backend_msg.reason  = request->reason;
	backend_msg.connection  = connection;
	enqueue(backend_queue, &backend_msg);
}

/*
 * notifyDataHalted
 *   notify_data_halted request message handler.
 *
 * Parameters:
 *   connection (input) - NDMP connection.
 *   msginfo    (input) - message.
 *
 * Returns:
 *   void
 */
void
notifyDataHalted(NdmpConnection connection, void *body)
{
    ndmp_notify_data_halted_request_v4*	request =
	    (ndmp_notify_data_halted_request_v4 *)body;
    
    MsgQueue	*backend_queue = (MsgQueue*)ndmpGetClientData(connection);
    MsgData		backend_msg;

    switch (request->reason)
    {
	case NDMP_DATA_HALT_NA:
	    (void)sprintf_s(backend_msg.text, sizeof(backend_msg.text) - 1, "NA");
	    break;

	case NDMP_DATA_HALT_SUCCESSFUL:
	    (void)sprintf_s(backend_msg.text, sizeof(backend_msg.text) - 1, "HALT: The operation was successful!");
	    break;

	case NDMP_DATA_HALT_ABORTED:
	    (void)sprintf_s(backend_msg.text, sizeof(backend_msg.text) - 1, "HALT: The operation was aborted!");
	    break;

	case NDMP_DATA_HALT_INTERNAL_ERROR:
	    (void)sprintf_s(backend_msg.text, sizeof(backend_msg.text) - 1, "HALT: Internal error!");
	    break;

	case NDMP_DATA_HALT_CONNECT_ERROR:
	    (void)sprintf_s(backend_msg.text, sizeof(backend_msg.text) - 1, "HALT: Connect error!");
	    break;

	default:
	    (void)sprintf_s(backend_msg.text, sizeof(backend_msg.text) - 1, "HALT: Unknown error.");
	    break;
    }

    backend_msg.message = NDMP_NOTIFY_DATA_HALTED;
    backend_msg.reason  = request->reason;
    backend_msg.connection  = connection;
    enqueue(backend_queue, &backend_msg);

    return;
}

/*
 * notifyDataRead
 *   notify_data_read request message handler.
 *
 * Parameters:
 *   connection (input) - NDMP connection.
 *   msginfo    (input) - message.
 *
 * Returns:
 *   void
 */
void
notifyDataRead(NdmpConnection connection, void *body)
{
    ndmp_notify_data_read_request *request =
		(ndmp_notify_data_read_request *)body;
	
    MsgData backend_msg;
    MsgQueue	*backend_queue = (MsgQueue*)ndmpGetClientData(connection);

	(void)sprintf_s(backend_msg.text, sizeof(backend_msg.text) - 1, "Received notify_data_read: "
				  "offset:%lld len:%lld\n",
		       (unsigned long long) quadToLongLong(request->offset),
		       (unsigned long long) quadToLongLong(request->length));
	
	backend_msg.message = NDMP_NOTIFY_DATA_READ;
	backend_msg.connection  = connection;
	enqueue(backend_queue, &backend_msg);
}

/*
 * End of file	:	notify.c
 */
