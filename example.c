#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mphf.h"

// 生成互不相同的随机字符串
void generate_unique_random_strings(char **keys, int key_count, int min_len, int max_len) {
    for (int i = 0; i < key_count; ) {
        int len = min_len + rand() % (max_len - min_len + 1);
        for (int j = 0; j < len; j++) {
            keys[i][j] = 'a' + rand() % 26;
        }
        keys[i][len] = '\0';
        // 检查是否重复
        int duplicate = 0;
        for (int k = 0; k < i; k++) {
            if (strcmp(keys[i], keys[k]) == 0) {
                duplicate = 1;
                break;
            }
        }
        if (!duplicate) {
            i++;
        }
    }
}

#ifdef _WIN32
#include <Windows.h>
#endif

int main() {
#ifdef _WIN32
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
#endif

    srand(time(NULL));
    int key_count = 10;  // 字符串数量
    int min_len = 1, max_len = 3;  // 字符串最小/最大长度
    
    // 生成随机字符串
    char **keys = (char **)malloc(key_count * sizeof(char *));
    for (int i = 0; i < key_count; i++) {
        keys[i] = (char *)malloc((max_len + 1) * sizeof(char));
    }
    generate_unique_random_strings(keys, key_count, min_len, max_len);

    printf("生成 %d 个随机字符串:\n", key_count);
    for(int i = 0; i < key_count; i++) {
        printf("index %d key : %s\n", i, keys[i]);
    }
    
    // 创建并构建MPHF
    mphf_t mphf;
    if (mphf_build((const char**)keys, key_count, &mphf)) {
        printf("\nMPHF构建成功！\n");
        printf("种子1: %llu\n", mphf.seed1);
        printf("种子2: %llu\n", mphf.seed2);
        
        // 验证哈希冲突
        printf("\n验证哈希冲突:\n");
        int *hash_values = (int *)calloc(key_count, sizeof(int));
        int conflict = 0;
        for (int i = 0; i < key_count; i++) {
            int h = mphf_hash(&mphf, keys[i]);
            printf("i : %d hash %d key : %s\n", i, h, keys[i]);
            if (hash_values[h]) {
                printf("  ! 冲突: 与字符串 %s 哈希值相同(%d)\n",
                       keys[hash_values[h]-1], h);
                conflict = 1;
            } else {
                hash_values[h] = i+1;
            }
        }
        if (!conflict) {
            printf("验证通过，无哈希冲突\n");
        } else {
            printf("警告: 检测到哈希冲突\n");
        }
        free(hash_values);
        
        // 释放MPHF资源
        mphf_free(&mphf);
    } else {
        printf("\n达到最大重试次数，无法生成无环图\n");
    }
    
    // 清理资源
    for (int i = 0; i < key_count; i++) {
        free(keys[i]);
    }
    free(keys);
    
    return 0;
}