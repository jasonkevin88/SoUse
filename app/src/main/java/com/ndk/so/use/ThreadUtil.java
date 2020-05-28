package com.ndk.so.use;

import android.util.Log;

/**
 * Description:ThreadUtil
 *
 * @author 陈宝阳
 * @create 2020/5/28 10: 56
 */
public class ThreadUtil {


  public native void createThread();

  public native void setJniEnv();

  public native void releaseThread();

  public void jniThreadCallBack() {

  }

  public static void formJni( int i) {
    Log.d("so_use","form jni : " +i);
  };

  public static void formJNIAgain(int i) {
    Log.v("so_use","form_JNI_Again : "+i);
  }
}
