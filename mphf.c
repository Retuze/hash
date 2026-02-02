#include "mphf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

int mphf_last_error = 0;

/* 内部数据结构 */

typedef struct adj_list {
    int32_t *nodes;   // 邻接顶点数组
    int32_t *indexs;  // 边的索引数组
    int32_t capacity; // 节点容量
    int32_t size;     // 节点大小
} adj_list;

typedef struct graph {
    int32_t node_num;
    int32_t edge_num;
    int32_t edge_num_max;
    int32_t **edges;
    adj_list *list;
    bool *node_exists;
} graph;

/* 内部函数声明 */

static void graph_init(graph *g, int32_t edge_num_max);
static void graph_free(graph *g);
static bool graph_add_edge(graph *g, int32_t u, int32_t v);
static bool graph_edge_removal_order(graph *g, int32_t *removed_order, int32_t *removed_count);
static int32_t hash_function(const void* key, int32_t key_len, uint64_t seed, int32_t range);
static bool has_duplicate_keys(const void* keys[], const int32_t key_lens[], int32_t key_count);

/* 图操作实现 */

static void graph_init(graph *g, int32_t edge_num_max)
{
    g->node_num = 0;
    g->edge_num = 0;
    g->edge_num_max = edge_num_max;
    g->edges = (int32_t **)malloc(edge_num_max * sizeof(int32_t *));
    int32_t edge_has_node_num = 2;
    for (int32_t i = 0; i < edge_num_max; i++)
    {
        g->edges[i] = (int32_t *)malloc(edge_has_node_num * sizeof(int32_t));
    }
    g->list = (adj_list *)malloc(edge_num_max * 2 * sizeof(adj_list));
    int32_t node_capacity = 2;
    for (int32_t i = 0; i < edge_num_max * 2; i++)
    {
        g->list[i].nodes = (int32_t *)malloc(node_capacity * sizeof(int32_t));
        g->list[i].indexs = (int32_t *)malloc(node_capacity * sizeof(int32_t));
        g->list[i].capacity = node_capacity;
        g->list[i].size = 0;
    }
    g->node_exists = (bool *)calloc(edge_num_max * 2, sizeof(bool));
}

static void graph_free(graph *g)
{
    for (int32_t i = 0; i < g->edge_num_max; i++)
    {
        free(g->edges[i]);
    }
    free(g->edges);
    for (int32_t i = 0; i < g->edge_num_max * 2; i++)
    {
        free(g->list[i].nodes);
        free(g->list[i].indexs);
    }
    free(g->list);
    free(g->node_exists);
}

static bool graph_add_edge(graph *g, int32_t u, int32_t v)
{
    if (g->edge_num >= g->edge_num_max)
    {
        return false;
    }
    if (u == v) {
        return false;
    }
    for (int32_t i = 0; i < g->list[u].size; i++)
    {
        if (g->list[u].nodes[i] == v)
        {
            return false;
        }
    }

    if (!g->node_exists[u]) {
        g->node_exists[u] = true;
        g->node_num++;
    }
    if (!g->node_exists[v]) {
        g->node_exists[v] = true;
        g->node_num++;
    }

    adj_list *list_u = &g->list[u];
    adj_list *list_v = &g->list[v];

    if (list_u->size >= list_u->capacity)
    {
        list_u->capacity *= 2;
        list_u->nodes = (int32_t *)realloc(list_u->nodes, list_u->capacity * sizeof(int32_t));
        list_u->indexs = (int32_t *)realloc(list_u->indexs, list_u->capacity * sizeof(int32_t));
    }

    if (list_v->size >= list_v->capacity)
    {
        list_v->capacity *= 2;
        list_v->nodes = (int32_t *)realloc(list_v->nodes, list_v->capacity * sizeof(int32_t));
        list_v->indexs = (int32_t *)realloc(list_v->indexs, list_v->capacity * sizeof(int32_t));
    }

    list_u->nodes[list_u->size] = v;
    list_u->indexs[list_u->size] = g->edge_num;
    list_u->size++;

    list_v->nodes[list_v->size] = u;
    list_v->indexs[list_v->size] = g->edge_num;
    list_v->size++;

    g->edges[g->edge_num][0] = u;
    g->edges[g->edge_num][1] = v;
    g->edge_num++;
    return true;
}

