/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file IOTask.h
 * @author t00302329
 * @brief 实现备份恢复过程中的IO读写
 * @version 0.1
 * @date 2021-01-14
 *
 */
#include "dataprocess/ioscheduler/IOTask.h"
#include "common/Log.h"
#include "dataprocess/datapath/VMwareNativeDataPathImpl.h"

const char ZERO_ARR[VMWARE_DATABLOCK_SIZE] = {0};
void IOTask::Exec()
{
    if (m_reader.get() == nullptr || m_writer.get() == nullptr || m_buffer.get() == nullptr) {
        COMMLOG(OS_LOG_ERROR, "Reader or writer is not set for this io task");
        return;
    }

    // 读端获取到数据后交由写端写
    mp_int32 rv = m_reader->Read(m_startAddr, m_bufferSize, m_buffer.get());
    if (rv != VIX_OK) {
        COMMLOG(OS_LOG_ERROR, "Read IO <'%llu', '%llu'> failed, ret: '%d'", m_startAddr, m_bufferSize, rv);
        m_errDesc = m_reader->GetErrDesc();
        m_result = -1;
        return;
    }

    if (m_taskType == VMWARE_VM_BACKUP) {
        if (m_bufferSize <= VMWARE_DATABLOCK_SIZE) {
            if (memcmp(m_buffer.get(), ZERO_ARR, m_bufferSize) == 0) {
                m_zero = true;
                return;
            }
        }
    }

    rv = m_writer->Write(m_startAddr, m_bufferSize, m_buffer.get());
    if (rv != VIX_OK) {
        COMMLOG(OS_LOG_ERROR, "Write IO <'%llu', '%llu'> failed, ret: '%d'", m_startAddr, m_bufferSize, rv);
        m_errDesc = m_writer->GetErrDesc();
        m_result = -1;
        return;
    }
}
