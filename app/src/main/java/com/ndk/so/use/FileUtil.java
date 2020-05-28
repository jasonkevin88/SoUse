package com.ndk.so.use;

/**
 * Description:FileUtil
 *
 * @author 陈宝阳
 * @create 2020/5/27 16: 47
 */
public class FileUtil {

  public static native int diffFile(String srcPath, String partPath, int count);

  public static native int mergeFile(String mergePath, String partPath, int count);
}