static bool graph_edge_removal_order(graph *g, int32_t *removed_order, int32_t *removed_count)
{
    int32_t max_node = g->edge_num_max * 2;
    int32_t *degree = (int32_t *)calloc(max_node, sizeof(int32_t));
    int32_t *edge_removed = (int32_t *)calloc(g->edge_num_max, sizeof(int32_t));
    int32_t *queue = (int32_t *)malloc(max_node * sizeof(int32_t));
    int32_t front = 0, rear = 0;

    for (int32_t i = 0; i < max_node; i++) {
        if (g->node_exists[i]) {
            degree[i] = g->list[i].size;
        }
    }

    for (int32_t i = 0; i < max_node; i++) {
        if (g->node_exists[i] && degree[i] == 1) {
            queue[rear++] = i;
        }
    }

    int32_t processed_edges = 0;
    *removed_count = 0;

    while (front < rear) {
        int32_t node = queue[front++];
        if (degree[node] == 0) continue;

        for (int32_t i = 0; i < g->list[node].size; i++) {
            int32_t neighbor = g->list[node].nodes[i];
            int32_t edge_idx = g->list[node].indexs[i];
            if (edge_removed[edge_idx]) continue;

            removed_order[(*removed_count)++] = edge_idx;
            edge_removed[edge_idx] = 1;
            processed_edges++;

            degree[neighbor]--;
            if (degree[neighbor] == 1) {
                queue[rear++] = neighbor;
            }
            degree[node]--;
            break;
        }
    }

    free(degree);
    free(edge_removed);
    free(queue);

    if (processed_edges != g->edge_num) {
        return true;
    }
    return false;
}

/* 哈希函数和辅助函数 */

static int32_t hash_function(const void* key, int32_t key_len, uint64_t seed, int32_t range) {
    if (range <= 0) range = 1;
    uint64_t hash = seed;
    const uint8_t* p = (const uint8_t*)key;
    for (int32_t i = 0; i < key_len; i++) {
        uint64_t weighted_char = p[i];
        weighted_char ^= (hash >> 33);
        weighted_char *= 0xff51afd7ed558ccdULL;
        weighted_char ^= (weighted_char >> 33);
        weighted_char *= 0xc4ceb9fe1a85ec53ULL;
        hash ^= weighted_char;
        hash = (hash << 31) | (hash >> 33);
        hash *= 0x9e3779b97f4a7c15ULL;
    }
    return (int32_t)((hash & 0x7FFFFFFF) % range);
}

static int cmp_buf(const void *a, const void *b, void* lens) {
    const void* pa = *(const void**)a;
    const void* pb = *(const void**)b;
    const int32_t* lens_arr = (const int32_t*)lens;
    int32_t ia = (int32_t)((const void**)a - (const void**)lens_arr);
    int32_t ib = (int32_t)((const void**)b - (const void**)lens_arr);
    int32_t la = lens_arr[ia];
    int32_t lb = lens_arr[ib];
    int32_t cmp = memcmp(pa, pb, la < lb ? la : lb);
    if (cmp != 0) return cmp;
    return la - lb;
}

typedef struct {
    const void* key_ptr;
    int32_t key_len;
} _dup_key_t;

static int _dup_cmp(const void* a, const void* b) {
    const _dup_key_t* ia = (const _dup_key_t*)a;
    const _dup_key_t* ib = (const _dup_key_t*)b;
    if (ia->key_len != ib->key_len) return ia->key_len - ib->key_len;
    return memcmp(ia->key_ptr, ib->key_ptr, ia->key_len);
}

static bool has_duplicate_keys(const void* key_ptrs[], const int32_t key_lens[], int32_t num_keys) {
    _dup_key_t* arr = (_dup_key_t*)malloc(num_keys * sizeof(_dup_key_t));
    for (int32_t i = 0; i < num_keys; i++) {
        arr[i].key_ptr = key_ptrs[i];
        arr[i].key_len = key_lens[i];
    }
    qsort(arr, num_keys, sizeof(_dup_key_t), _dup_cmp);
    for (int32_t i = 1; i < num_keys; i++) {
        if (_dup_cmp(&arr[i-1], &arr[i]) == 0) {
            free(arr);
            return true;
        }
    }
    free(arr);
    return false;
}

/* 公开API实现 */

