package com.yukinoshita.pdfium

import android.graphics.Bitmap

class PdfiumBridge {

    companion object {
        init {
            System.loadLibrary("pdfium")     // 加载 libpdfium.so
            System.loadLibrary("yukino_pdf") // 加载你的桥接层 libyukino_pdf.so (看你之前的配置)
        }
    }

    // 将外部可见的方法暴露出去
    fun openDocument(fd: Int): Long = openDocumentNative(fd)
    fun getPageCount(docPtr: Long): Int = getPageCountNative(docPtr)
    fun getPageWidth(docPtr: Long, pageIdx: Int): Int = getPageWidthNative(docPtr, pageIdx)
    fun getPageHeight(docPtr: Long, pageIdx: Int): Int = getPageHeightNative(docPtr, pageIdx)
    fun renderPageBitmap(docPtr: Long, bitmap: Bitmap, pageIdx: Int, drawWidth: Int, drawHeight: Int) =
        renderPageBitmapNative(docPtr, bitmap, pageIdx, drawWidth, drawHeight)
    fun closeDocument(docPtr: Long) = closeDocumentNative(docPtr)

    // JNI 接口保持 private，不对主 App 暴露
    private external fun openDocumentNative(fd: Int): Long
    private external fun getPageCountNative(docPtr: Long): Int
    private external fun getPageWidthNative(docPtr: Long, pageIdx: Int): Int
    private external fun getPageHeightNative(docPtr: Long, pageIdx: Int): Int
    private external fun renderPageBitmapNative(docPtr: Long, bitmap: Bitmap, pageIdx: Int, drawWidth: Int, drawHeight: Int)
    private external fun closeDocumentNative(docPtr: Long)
}
