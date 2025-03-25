#ifndef _EXTERNAL_PLUGIN_RUN_STATE_H
#define _EXTERNAL_PLUGIN_RUN_STATE_H
#include <memory>
#include "common/Types.h"
typedef enum {
    INITIALIZE = 0,  // 初始状态
    STARTING,        // 启动中，外部插件还未注册
    ISREGISTERED,      // 已注册正常运行中
    CLOSING,         // 关闭中
    CLOSED           // 当插件去注册后,状态即为已关闭
} EX_PLUGIN_STATUS;

class ExternalPlugin;
class ExPluginStateBase {
public:
    ExPluginStateBase(ExternalPlugin *context) : m_context(context)
    {}
    virtual ~ExPluginStateBase()
    {}
    virtual mp_uint32 PluginStarting()
    {
        return MP_FAILED;
    }
    virtual mp_uint32 PluginRegistered()
    {
        return MP_FAILED;
    }
    virtual mp_uint32 PluginClosing()
    {
        return MP_FAILED;
    }
    virtual mp_uint32 PluginClosed();

protected:
    ExternalPlugin *m_context;
};

class PluginIdleState : public ExPluginStateBase {
public:
    PluginIdleState(ExternalPlugin *context) : ExPluginStateBase(context)
    {}
    ~PluginIdleState()
    {}
    mp_uint32 PluginStarting() override;
};

class PluginStartingState : public ExPluginStateBase {
public:
    PluginStartingState(ExternalPlugin *context) : ExPluginStateBase(context)
    {}
    ~PluginStartingState()
    {}
    mp_uint32 PluginRegistered() override;
    mp_uint32 PluginClosing() override;
};

class PluginRegisterdState : public ExPluginStateBase {
public:
    PluginRegisterdState(ExternalPlugin *context) : ExPluginStateBase(context)
    {}
    ~PluginRegisterdState()
    {}
    mp_uint32 PluginStarting() override;
    mp_uint32 PluginClosing() override;
};

class PluginClosingState : public ExPluginStateBase {
public:
    PluginClosingState(ExternalPlugin *context) : ExPluginStateBase(context)
    {}
    ~PluginClosingState()
    {}
    mp_uint32 PluginClosing() override
    {
        return MP_SUCCESS;
    }
};
#endif