int mphf_build(mphf_t* mphf, const mphf_key_t* keys, int32_t num_keys) {
    mphf_last_error = MPHF_OK;
    if (!mphf || !keys || num_keys <= 0) {
        mphf_last_error = MPHF_ERR_PARAM;
        fprintf(stderr, "[MPHF_ERR_%d] invalid parameter: mphf=%p, keys=%p, num_keys=%d\n", mphf_last_error, (void*)mphf, (void*)keys, num_keys);
        return MPHF_ERR_PARAM;
    }
    int32_t table_size = num_keys * 2;
    int32_t max_retry = 1000;

    static bool seed_initialized = false;
    if (!seed_initialized) {
        srand((unsigned int)time(NULL));
        seed_initialized = true;
    }

    int32_t* removed_order = (int32_t*)malloc(num_keys * sizeof(int32_t));
    int32_t removed_count = 0;

    // 构造key_ptrs和key_lens临时数组用于复用原有判重逻辑
    const void** key_ptrs = (const void**)malloc(num_keys * sizeof(void*));
    int32_t* key_lens = (int32_t*)malloc(num_keys * sizeof(int32_t));
    for (int32_t i = 0; i < num_keys; i++) {
        key_ptrs[i] = keys[i].key_ptr;
        key_lens[i] = keys[i].key_len;
    }
    if (has_duplicate_keys(key_ptrs, key_lens, num_keys)) {
        mphf_last_error = MPHF_ERR_DUPLICATE;
        fprintf(stderr, "[MPHF_ERR_%d] key duplicate detected, abort build.\n", mphf_last_error);
        free(removed_order);
        free(key_ptrs);
        free(key_lens);
        return MPHF_ERR_DUPLICATE;
    }

    for (int32_t retry = 0; retry < max_retry; retry++) {
        uint64_t seed1 = ((uint64_t)rand() << 32) | rand();
        uint64_t seed2 = ((uint64_t)rand() << 32) | rand();

        graph g;
        graph_init(&g, num_keys);

        bool self_loop = false;
        for (int32_t i = 0; i < num_keys; i++) {
            int32_t v1 = hash_function(keys[i].key_ptr, keys[i].key_len, seed1, table_size);
            int32_t v2 = hash_function(keys[i].key_ptr, keys[i].key_len, seed2, table_size);

            if (v1 == v2) {
                self_loop = true;
                break;
            }

            if (!graph_add_edge(&g, v1, v2)) {
                self_loop = true;
                break;
            }
        }
        if (self_loop) {
            graph_free(&g);
            continue;
        }

        removed_count = 0;
        bool has_cycle = graph_edge_removal_order(&g, removed_order, &removed_count);

        if (!has_cycle && removed_count == num_keys) {
            int32_t* g_value = (int32_t*)calloc(table_size, sizeof(int32_t));
            int32_t* used = (int32_t*)calloc(table_size, sizeof(int32_t));

            for (int32_t i = removed_count - 1; i >= 0; i--) {
                int32_t edge_idx = removed_order[i];
                int32_t v1 = g.edges[edge_idx][0];
                int32_t v2 = g.edges[edge_idx][1];
                if (!used[v1] && !used[v2]) {
                    g_value[v1] = 0;
                    used[v1] = 1;
                    g_value[v2] = edge_idx;
                    used[v2] = 1;
                } else if (!used[v1]) {
                    g_value[v1] = edge_idx - g_value[v2];
                    used[v1] = 1;
                } else {
                    g_value[v2] = edge_idx - g_value[v1];
                    used[v2] = 1;
                }
            }

            mphf->g_value = g_value;
            mphf->g_size = table_size;
            mphf->seed1 = seed1;
            mphf->seed2 = seed2;

            free(used);
            free(removed_order);
            graph_free(&g);
            free(key_ptrs);
            free(key_lens);
            return MPHF_OK;
        }
        graph_free(&g);
    }
    mphf_last_error = MPHF_ERR_MAXRETRY;
    fprintf(stderr, "[MPHF_ERR_%d] reach max retry, cannot build acyclic graph.\n", mphf_last_error);
    free(removed_order);
    free(key_ptrs);
    free(key_lens);
    return MPHF_ERR_MAXRETRY;
}

int32_t mphf_hash(const mphf_t* mphf, const void* key_ptr, int32_t key_len) {
    int32_t v1 = hash_function(key_ptr, key_len, mphf->seed1, mphf->g_size);
    int32_t v2 = hash_function(key_ptr, key_len, mphf->seed2, mphf->g_size);
    int32_t num_keys = mphf->g_size / 2;
    return (mphf->g_value[v1] + mphf->g_value[v2]) % num_keys;
}

void mphf_free(mphf_t* mphf) {
    if (mphf && mphf->g_value) {
        free(mphf->g_value);
        mphf->g_value = NULL;
    }
}