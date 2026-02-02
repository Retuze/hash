#ifndef MPHF_H
#define MPHF_H

#include <stdbool.h>
#include <stdint.h>

/**
 * 最小完美哈希函数结构体
 */

typedef struct mphf_t {
    int32_t *g_value;            // g数组
    int32_t g_size;              // g数组长度
    uint64_t seed1; // 第一个哈希种子
    uint64_t seed2; // 第二个哈希种子
} mphf_t;

typedef struct {
    const void* key_ptr;
    int32_t key_len;
} mphf_key_t;


#ifdef __cplusplus
extern "C" {
#endif

// 错误码定义
#define MPHF_OK 0
#define MPHF_ERR_DUPLICATE 1
#define MPHF_ERR_MAXRETRY 2
#define MPHF_ERR_PARAM 3 // 参数错误，如key_count为0等

extern int mphf_last_error;

/**
 * 构建最小完美哈希函数（结构体数组接口）
 * @param mphf 输出参数，存储构建的MPHF
 * @param keys key数组，每个元素包含指针和长度
 * @param num_keys key数量
 * @return 0成功，非0为错误码
 */
int mphf_build(mphf_t* mphf, const mphf_key_t* keys, int32_t num_keys);

/**
 * 使用MPHF计算buffer的哈希值
 * @param mphf MPHF结构体指针
 * @param key_ptr buffer指针
 * @param key_len buffer长度
 * @return 哈希值（范围为0到num_keys-1）
 *
 * 实现参考：
 * int32_t hash_function(const void* key_ptr, int32_t key_len, uint64_t seed, int32_t range) {
 *     if (range <= 0) range = 1;
 *     uint64_t hash = seed;
 *     const uint8_t* p = (const uint8_t*)key_ptr;
 *     for (int32_t i = 0; i < key_len; i++) {
 *         uint64_t weighted_char = p[i];
 *         weighted_char ^= (hash >> 33);
 *         weighted_char *= 0xff51afd7ed558ccdULL;
 *         weighted_char ^= (weighted_char >> 33);
 *         weighted_char *= 0xc4ceb9fe1a85ec53ULL;
 *         hash ^= weighted_char;
 *         hash = (hash << 31) | (hash >> 33);
 *         hash *= 0x9e3779b97f4a7c15ULL;
 *     }
 *     return (int32_t)((hash & 0x7FFFFFFF) % range);
 * }
 * int32_t mphf_hash(const mphf_t* mphf, const void* key_ptr, int32_t key_len) {
 *     int32_t v1 = hash_function(key_ptr, key_len, mphf->seed1, mphf->g_size);
 *     int32_t v2 = hash_function(key_ptr, key_len, mphf->seed2, mphf->g_size);
 *     int32_t num_keys = mphf->g_size / 2;
 *     return (mphf->g_value[v1] + mphf->g_value[v2]) % num_keys;
 * }
 */
int32_t mphf_hash(const mphf_t* mphf, const void* key_ptr, int32_t key_len);

/**
 * 释放MPHF结构体占用的内存
 * 
 * @param mphf 要释放的MPHF结构体指针
 */
void mphf_free(mphf_t* mphf);

#ifdef __cplusplus
}
#endif


#endif /* MPHF_H */