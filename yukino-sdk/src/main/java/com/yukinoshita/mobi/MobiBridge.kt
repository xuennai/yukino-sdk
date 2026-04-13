package com.yukinoshita.mobi

import android.util.Log

/**
 * 专门负责与 C++ 层通信的 Mobi 桥接类
 */
object MobiBridge {
    init {
        try {
            System.loadLibrary("yukino_mobi")
        } catch (e: UnsatisfiedLinkError) {
            Log.e("MobiBridge", "无法加载 yukino_mobi 本地库，请检查 NDK 编译是否成功", e)
        }
    }

    /**
     * JNI 接口：获取漫画总页数 (图片总数)
     * @param fd 文件描述符
     * @param drmPid 用于解密的 PID，如果没有则传 null
     */
    external fun getPageCountNative(fd: Int, drmPid: String?): Int

    /**
     * JNI 接口：获取指定页的图片字节数组
     * @param fd 文件描述符
     * @param index 图片索引 (0 开始)
     * @param drmPid 用于解密的 PID，如果没有则传 null
     */
    external fun getPageBytesNative(fd: Int, index: Int, drmPid: String?): ByteArray?
}
