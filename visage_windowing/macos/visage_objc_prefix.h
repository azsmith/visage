#pragma once

#ifdef __OBJC__

#ifndef VISAGE_OBJC_API_SUFFIX
#if defined(CLAP_API)
#define VISAGE_OBJC_API_SUFFIX "_clap"
#elif defined(VST2_API)
#define VISAGE_OBJC_API_SUFFIX "_vst"
#elif defined(AU_API)
#define VISAGE_OBJC_API_SUFFIX "_au"
#elif defined(AUv3_API)
#define VISAGE_OBJC_API_SUFFIX "_auv3"
#elif defined(AAX_API)
#define VISAGE_OBJC_API_SUFFIX "_aax"
#elif defined(VST3_API)
#define VISAGE_OBJC_API_SUFFIX "_vst3"
#elif defined(APP_API)
#define VISAGE_OBJC_API_SUFFIX "_app"
#elif defined(REAPER_EXT_API)
#define VISAGE_OBJC_API_SUFFIX "_reaperext"
#else
#define VISAGE_OBJC_API_SUFFIX ""
#endif
#endif

#define VISAGE_OBJC_STRINGIFY2(x) #x
#define VISAGE_OBJC_STRINGIFY(x) VISAGE_OBJC_STRINGIFY2(x)

#ifndef VISAGE_OBJC_RUNTIME_SUFFIX
#if defined(VISAGE_APPLICATION_NAME)
#define VISAGE_OBJC_RUNTIME_SUFFIX "_" VISAGE_APPLICATION_NAME
#elif defined(PLUG_CLASS_NAME)
#define VISAGE_OBJC_RUNTIME_SUFFIX "_" VISAGE_OBJC_STRINGIFY(PLUG_CLASS_NAME)
#elif defined(OBJC_PREFIX)
#define VISAGE_OBJC_RUNTIME_SUFFIX "_" VISAGE_OBJC_STRINGIFY(OBJC_PREFIX) VISAGE_OBJC_API_SUFFIX
#else
#define VISAGE_OBJC_RUNTIME_SUFFIX "_Visage"
#endif
#endif

#define VISAGE_OBJC_RUNTIME(base) __attribute__((objc_runtime_name(base VISAGE_OBJC_RUNTIME_SUFFIX)))

#endif
