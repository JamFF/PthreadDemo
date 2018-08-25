#include <jni.h>
#include <stdio.h>
#include <pthread.h>
#include <android/log.h>
#include <unistd.h>

#define LOG_I(FORMAT, ...) __android_log_print(ANDROID_LOG_INFO,"JamFF",FORMAT,##__VA_ARGS__);
#define LOG_E(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"JamFF",FORMAT,##__VA_ARGS__);

// JavaVM 代表的是Java虚拟机，所有的工作都是从JavaVM开始
// 可以通过JavaVM获取到每个线程关联的JNIEnv

// 如何获取JavaVM？
// 1.在JNI_OnLoad函数中获取// 2.2以后版本
// 2.(*env)->GetJavaVM(env,&javaVM);// 兼容各个版本

JavaVM *javaVM;
jobject uuidutils_jcls_global;
jmethodID uuidutils_get_mid;

// 动态库加载时会执行
// Android SDK 2.2之后才有，2.2没有这个函数
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    LOG_I("%s", "JNI_OnLoad");
    // 获取JavaVM
    javaVM = vm;
    return JNI_VERSION_1_4;
}

void *th_fun(void *arg) {

    char *no = (char *) arg;
    LOG_I("thread %s", no);

    // 每个线程都有独立的JNIEnv
    JNIEnv *env;

    // args：关联参数，可以不设置
    // JavaVMAttachArgs args = {JNI_VERSION_1_4, "my_thread", NULL};
    // (*javaVM)->AttachCurrentThread(javaVM, &env, &args);

    // 通过JavaVM关联当前线程，获取当前线程的JNIEnv
    (*javaVM)->AttachCurrentThread(javaVM, &env, NULL);

    /*// 获取MethodID，可以在子线程
    uuidutils_get_mid = (*env)->GetStaticMethodID(env, uuidutils_jcls_global, "get",
                                                  "()Ljava/lang/String;");*/

    int i;
    for (i = 0; i < 5; ++i) {

        jobject uuid_jstr = (*env)->CallStaticObjectMethod(env, uuidutils_jcls_global,
                                                           uuidutils_get_mid);
        const char *uuid_cstr = (*env)->GetStringUTFChars(env, uuid_jstr, NULL);

        LOG_I("uuid:%s", uuid_cstr);

        if (i == 3) {
            goto end;
        }

        sleep(1);
    }

    end:
    // 取消关联
    (*javaVM)->DetachCurrentThread(javaVM);
    LOG_I("DetachCurrentThread");
    pthread_exit((void *) 0);

    return NULL;
}

JNIEXPORT void JNICALL
Java_com_hongfan_pthread_PosixThread_pthread(JNIEnv *env, jobject instance) {

    pthread_t tid;
    pthread_create(&tid, NULL, th_fun, "NO1");
}

JNIEXPORT void JNICALL
Java_com_hongfan_pthread_PosixThread_init(JNIEnv *env, jobject instance) {

    // 在主线程中FindClass，获取class必须要在主线程
    jclass uuidutils_jcls = (*env)->FindClass(env, "com/hongfan/pthread/UUIDUtils");
    // 创建全局引用
    uuidutils_jcls_global = (*env)->NewGlobalRef(env, uuidutils_jcls);
    // 获取MethodID，可以在子线程
    uuidutils_get_mid = (*env)->GetStaticMethodID(env, uuidutils_jcls_global, "get",
                                                  "()Ljava/lang/String;");
    // 获取JavaVM
    // (*env)->GetJavaVM(env, &javaVM);
}

JNIEXPORT void JNICALL
Java_com_hongfan_pthread_PosixThread_destroy(JNIEnv *env, jobject instance) {

    // 释放全局引用
    (*env)->DeleteGlobalRef(env, uuidutils_jcls_global);
}