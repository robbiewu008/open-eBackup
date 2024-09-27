#include <jni.h>
/* Header for class com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil */

#ifndef Included_com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil
#define Included_com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil
 * Method:    opaInitialize
 * Signature: (Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL
 Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil_opaInitialize
  (JNIEnv *, jobject, jstring);

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil
 * Method:    getSecurityItemValue
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
 Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil_getSecurityItemValue
  (JNIEnv *, jobject, jint);

/*
 * Class:     com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil
 * Method:    decrypt
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
 Java_com_huawei_oceanprotect_sdk_agent_plugin_lib_security_ability_definition_SecurityUtil_decrypt
  (JNIEnv *, jobject, jstring);

#ifdef __cplusplus
}
#endif
#endif
