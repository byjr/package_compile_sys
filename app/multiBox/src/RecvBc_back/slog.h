#include "android/log.h"
#define s_err(x...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG1,x)
#define s_war(x...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG1,x)
#define s_inf(x...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG1,x)
#define s_dbg(x...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG1,x)