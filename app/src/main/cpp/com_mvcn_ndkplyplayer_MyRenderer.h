/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_mvcn_ndkplyplayer_MyRenderer */

#ifndef _Included_com_mvcn_ndkplyplayer_MyRenderer
#define _Included_com_mvcn_ndkplyplayer_MyRenderer
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_mvcn_ndkplyplayer_MyRenderer
 * Method:    nativeInitGLES20
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_com_mvcn_ndkplyplayer_MyRenderer_nativeInitGLES20
  (JNIEnv *, jobject , jstring, jstring);

/*
 * Class:     com_mvcn_ndkplyplayer_MyRenderer
 * Method:    nativeDrawGraphics
 * Signature: (FFF)V
 */
JNIEXPORT void JNICALL Java_com_mvcn_ndkplyplayer_MyRenderer_nativeDrawGraphics
  (JNIEnv *, jobject, jfloat, jfloat, jfloat);

/*
 * Class:     com_mvcn_ndkplyplayer_MyRenderer
 * Method:    nativeSurfaceChanged
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_mvcn_ndkplyplayer_MyRenderer_nativeSurfaceChanged
  (JNIEnv *, jobject, jint, jint);

#ifdef __cplusplus
}
#endif
#endif
