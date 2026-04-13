#include <jni.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fpdfview.h>
#include <android/bitmap.h>
#include <android/log.h>
#include <mutex>

#define LOG_TAG "PdfComic"

// ==========================================
// 自动化日志开关
// ==========================================
#ifdef NDEBUG
// Release 版本：日志变为空白，不执行任何操作
    #define LOGD(...) ((void)0)
    #define LOGE(...) ((void)0)
#else
// Debug 版本：正常打印日志
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif

struct PdfContext {
    FPDF_FILEACCESS fileAccess;
    int fd;
    FPDF_DOCUMENT doc;
};

static int GetBlock(void* param, unsigned long position, unsigned char* pBuf, unsigned long size) {
    PdfContext* ctx = (PdfContext*)param;
    ssize_t bytes_read = pread(ctx->fd, pBuf, size, position);
    return (bytes_read < 0) ? 0 : 1;
}

extern "C" {

// 1. 打开 PDF 文档
// 【注意】：这里的函数名已经修改为匹配 com.yukinoshita.pdfium.PdfiumBridge
JNIEXPORT jlong JNICALL
Java_com_yukinoshita_pdfium_PdfiumBridge_openDocumentNative(
        JNIEnv *env, jobject thiz, jint fd) {

    static std::once_flag initFlag;
    std::call_once(initFlag, [](){
        FPDF_LIBRARY_CONFIG config;
        config.version = 2;
        config.m_pUserFontPaths = NULL;
        config.m_pIsolate = NULL;
        config.m_v8EmbedderSlot = 0;
        FPDF_InitLibraryWithConfig(&config);
        LOGD("PDFium 引擎已初始化");
    });

    int dupFd = dup(fd);
    if (dupFd < 0) {
        LOGE("复制文件描述符失败");
        return 0;
    }

    struct stat st;
    if (fstat(dupFd, &st) != 0) {
        LOGE("获取文件状态失败");
        close(dupFd);
        return 0;
    }

    PdfContext* ctx = new PdfContext();
    ctx->fd = dupFd;
    ctx->fileAccess.m_FileLen = st.st_size;
    ctx->fileAccess.m_GetBlock = GetBlock;
    ctx->fileAccess.m_Param = ctx;

    ctx->doc = FPDF_LoadCustomDocument(&ctx->fileAccess, NULL);
    if (!ctx->doc) {
        LOGE("PDFium 文档加载失败! 错误码: %lu", FPDF_GetLastError());
        close(dupFd);
        delete ctx;
        return 0;
    }

    LOGD("文档加载成功: %lld 字节", (long long)st.st_size);
    return (jlong)ctx;
}

// 2. 获取总页数
JNIEXPORT jint JNICALL
Java_com_yukinoshita_pdfium_PdfiumBridge_getPageCountNative(
        JNIEnv *env, jobject thiz, jlong docPtr) {
    if (docPtr == 0) return 0;
    return FPDF_GetPageCount(((PdfContext*)docPtr)->doc);
}

// 3. 获取页面尺寸
JNIEXPORT jint JNICALL
Java_com_yukinoshita_pdfium_PdfiumBridge_getPageWidthNative(
        JNIEnv *env, jobject thiz, jlong docPtr, jint pageIdx) {
    if (docPtr == 0) return 0;
    FPDF_PAGE page = FPDF_LoadPage(((PdfContext*)docPtr)->doc, pageIdx);
    int width = (int)FPDF_GetPageWidth(page);
    FPDF_ClosePage(page);
    return width;
}

JNIEXPORT jint JNICALL
Java_com_yukinoshita_pdfium_PdfiumBridge_getPageHeightNative(
        JNIEnv *env, jobject thiz, jlong docPtr, jint pageIdx) {
    if (docPtr == 0) return 0;
    FPDF_PAGE page = FPDF_LoadPage(((PdfContext*)docPtr)->doc, pageIdx);
    int height = (int)FPDF_GetPageHeight(page);
    FPDF_ClosePage(page);
    return height;
}

// 4. 渲染页面
JNIEXPORT void JNICALL
Java_com_yukinoshita_pdfium_PdfiumBridge_renderPageBitmapNative(
        JNIEnv *env, jobject thiz, jlong docPtr, jobject bitmap, jint pageIdx, jint drawWidth, jint drawHeight) {

if (docPtr == 0 || bitmap == NULL) return;
PdfContext* ctx = (PdfContext*)docPtr;

AndroidBitmapInfo info;
void* pixels;
if (AndroidBitmap_getInfo(env, bitmap, &info) < 0) return;
if (AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0) return;

FPDF_PAGE page = FPDF_LoadPage(ctx->doc, pageIdx);
if (page) {
FPDF_BITMAP fBitmap = FPDFBitmap_CreateEx(info.width, info.height, FPDFBitmap_BGRA, pixels, info.stride);
if (fBitmap) {
FPDFBitmap_FillRect(fBitmap, 0, 0, info.width, info.height, 0xFFFFFFFF);
// 0x10 = FPDF_REVERSE_BYTE_ORDER (修正 Android 色偏)
FPDF_RenderPageBitmap(fBitmap, page, 0, 0, drawWidth, drawHeight, 0, FPDF_LCD_TEXT | 0x10);
FPDFBitmap_Destroy(fBitmap);
} else {
LOGE("创建 FPDF_BITMAP 画布失败");
}
FPDF_ClosePage(page);
} else {
LOGE("加载页面 %d 失败", pageIdx);
}

AndroidBitmap_unlockPixels(env, bitmap);
}

// 5. 资源释放
JNIEXPORT void JNICALL
Java_com_yukinoshita_pdfium_PdfiumBridge_closeDocumentNative(
        JNIEnv *env, jobject thiz, jlong docPtr) {
if (docPtr == 0) return;
PdfContext* ctx = (PdfContext*)docPtr;

if (ctx->doc) FPDF_CloseDocument(ctx->doc);
if (ctx->fd >= 0) close(ctx->fd);
delete ctx;
LOGD("PDF 文档上下文已关闭释放");
}

} // extern "C"
