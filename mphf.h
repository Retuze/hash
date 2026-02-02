#ifndef MPHF_H
#define MPHF_H

#include <stdbool.h>

/**
 * 最小完美哈希函数结构体
 */
typedef struct mphf_t {
    int *g_value;            // g数组
    int g_size;              // g数组长度
    unsigned long long seed1; // 第一个哈希种子
    unsigned long long seed2; // 第二个哈希种子
} mphf_t;


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const void* data;
    int len;
} mphf_item_t;

/**
 * 构建最小完美哈希函数（结构体数组接口）
 * @param mphf 输出参数，存储构建的MPHF
 * @param items key数组，每个元素包含指针和长度
 * @param key_count key数量
 * @return 构建是否成功
 */
bool mphf_build(mphf_t* mphf, const mphf_item_t* items, int key_count);

/**
 * 使用MPHF计算buffer的哈希值
 * @param mphf MPHF结构体指针
 * @param key buffer指针
 * @param key_len buffer长度
 * @return 哈希值（范围为0到key_count-1）
 */
int mphf_hash(const mphf_t* mphf, const void* key, int key_len);

#ifdef __cplusplus
}
#endif

/**
 * 释放MPHF结构体占用的内存
 * 
 * @param mphf 要释放的MPHF结构体指针
 */
void mphf_free(mphf_t* mphf);

#endif /* MPHF_H */