#pragma once



///[Guid("D92270BF-3E59-4DCB-9F4A-954C7BDCA1A1")]

#define WPP_CONTROL_GUIDS                                              \
    WPP_DEFINE_CONTROL_GUID(                                           \
        BkDriverTraceGuid, (D92270BF, 3E59, 4DCB, 9F4A, 954C7BDCA1A1), \
                                                                            \
        WPP_DEFINE_BIT(FLAG_BK_DRIVER_COMMON)                                   \
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

//
// This comment block is scanned by the trace preprocessor to define our
// Trace function.
//
// begin_wpp config
// FUNC TracePrint{FLAG=FLAG_BK_DRIVER_COMMON}(LEVEL, MSG, ...);
// end_wpp
//
