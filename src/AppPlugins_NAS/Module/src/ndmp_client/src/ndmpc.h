/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#ifndef _MODULE_NDMPC_H_
#define _MODULE_NDMPC_H_

#include "ndmp_common.h"

char *ndmpErrToStr(ndmp_error err);

int getCdb(int num, char **cdb, int *cdb_len, char **data, int *data_len);
void displayCdbReply(char status, u_long dataout_len, char *data, int data_len, char *sense, int sense_len);

NdmpMsgHandlerFunc	notifyConnected;
NdmpMsgHandlerFunc	notifyDataHalted;
NdmpMsgHandlerFunc	notifyDataRead;
NdmpMsgHandlerFunc	logLog;
NdmpMsgHandlerFunc	logDebug;
NdmpMsgHandlerFunc	logFile;
NdmpMsgHandlerFunc	logMessage;
NdmpMsgHandlerFunc	fileHistoryAddUnix;
NdmpMsgHandlerFunc	fileHistoryAddUnixNode;
NdmpMsgHandlerFunc	fileHistoryAddUnixDir;
NdmpMsgHandlerFunc	fileHistoryAddNode;
NdmpMsgHandlerFunc	fileHistoryAddDir;

#endif /* ifndef ndmpc_h_ */

/*
 * End of file	:	ndmpc.h
 */
