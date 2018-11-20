#pragma once

#include <sstream>
#include <iostream>
//#ifdef ANDROID
#include <android/log.h>
//#endif

#define MV_LOG_TAG "NdkPlyPlayer"

class MyStream
{
private:
   std::stringstream m_ss;
   int m_logLevel;
public:

   MyStream(int Xi_logLevel)
   {
      m_logLevel = Xi_logLevel;
   };
   ~MyStream()
   {
#ifdef ANDROID
      __android_log_print(m_logLevel,MV_LOG_TAG,"%s", m_ss.str().c_str());
#else
      printf("%s : %s\n", MV_LOG_TAG, m_ss.str().c_str());
#endif
   }

   template<typename T> MyStream& operator<<(T const& Xi_val)
   {
      m_ss << Xi_val;
      return *this;
   }
};

#define MANTIS_LOG(LOG_LEVEL) MyStream(ANDROID_##LOG_LEVEL) << __FUNCTION__ << ":" << __LINE__ << " : "