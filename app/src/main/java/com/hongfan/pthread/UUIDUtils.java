package com.hongfan.pthread;

import java.util.UUID;

/**
 * 描述：提供给JNI调用
 * 作者：JamFF
 * 创建时间：2018/7/20 20:05
 */
public class UUIDUtils {

    public static String get() {
        return UUID.randomUUID().toString();
    }
}
