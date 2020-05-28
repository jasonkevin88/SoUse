#include <jni.h>
#include <string>
#include <android/log.h>
#include "my-test.h"

#define LOG_TAG "so_use"
#define LOG_I(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

extern "C" JNIEXPORT jstring JNICALL
Java_com_ndk_so_use_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

//定义的对应java中的定义native方法
JNIEXPORT jint JNICALL native_add(JNIEnv *env, jobject job, jint a, jint b)
{
    LOG_I("%d,%d", a, b);
    return sum(a,b);
}

static int registerNativeMethods(JNIEnv *env, const char *className,
                                 JNINativeMethod *gMethods, int numMethods) {
    jclass clazz;
    clazz = env->FindClass(className);
    if (clazz == NULL) {
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

//需要动态注册的native方法所在的类
#define JNIREG_CLS_MAIN "com/ndk/so/use/MainActivity"


//创建JNINativeMethod的数组，用来存放，JNINativeMethod结构变量，JNINativeMethod结构存放：注册的native方法，对应的签名，C++/C的对应的JNI方法
static JNINativeMethod gMethods[] = {
        {"doAdd","(II)I", (void *)native_add}
};


/***
* 注册MainActivity的native方法
*/
static int registerNativesMainActivity(JNIEnv* env) {
    if (!registerNativeMethods(env, JNIREG_CLS_MAIN, gMethods, sizeof(gMethods) / sizeof(gMethods[0]))) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}
//------------------------------------文件的拆分和合并-----------------------------------------

/**
 * 获取文件大小
 */
long  get_file_size(const char* path) {
    FILE *fp = fopen(path, "rb"); //打开一个文件， 文件必须存在，只运行读
    if(fp == NULL) {
        LOG_I("open file failed...");
        return 0;
    }
    fseek(fp, 0, SEEK_END);
    long ret = ftell(fp);
    fclose(fp);
    return ret;
}

/**
 * 文件拆分
 */
JNIEXPORT jint JNICALL native_diff(JNIEnv *env, jclass jclz, jstring src_path, jstring part_path, jint count)
{
    LOG_I("start begin diff file.");
    //jstring 转 char*
    // 需要拆分的文件路径
    const char* srcPath = env->GetStringUTFChars(src_path, NULL);
    // 拆分后的文件路径格式
    const char* partPath = env->GetStringUTFChars(part_path, NULL);

    char *partPaths[count];
    int i;
    for (i = 0; i < count; ++i) {
        //每个文件名申请地址
        LOG_I("char = %d char * = %d", sizeof(char), sizeof(char *));
        partPaths[i] = (char*)malloc(sizeof(char) * 200);
        // 需要分割的文件 Vibrato.mp4
        // 每个子文件名称 Vibrato_n.mp4
        sprintf(partPaths[i], partPath, i);
        LOG_I("patch path : %s", partPaths[i]);
    }

    int fileSize = get_file_size(srcPath);
    FILE *fpr = fopen(srcPath, "rb");
    if(fpr == NULL) {
        LOG_I("open file failed...");
        return 0;
    }

    //判断文件大小能够被 count 整除
    if(fileSize % count == 0) {
        //能整除就平分，获取每一份的大小
        int part = fileSize / count;
        for (int i = 0; i < count; ++i) {
            FILE *fpw = fopen(partPaths[i], "wb"); //文件已经存在 就删除，只运行写
            for (int j = 0; j < part; ++j) {
                fputc(fgetc(fpr), fpw);
            }
            fclose(fpw);
        }
    } else{
        //不能整除就先分 count -1， 剩下的作为单独一份
        int part = fileSize / (count -1);
        for (int i = 0; i < count - 1; ++i) {
            FILE *fpw = fopen(partPaths[i], "wb");
            for (int j = 0; j < part; ++j) {
                fputc(fgetc(fpr), fpw);
            }
            fclose(fpw);
        }

        FILE *fpw = fopen(partPaths[count - 1], "wb");
        for (int j = 0; j < fileSize % (count - 1); ++j) {
            fputc(fgetc(fpr), fpw);
        }
        fclose(fpw);
    }

    fclose(fpr);

    free(partPaths);
    env->ReleaseStringUTFChars(src_path, srcPath);
    env->ReleaseStringUTFChars(part_path, partPath);
    LOG_I("diff file finish.");
    return 1;
}

/**
 * 文件合并
 */
JNIEXPORT jint JNICALL native_merge(JNIEnv *env, jclass jclz, jstring merge_path, jstring part_path, jint count)
{
    LOG_I("start begin merge file.");
    //获取合并后生成的文件路径
    const char * mergePath = env->GetStringUTFChars(merge_path, NULL);
    //获取拆分文件的格式
    const char * partPath = env->GetStringUTFChars(part_path, NULL);

    char *partPaths[count];

    for (int i = 0; i < count; ++i) {
        partPaths[i] = (char*) malloc(sizeof(char) * 200);

        sprintf(partPaths[i], partPath, i);
        LOG_I("partPaths[%d] = %s", i, partPaths[i]);
    }

    FILE *fpw = fopen(mergePath, "wb");
    if(fpw == NULL) {
        return 0;
    }

    for (int i = 0; i < count; ++i) {
        FILE *fpr = fopen(partPaths[i], "rb");
        int fileSize = get_file_size(partPaths[i]);
        for (int j = 0; j < fileSize; ++j) {
            fputc(fgetc(fpr), fpw);
        }
        fclose(fpr);
    }
    fclose(fpw);
    free(partPaths);
    env->ReleaseStringUTFChars(merge_path, mergePath);
    env->ReleaseStringUTFChars(part_path, partPath);
    LOG_I("file merge finish");
    return 1;
}


//定义类文件变量
#define JNIREG_CLS_FILE "com/ndk/so/use/FileUtil"

//创建JNINativeMethod的数组，用来存放，JNINativeMethod结构变量，JNINativeMethod结构存放：注册的native方法，对应的签名，C++/C的对应的JNI方法
static JNINativeMethod gMethods_File[] = {
        {"diffFile","(Ljava/lang/String;Ljava/lang/String;I)I", (void *)native_diff},
        {"mergeFile","(Ljava/lang/String;Ljava/lang/String;I)I", (void *)native_merge}
};

/***
* 注册MainActivity的native方法
*/
static int registerNativesFileUtil(JNIEnv* env) {
    if (!registerNativeMethods(env, JNIREG_CLS_FILE, gMethods_File, sizeof(gMethods_File) / sizeof(gMethods_File[0]))) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

//-------------------------------------文件的拆分和合并-End------------------------------------

//-------------------------------------多线程-------------------------------------------------

#include <pthread.h>

JavaVM* g_jvm = NULL;
jobject g_obj = NULL;

//一个进程只有一个jvm
//一个线程一个JniEnv
void* thread_fun(void * args) {

    JNIEnv *env;
    jclass clz;
    jmethodID  mid, mid1;

    if(g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
        LOG_I("%s AttachCurrentThread error failed ",__FUNCTION__);
        return NULL;
    }

    clz = env->GetObjectClass(g_obj);

    if(clz == NULL) {
        LOG_I("load class failed.");
        goto error;
    }

    LOG_I("call back begin");
    mid = env->GetStaticMethodID(clz, "formJni", "(I)V");
    if (mid == NULL) {
        LOG_I("GetStaticMethodID error....");
        goto error;
    }

    env-> CallStaticVoidMethod(clz, mid, args);

    mid1 = env->GetStaticMethodID(clz, "formJNIAgain", "(I)V");
    if (mid1 == NULL) {
        LOG_I("GetStaticMethodID error....");
        goto error;
    }

    env-> CallStaticVoidMethod(clz, mid1, args);

    error:
    if (g_jvm -> DetachCurrentThread() != JNI_OK) {
        LOG_I("%s DetachCurrentThread error failed ",__FUNCTION__);
    }
    pthread_exit(0);
}

JNIEXPORT void JNICALL native_createThread(JNIEnv *env, jobject job)
{
    LOG_I("createThread begin");
    int i;
    pthread_t pt[5];

    for (int i = 0; i < 5; ++i) {
        pthread_create(&pt[i], NULL, &thread_fun, (void*)i);
    }
}

JNIEXPORT void JNICALL native_setJniEnv(JNIEnv *env, jobject job)
{
    LOG_I("native_setJniEnv");
    if(g_jvm != NULL) {
        g_jvm = NULL;
    }

    //保存JVM
    env -> GetJavaVM(&g_jvm);
    //保持actvity对象
    g_obj = env -> NewGlobalRef(job);
}

JNIEXPORT void JNICALL native_releaseThread(JNIEnv *env, jobject job)
{
    LOG_I("native_releaseThread");
    if(g_jvm != NULL) {
        g_jvm = NULL;
    }
    env->DeleteGlobalRef(g_obj);
}


#define JNIREG_CLS_THREAD "com/ndk/so/use/ThreadUtil"



static JNINativeMethod gMethods_Thread[] {
        {"createThread","()V", (void*)native_createThread},
        {"setJniEnv","()V", (void*)native_setJniEnv},
        {"releaseThread","()V", (void*)native_releaseThread},
};

/***
* 注册MainActivity的native方法
*/
static int registerNativesThreadUtil(JNIEnv* env) {
    if (!registerNativeMethods(env, JNIREG_CLS_THREAD, gMethods_Thread, sizeof(gMethods_Thread) / sizeof(gMethods_Thread[0]))) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

//-------------------------------------动态注册native方法--------------------------------------
/**
* 如果要实现动态注册，这个方法一定要实现
* 动态注册工作在这里进行
*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }
    assert(env != NULL);

    if (!registerNativesMainActivity(env)) { //注册
        return -1;
    }

    if (!registerNativesFileUtil(env)) { //注册
        return -1;
    }

    if (!registerNativesThreadUtil(env)) {
        return -1;
    }
    result = JNI_VERSION_1_4;
    return result;
}