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

/**
 * 构建最小完美哈希函数
 * 
 * @param keys 字符串数组
 * @param key_count 字符串数量
 * @param mphf 输出参数，存储构建的MPHF
 * @return 构建是否成功
 */
bool mphf_build(const char* keys[], int key_count, mphf_t* mphf);

/**
 * 使用MPHF计算字符串的哈希值
 * 
 * @param mphf MPHF结构体指针
 * @param key 要计算哈希值的字符串
 * @return 哈希值（范围为0到key_count-1）
 */
int mphf_hash(const mphf_t* mphf, const char* key);

/**
 * 释放MPHF结构体占用的内存
 * 
 * @param mphf 要释放的MPHF结构体指针
 */
void mphf_free(mphf_t* mphf);

#endif /* MPHF_H */