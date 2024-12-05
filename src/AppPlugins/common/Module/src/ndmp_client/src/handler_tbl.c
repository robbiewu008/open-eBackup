/*                               -*- Mode: C -*- 
 * handler_tbl.c
 * 
 * Description     : NDMP request handler table.
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

NdmpMsgHandler ndmp_msg_handler_tbl[] =
{
	{
		NDMP_CONNECT_OPEN, NDMPVER,
		0,
		AUTH_NOT_REQUIRED,
		XDR_AND_SIZE(ndmp_connect_open_request),
		XDR_AND_SIZE(ndmp_connect_open_reply)
	},
	{
		NDMP_CONNECT_CLIENT_AUTH, NDMPVER,
		0,
		AUTH_NOT_REQUIRED,
		XDR_AND_SIZE(ndmp_connect_client_auth_request),
		XDR_AND_SIZE(ndmp_connect_client_auth_reply)
	},
	{
		NDMP_CONNECT_CLOSE, NDMPVER,
		0,
		AUTH_NOT_REQUIRED,
		0, 0,
		0, 0
	},
	{
		NDMP_CONFIG_GET_HOST_INFO, NDMPVER,
		0,
		AUTH_NOT_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_config_get_host_info_reply_v4)
	},
	{
		NDMP_CONFIG_GET_FS_INFO, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_config_get_fs_info_reply_v4)
	},
	{
		NDMP_CONFIG_GET_SERVER_INFO, NDMPVER,
		0,
		AUTH_NOT_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_config_get_server_info_reply_v4)
	},
	{
		NDMP_CONFIG_GET_BUTYPE_ATTR, NDMPVER,
		0,
		AUTH_NOT_REQUIRED,
		XDR_AND_SIZE(ndmp_config_get_butype_attr_request),
		XDR_AND_SIZE(ndmp_config_get_butype_attr_reply)
	},
    {
        NDMP_CONFIG_GET_AUTH_ATTR, NDMPVER,
        0,
        AUTH_NOT_REQUIRED,
        XDR_AND_SIZE(ndmp_config_get_auth_attr_request),
        XDR_AND_SIZE(ndmp_config_get_auth_attr_reply)
    },
	{
		NDMP_CONFIG_GET_EXT_LIST, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_config_get_ext_list_reply_v4)
	},
	{
		NDMP_CONFIG_SET_EXT_LIST, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_config_set_ext_list_request_v4),
		XDR_AND_SIZE(ndmp_config_set_ext_list_reply_v4)
	},
	{
		NDMP_CONFIG_GET_CONNECTION_TYPE, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_config_get_connection_type_reply_v4)
	},
	{
		NDMP_CAB_DATA_CONN_PREPARE, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_data_start_backup_request_v4),
		XDR_AND_SIZE(ndmp_data_start_backup_reply_v4)
	},
	{
		NDMP_SCSI_OPEN, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_scsi_open_request),
		XDR_AND_SIZE(ndmp_scsi_open_reply)
	},
	{
		NDMP_SCSI_CLOSE, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_scsi_close_reply)
	},
	{
		NDMP_SCSI_GET_STATE, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_scsi_get_state_reply)
	},
	{
		NDMP_SCSI_SET_TARGET, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_scsi_set_target_request),
		XDR_AND_SIZE(ndmp_scsi_set_target_reply)
	},
	{
		NDMP_SCSI_RESET_DEVICE, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_scsi_reset_device_reply)
	},
	{
		NDMP_SCSI_RESET_BUS, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_scsi_reset_bus_reply)
	},
	{
		NDMP_SCSI_EXECUTE_CDB, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_execute_cdb_request),
		XDR_AND_SIZE(ndmp_execute_cdb_reply)
	},
	{
		NDMP_TAPE_OPEN, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_tape_open_request),
		XDR_AND_SIZE(ndmp_tape_open_reply)
	},
	{
		NDMP_TAPE_CLOSE, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_tape_close_reply)
	},
	{
		NDMP_TAPE_GET_STATE, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_tape_get_state_reply_v2)
	},
	{
		NDMP_TAPE_MTIO, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_tape_mtio_request),
		XDR_AND_SIZE(ndmp_tape_mtio_reply)
	},
	{
		NDMP_TAPE_WRITE, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_tape_write_request),
		XDR_AND_SIZE(ndmp_tape_write_reply)
	},
	{
		NDMP_TAPE_READ, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_tape_read_request),
		XDR_AND_SIZE(ndmp_tape_read_reply)
	},
	{
		NDMP_TAPE_EXECUTE_CDB, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_tape_execute_cdb_request),
		XDR_AND_SIZE(ndmp_tape_execute_cdb_reply)
	},
	{
		NDMP_DATA_GET_STATE, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_data_get_state_reply_v4)
	},
	{
		NDMP_DATA_START_BACKUP, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_data_start_backup_request_v4),
		XDR_AND_SIZE(ndmp_data_start_backup_reply_v4)
	},
	{
		NDMP_DATA_START_RECOVER, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_data_start_recover_request_v4),
		XDR_AND_SIZE(ndmp_data_start_recover_reply_v4)
	},
	{
		NDMP_DATA_ABORT, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_data_abort_reply)
	},
	{
		NDMP_DATA_GET_ENV, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_data_get_env_reply)
	},
	{
		NDMP_DATA_STOP, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_data_stop_reply)
	},
	{
		NDMP_DATA_LISTEN, NDMPVER,
		0,
		AUTH_REQUIRED,
        XDR_AND_SIZE(ndmp_data_listen_request_v4),
        XDR_AND_SIZE(ndmp_data_listen_reply_v4)
	},
	{
		NDMP_DATA_CONNECT, NDMPVER,
		0,
		AUTH_REQUIRED,
        XDR_AND_SIZE(ndmp_data_connect_request_v4),
        XDR_AND_SIZE(ndmp_data_connect_reply_v4)
	},
	{
		NDMP_NOTIFY_DATA_HALTED, NDMPVER,
		notifyDataHalted,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_notify_data_halted_request_v4),
		0, 0
	},
	{
		NDMP_NOTIFY_CONNECTION_STATUS, NDMPVER,
		notifyConnected,
		AUTH_NOT_REQUIRED,
		XDR_AND_SIZE(ndmp_notify_connected_request),
		0, 0
	},
	{
		NDMP_NOTIFY_DATA_READ, NDMPVER,
		notifyDataRead,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_notify_data_read_request),
		0, 0
	},
	{
		_NDMP_LOG_LOG, NDMPVER,
		logLog,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_log_log_request),
		0, 0
	},
	{
		_NDMP_LOG_DEBUG, NDMPVER,
		logDebug,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_log_debug_request),
		0, 0
	},
	{
		NDMP_LOG_FILE, NDMPVER,
		logFile,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_log_file_request_v4),
		0, 0
	},
	{
		NDMP_LOG_MESSAGE, NDMPVER,
		logMessage,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_log_message_request_v4),
		0, 0
	},
	{
		NDMP_FH_ADD_UNIX_PATH, NDMPVER,
		fileHistoryAddUnix,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_fh_add_unix_path_request),
		0, 0
	},
	{
		NDMP_FH_ADD_UNIX_NODE, NDMPVER,
		fileHistoryAddUnixNode,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_fh_add_unix_node_request),
		0, 0
	},
	{
		NDMP_FH_ADD_UNIX_DIR, NDMPVER,
		fileHistoryAddUnixDir,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_fh_add_unix_dir_request),
		0, 0
	},
	{
		NDMP_FH_ADD_DIR, NDMPVER,
		fileHistoryAddDir,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_fh_add_dir_request_v3),
		0, 0
	},
	{
		NDMP_FH_ADD_NODE, NDMPVER,
		fileHistoryAddNode,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_fh_add_node_request_v3),
		0, 0
	},
	{
		NDMP_FH_ADD_FILE, NDMPVER,
		FileHistoryAddFile,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_fh_add_file_request_v3),
		0, 0
	},
	{
		NDMP_MOVER_GET_STATE, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_mover_get_state_reply)
	},
	{
		NDMP_MOVER_LISTEN, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_mover_listen_request),
		XDR_AND_SIZE(ndmp_mover_listen_reply)
	},
	{
		NDMP_MOVER_CONTINUE, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_mover_continue_reply)
	},
	{
		NDMP_MOVER_ABORT, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_mover_abort_reply)
	},
	{
		NDMP_MOVER_STOP, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_mover_stop_reply)
	},
	{
		NDMP_MOVER_SET_WINDOW, 2,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_mover_set_window_request),
		XDR_AND_SIZE(ndmp_mover_set_window_reply)
	},
	{
		NDMP_MOVER_READ, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_mover_read_request),
		XDR_AND_SIZE(ndmp_mover_read_reply)
	},
	{
		NDMP_MOVER_CLOSE, NDMPVER,
		0,
		AUTH_REQUIRED,
		0, 0,
		XDR_AND_SIZE(ndmp_mover_close_reply)
	},
	{
		NDMP_MOVER_SET_RECORD_SIZE, NDMPVER,
		0,
		AUTH_REQUIRED,
		XDR_AND_SIZE(ndmp_mover_set_record_size_request),
		XDR_AND_SIZE(ndmp_mover_set_record_size_reply)
	},
	{
		(ndmp_message)0,
		0,
		0,
		0, 0,
		0, 0
	}
};