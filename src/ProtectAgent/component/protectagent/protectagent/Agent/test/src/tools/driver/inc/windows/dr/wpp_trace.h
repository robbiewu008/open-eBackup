#pragma once



///[Guid("24BF7176-37EE-4837-B5E8-B42DDC759338")]

#define WPP_CONTROL_GUIDS                                              \
    WPP_DEFINE_CONTROL_GUID(                                           \
        DrDriverTraceGuid, (24BF7176, 37EE, 4837, B5E8, B42DDC759338), \
                                                                            \
        WPP_DEFINE_BIT(FLAG_DR_DRIVER_COMMON)                                   \
        )                             

#define WPP_FLAG_LEVEL_LOGGER(flag, level)                                  \
    WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_LEVEL_ENABLED(flag, level)                                 \
    (WPP_LEVEL_ENABLED(flag) &&                                             \
     WPP_CONTROL(WPP_BIT_ ## flag).Level >= level)

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) \
           WPP_LEVEL_LOGGER(flags)

#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) \
           (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)


struct xstr_t 
{
	CHAR * _buf;
	USHORT _len;
	xstr_t(__in_ecount(len) CHAR *buf, USHORT len) :_buf(buf), _len(len) {}
};

#define LOG_LENSTR(str, len) xstr_t((CHAR*)str, len)
#define WPP_LOGHEXDUMP(x) WPP_LOGPAIR(2,&(x)._len) WPP_LOGPAIR((x)._len, (x)._buf)

//
// This comment block is scanned by the trace preprocessor to define our
// Trace function.
//
// begin_wpp config
// DEFINE_CPLX_TYPE(HEXDUMP, WPP_LOGHEXDUMP, const xstr_t&, ItemHEXDump,"s", _HEX_, 0, 2);
// FUNC TracePrint{FLAG=FLAG_DR_DRIVER_COMMON}(LEVEL, MSG, ...);
// end_wpp
//
