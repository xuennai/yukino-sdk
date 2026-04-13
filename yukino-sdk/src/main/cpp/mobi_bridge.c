#include <jni.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <android/log.h>
#include "mobi.h"

#define LOG_TAG "MobiJNI"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// 内部图片过滤逻辑
static bool is_image_resource(MOBIPart *part) {
    if (!part || !part->data || part->size < 4) return false;
    if (part->type == T_JPG || part->type == T_PNG || part->type == T_GIF || part->type == T_BMP) return true;

    const unsigned char *d = part->data;
    if (d[0] == 0xFF && d[1] == 0xD8) return true;
    if (memcmp(d, "\x89PNG", 4) == 0) return true;
    if (memcmp(d, "RIFF", 4) == 0) return true; // WebP 支持

    return false;
}

// 通用解析引擎
MOBIPart** parseMobiCore(int fd, const char* drm_pid, MOBIData** m_out, MOBIRawml** rawml_out, size_t* count_out) {
    MOBIPart** images = NULL;
    size_t capacity = 128;
    size_t count = 0;

    *m_out = NULL;
    *rawml_out = NULL;
    *count_out = 0;

    int dupFd = dup(fd);
    FILE *file = fdopen(dupFd, "rb");
    if (!file) {
        LOGE("Could not open FD");
        return NULL;
    }

    MOBIData *m = mobi_init();
    if (mobi_load_file(m, file) != MOBI_SUCCESS) {
        LOGE("Load MOBI failed");
        fclose(file);
        mobi_free(m);
        return NULL;
    }
    fclose(file);

    if (mobi_is_encrypted(m)) {
        if (drm_pid && strlen(drm_pid) > 0) {
            mobi_drm_setkey(m, drm_pid);
            if (mobi_drm_decrypt(m) != MOBI_SUCCESS) {
                LOGE("DRM Decrypt failed");
                *m_out = m;
                return NULL;
            }
        } else {
            LOGE("Encrypted file but no PID provided");
            *m_out = m;
            return NULL;
        }
    }

    MOBIRawml *rawml = mobi_init_rawml(m);
    if (mobi_parse_rawml(rawml, m) != MOBI_SUCCESS) {
        LOGE("Parse RAWML failed");
        *m_out = m;
        *rawml_out = rawml;
        return NULL;
    }

    images = (MOBIPart**)malloc(capacity * sizeof(MOBIPart*));
    if (!images) return NULL;

    MOBIPart *part = rawml->resources;
    while (part != NULL) {
        if (is_image_resource(part)) {
            if (count >= capacity) {
                capacity *= 2;
                MOBIPart** new_ptr = (MOBIPart**)realloc(images, capacity * sizeof(MOBIPart*));
                if (!new_ptr) break;
                images = new_ptr;
            }
            images[count++] = part;
        }
        part = part->next;
    }

    *m_out = m;
    *rawml_out = rawml;
    *count_out = count;
    return images;
}

// JNI: 获取页数 (函数名已修改为匹配 SDK 包名)
JNIEXPORT jint JNICALL
Java_com_yukinoshita_mobi_MobiBridge_getPageCountNative(
        JNIEnv *env, jobject thiz, jint fd, jstring drm_pid_str) {

    const char *drm_pid = drm_pid_str ? (*env)->GetStringUTFChars(env, drm_pid_str, NULL) : NULL;
    MOBIData *m = NULL;
    MOBIRawml *rawml = NULL;
    size_t count = 0;

    MOBIPart** images = parseMobiCore(fd, drm_pid, &m, &rawml, &count);

    if (drm_pid) (*env)->ReleaseStringUTFChars(env, drm_pid_str, drm_pid);

    jint result = (jint)count;
    if (images) free(images);
    if (rawml) mobi_free_rawml(rawml);
    if (m) mobi_free(m);

    return result;
}

// JNI: 获取图片数据 (函数名已修改为匹配 SDK 包名)
JNIEXPORT jbyteArray JNICALL
Java_com_yukinoshita_mobi_MobiBridge_getPageBytesNative(
        JNIEnv *env, jobject thiz, jint fd, jint index, jstring drm_pid_str) {

    const char *drm_pid = drm_pid_str ? (*env)->GetStringUTFChars(env, drm_pid_str, NULL) : NULL;
    MOBIData *m = NULL;
    MOBIRawml *rawml = NULL;
    size_t count = 0;

    MOBIPart** images = parseMobiCore(fd, drm_pid, &m, &rawml, &count);
    if (drm_pid) (*env)->ReleaseStringUTFChars(env, drm_pid_str, drm_pid);

    jbyteArray result = NULL;
    if (index >= 0 && (size_t)index < count && images) {
        MOBIPart* target = images[index];
        result = (*env)->NewByteArray(env, (jsize)target->size);
        (*env)->SetByteArrayRegion(env, result, 0, (jsize)target->size, (jbyte *)target->data);
    }

    if (images) free(images);
    if (rawml) mobi_free_rawml(rawml);
    if (m) mobi_free(m);

    return result;
}
