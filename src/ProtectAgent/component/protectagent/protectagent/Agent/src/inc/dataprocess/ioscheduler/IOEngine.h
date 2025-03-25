#ifndef __IO_ENGINE_H__
#define __IO_ENGINE_H__

#include <cstdint>
#include "common/Types.h"

class IOEngine {
public:
    virtual ~IOEngine() = default;
    /**
     *  @brief  指定位置读
     *  @param  offsetInBytes 偏移地址
     *  @param  bufferSizeInBytes 数据块大小
     *  @param  buffer 数据块
     */
    virtual mp_int32 Read(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer) = 0;
    virtual mp_string GetFileName() = 0;
    virtual mp_string GetFileNameForWrite() {return "";}
    /**
     *  @brief  指定位置写
     *  @param  offsetInBytes 偏移地址
     *  @param  bufferSizeInBytes 数据块大小
     *  @param  buffer 数据块
     */
    virtual mp_int32 Write(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer) = 0;
    
    /**
     *  @brief  打开IO引擎，默认为空处理
     */
    virtual mp_int32 Open()
    {
        return MP_SUCCESS;
    }

    /**
     *  @brief  关闭IO引擎，默认为空处理
     */
    virtual mp_int32 Close()
    {
        return MP_SUCCESS;
    }
    
    /**
     *  @brief  备份任务的后置处理，默认为空处理
     */
    virtual mp_int32 PostBackup()
    {
        return MP_SUCCESS;
    }

    /**
     *  @brief  恢复任务的后置处理，默认为空处理
     */
    virtual mp_int32 PostRecovery()
    {
        return MP_SUCCESS;
    }

    virtual mp_string GetErrDesc()
    {
        return m_errDesc;
    }

protected:
    mp_string m_errDesc = "failure";
};

#endif