package com.hongfan.pthread;

/**
 * 描述：
 * 作者：JamFF
 * 创建时间：2018/7/16 16:40
 */
public class PosixThread {

    public native void init();

    public native void destroy();

    public native void startThread();

    static {
        System.loadLibrary("native-lib");
    }
}
