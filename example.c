#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mphf.h"


// 生成所有中英文Unicode码点的UTF-8字符串
// 范围：ASCII可见字符（0x20~0x7E），CJK统一汉字（0x4E00~0x9FFF），扩展A/B/C/D/E/F，兼容汉字等
#include <stdint.h>



// 生成所有有效Unicode码点（U+0000~U+10FFFF，排除代理区）
int32_t generate_all_unicode_keys(mphf_key_t **keys) {
    int32_t total = 0;
    // 统计有效码点数（排除U+D800~U+DFFF代理区）
    for (int32_t cp = 0; cp <= 0x10FFFF; cp++) {
        if (cp >= 0xD800 && cp <= 0xDFFF) continue;
        total++;
    }
    *keys = (mphf_key_t *)malloc(total * sizeof(mphf_key_t));
    int32_t idx = 0;
    for (int32_t cp = 0; cp <= 0x10FFFF; cp++) {
        if (cp >= 0xD800 && cp <= 0xDFFF) continue;
        unsigned char *buf = (unsigned char *)malloc(4);
        int32_t key_len = 0;
        if (cp <= 0x7F) {
            buf[0] = cp;
            key_len = 1;
        } else if (cp <= 0x7FF) {
            buf[0] = 0xC0 | ((cp >> 6) & 0x1F);
            buf[1] = 0x80 | (cp & 0x3F);
            key_len = 2;
        } else if (cp <= 0xFFFF) {
            buf[0] = 0xE0 | ((cp >> 12) & 0x0F);
            buf[1] = 0x80 | ((cp >> 6) & 0x3F);
            buf[2] = 0x80 | (cp & 0x3F);
            key_len = 3;
        } else {
            buf[0] = 0xF0 | ((cp >> 18) & 0x07);
            buf[1] = 0x80 | ((cp >> 12) & 0x3F);
            buf[2] = 0x80 | ((cp >> 6) & 0x3F);
            buf[3] = 0x80 | (cp & 0x3F);
            key_len = 4;
        }
        (*keys)[idx].key_ptr = buf;
        (*keys)[idx].key_len = key_len;
        idx++;
    }
    return total;
}

#ifdef _WIN32
#include <Windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    mphf_key_t *keys = NULL;
    int32_t num_keys = generate_all_unicode_keys(&keys);
    printf("生成所有有效Unicode码点，共%d个\n", num_keys);

    mphf_t mphf;
    if (mphf_build(&mphf, keys, num_keys) == 0) {
        printf("\nMPHF构建成功！\n");
        printf("种子1: %llu\n", mphf.seed1);
        printf("种子2: %llu\n", mphf.seed2);

        printf("\n部分哈希值示例（前100个）：\n");
        for (int32_t i = 0; i < 100 && i < num_keys; i++) {
            int32_t h = mphf_hash(&mphf, keys[i].key_ptr, keys[i].key_len);
            printf("U+%06X hash=%d\n", i < 0xD800 ? i : i + 0x800, h);
        }

        mphf_free(&mphf);
    } else {
        printf("\n达到最大重试次数，无法生成无环图\n");
    }

    for (int32_t i = 0; i < num_keys; i++) {
        free((void*)keys[i].key_ptr);
    }
    free(keys);

    return 0;
}