//
// Created by poe on 2020/3/24.
//

#include "JavaCallHelper.h"
#include "macro.h"

JavaCallHelper::JavaCallHelper(JavaVM *_javaVM, JNIEnv *_env, jobject &_obj) :javaVm(_javaVM),env(_env){
    //建立全局的引用对象jobj防止方法执行结束，内存被回收.
    jobj = env->NewGlobalRef(_obj);
    jclass jclassz = env->GetObjectClass(jobj);
    //ArtMethod error.
    jmid_error = env->GetMethodID(jclassz,"onError","(I)V");
    jmid_prepare = env->GetMethodID(jclassz,"onPrepare","()V");
    jmid_progress = env->GetMethodID(jclassz,"onProgress","(I)V");
}

JavaCallHelper::~JavaCallHelper() {

}

void JavaCallHelper::onPrepare(int thread) {
    if(thread == THREAD_CHILD){
        JNIEnv* jniEnv;
        if((javaVm->AttachCurrentThread(&jniEnv , 0)) != JNI_OK){
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_prepare );
        //调用完成之后需要解绑.
        javaVm->DetachCurrentThread();
    }else{
        env->CallVoidMethod(jobj,jmid_prepare);

    }
}

void JavaCallHelper::onProgress(int thread, int progress) {

    if(thread == THREAD_CHILD){
        JNIEnv* jniEnv;
        if((javaVm->AttachCurrentThread(&jniEnv , 0)) != JNI_OK){
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_progress , progress);
        //调用完成之后需要解绑.
        javaVm->DetachCurrentThread();
    }else{
        env->CallVoidMethod(jobj,jmid_progress, progress);

    }
}

void JavaCallHelper::onError(int thread, int code) {

    if(thread == THREAD_CHILD){
        JNIEnv* jniEnv;
        if((javaVm->AttachCurrentThread(&jniEnv , 0)) != JNI_OK){
            return;
        }
        jniEnv->CallVoidMethod(jobj, jmid_error , code);
        //调用完成之后需要解绑.
        javaVm->DetachCurrentThread();
    }else{
        env->CallVoidMethod(jobj,jmid_error, code);

    }
}
