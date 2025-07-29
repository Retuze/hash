#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define DEBUG 0  // 调试开关，1为开启，0为关闭

typedef struct adj_list
{
    int *nodes;   // 邻接顶点数组
    int *indexs;  // 边的索引数组
    int capacity; // 节点容量
    int size;     // 节点大小
} adj_list;

typedef struct graph
{
    int node_num;
    int edge_num;
    int edge_num_max;
    int **edges;
    adj_list *list;
    bool *node_exists;
} graph;

// 初始化图结构体
void graph_init(graph *g, int edge_num_max)
{
    g->node_num = 0;
    g->edge_num = 0;
    g->edge_num_max = edge_num_max;
    g->edges = (int **)malloc(edge_num_max * sizeof(int *));
    int edge_has_node_num = 2;
    for (int i = 0; i < edge_num_max; i++)
    {
        g->edges[i] = (int *)malloc(edge_has_node_num * sizeof(int));
    }
    g->list = (adj_list *)malloc(edge_num_max * 2 * sizeof(adj_list));
    int node_capacity = 2;
    for (int i = 0; i < edge_num_max * 2; i++)
    {
        g->list[i].nodes = (int *)malloc(node_capacity * sizeof(int));
        g->list[i].indexs = (int *)malloc(node_capacity * sizeof(int));
        g->list[i].capacity = node_capacity;
        g->list[i].size = 0;
    }
    g->node_exists = (bool *)calloc(edge_num_max * 2, sizeof(bool));
}

void graph_free(graph *g)
{
    for (int i = 0; i < g->edge_num_max; i++)
    {
        free(g->edges[i]);
    }
    free(g->edges);
    for (int i = 0; i < g->edge_num_max * 2; i++)
    {
        free(g->list[i].nodes);
        free(g->list[i].indexs);
    }
    free(g->list);
    free(g->node_exists);
}

// 添加边到图中，返回true表示添加成功，false表示重复或自环
bool graph_add_edge(graph *g, int u, int v)
{
    if (g->edge_num >= g->edge_num_max)
    {
        return false;
    }
    if (u == v) {
        return false;
    }
    for (int i = 0; i < g->list[u].size; i++)
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
        list_u->nodes = (int *)realloc(list_u->nodes, list_u->capacity * sizeof(int));
        list_u->indexs = (int *)realloc(list_u->indexs, list_u->capacity * sizeof(int));
    }

    if (list_v->size >= list_v->capacity)
    {
        list_v->capacity *= 2;
        list_v->nodes = (int *)realloc(list_v->nodes, list_v->capacity * sizeof(int));
        list_v->indexs = (int *)realloc(list_v->indexs, list_v->capacity * sizeof(int));
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

// dfs遍历无向图，判断是否有环
bool graph_dfs(graph *g, int node, int parent, int *visited)
{
    visited[node] = 1;

    for (int i = 0; i < g->list[node].size; i++)
    {
        int neighbor = g->list[node].nodes[i];
        if (!visited[neighbor])
        {
            if (graph_dfs(g, neighbor, node, visited))
            {
                return true;
            }
        }
        else if (neighbor != parent)
        {
            return true;
        }
    }
    return false;
}

// 按顺序输出无向图剥离后的边（无环则输出全部边的剥离顺序，有环则输出到无法剥离为止）
// removed_order: 输出边的索引顺序，removed_count: 实际输出的边数
// 返回值：true=有环，false=无环
bool graph_edge_removal_order(graph *g, int *removed_order, int *removed_count)
{
    int max_node = g->edge_num_max * 2;
    int *degree = (int *)calloc(max_node, sizeof(int));
    int *edge_removed = (int *)calloc(g->edge_num_max, sizeof(int));
    int *queue = (int *)malloc(max_node * sizeof(int));
    int front = 0, rear = 0;

    for (int i = 0; i < max_node; i++) {
        if (g->node_exists[i]) {
            degree[i] = g->list[i].size;
        }
    }

    for (int i = 0; i < max_node; i++) {
        if (g->node_exists[i] && degree[i] == 1) {
            queue[rear++] = i;
        }
    }

    int processed_edges = 0;
    *removed_count = 0;

    while (front < rear) {
        int node = queue[front++];
        if (degree[node] == 0) continue;

        for (int i = 0; i < g->list[node].size; i++) {
            int neighbor = g->list[node].nodes[i];
            int edge_idx = g->list[node].indexs[i];
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

// 哈希函数
static int hash_function(const char* key, unsigned long long seed, int range) {
    if (range <= 0) range = 1;
    unsigned long long hash = seed;
    for (int i = 0; key[i] != '\0'; i++) {
        unsigned long long weighted_char = (unsigned char)key[i];
        weighted_char ^= (hash >> 33);
        weighted_char *= 0xff51afd7ed558ccdULL;
        weighted_char ^= (weighted_char >> 33);
        weighted_char *= 0xc4ceb9fe1a85ec53ULL;
        hash ^= weighted_char;
        hash = (hash << 31) | (hash >> 33);
        hash *= 0x9e3779b97f4a7c15ULL;
    }
    return (int)((hash & 0x7FFFFFFF) % range);
}

typedef struct mphf_t {
    int *g_value;            // g数组
    int g_size;              // g数组长度
    unsigned long long seed1;
    unsigned long long seed2;
} mphf_t;

// 检查字符串数组中是否有重复
int cmp_str(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

bool has_duplicate_keys(const char* keys[], int key_count) {
    char **tmp = (char **)malloc(key_count * sizeof(char *));
    for (int i = 0; i < key_count; i++) tmp[i] = (char *)keys[i];
    qsort(tmp, key_count, sizeof(char *), cmp_str);
    for (int i = 1; i < key_count; i++) {
        if (strcmp(tmp[i], tmp[i-1]) == 0) {
            free(tmp);
            return true;
        }
    }
    free(tmp);
    return false;
}

// 构建MPHF，输出g_value和种子
bool mphf_build(const char* keys[], int key_count, mphf_t* mphf) {
    int table_size = key_count * 2;
    int max_retry = 1000;
    srand((unsigned int)time(NULL));
    int* removed_order = (int*)malloc(key_count * sizeof(int));
    int removed_count = 0;

    if (has_duplicate_keys(keys, key_count)) {
        printf("error: duplicate key exists, abort build.\n");
        free(removed_order);
        return false;
    }

    for (int retry = 0; retry < max_retry; retry++) {
        unsigned long long seed1 = ((unsigned long long)rand() << 32) | rand();
        unsigned long long seed2 = ((unsigned long long)rand() << 32) | rand();

        graph g;
        graph_init(&g, key_count);

#if DEBUG
        printf("\n尝试 %d: 使用种子 %llu 和 %llu\n", retry+1, seed1, seed2);
        printf("生成的边:\n");
#endif

        bool self_loop = false;
        for (int i = 0; i < key_count; i++) {
            int v1 = hash_function(keys[i], seed1, table_size);
            int v2 = hash_function(keys[i], seed2, table_size);
            
            if (v1 == v2) {
#if DEBUG
                printf("字符串 %s 生成自环顶点 %d-%d\n", keys[i], v1, v2);
#endif
                self_loop = true;
                break;
            }
            
            if (!graph_add_edge(&g, v1, v2)) {
#if DEBUG
                printf("边 %d - %d 已存在 (来自字符串: %s)\n", v1, v2, keys[i]);
#endif
                self_loop = true;
                break;
            }
#if DEBUG
            printf("添加边 %d: %d - %d (来自字符串: %s)\n",
                  g.edge_num-1, v1, v2, keys[i]);
#endif
        }
        if (self_loop) {
            graph_free(&g);
            continue;
        }

        removed_count = 0;
        bool has_cycle = graph_edge_removal_order(&g, removed_order, &removed_count);

#if DEBUG
        printf("\n边剥离顺序:\n");
        for (int i = 0; i < removed_count; i++) {
            int edge_idx = removed_order[i];
            printf("%d: %d - %d (来自字符串: %s)\n",
                  i, g.edges[edge_idx][0], g.edges[edge_idx][1], keys[edge_idx]);
        }
        printf("\n图%s环\n", has_cycle ? "有" : "无");
#endif

        if (!has_cycle && removed_count == key_count) {
            // 分配g_value数组
            int* g_value = (int*)calloc(table_size, sizeof(int));
            int* used = (int*)calloc(table_size, sizeof(int));

#if DEBUG
            printf("\n开始分配g_value:\n");
#endif
            // 逆序处理剥离顺序，分配g_value
            for (int i = removed_count - 1; i >= 0; i--) {
                int edge_idx = removed_order[i];
                int v1 = g.edges[edge_idx][0];
                int v2 = g.edges[edge_idx][1];
                
#if DEBUG
                printf("\n处理边 %d: %d-%d (来自字符串: %s)\n",
                      edge_idx, v1, v2, keys[edge_idx]);
#endif
                if (!used[v1] && !used[v2]) {
                    g_value[v1] = 0;
                    used[v1] = 1;
                    g_value[v2] = edge_idx;
                    used[v2] = 1;
#if DEBUG
                    printf("  - 分配 g[%d] = 0 (初始值)\n", v1);
                    printf("  - 分配 g[%d] = %d (边序号)\n", v2, edge_idx);
#endif
                } else if (!used[v1]) {
                    g_value[v1] = edge_idx - g_value[v2];
                    used[v1] = 1;
#if DEBUG
                    printf("  - 分配 g[%d] = %d (边序号 %d - g[%d] %d)\n",
                          v1, g_value[v1], edge_idx, v2, g_value[v2]);
#endif
                } else {
                    g_value[v2] = edge_idx - g_value[v1];
                    used[v2] = 1;
#if DEBUG
                    printf("  - 分配 g[%d] = %d (边序号 %d - g[%d] %d)\n",
                          v2, g_value[v2], edge_idx, v1, g_value[v1]);
#endif
                }
            }

#if DEBUG
            printf("\n最终g_value分配结果:\n");
            for (int i = 0; i < table_size; i++) {
                if (used[i]) {
                    printf("顶点 %d: g_value = %d\n", i, g_value[i]);
                }
            }
#endif

            mphf->g_value = g_value;
            mphf->g_size = table_size;
            mphf->seed1 = seed1;
            mphf->seed2 = seed2;

            free(used);
            free(removed_order);
            graph_free(&g);
            return true;
        }
        graph_free(&g);
    }
    free(removed_order);
    return false;
}

// 查询MPHF哈希值
int mphf_hash(const mphf_t* mphf, const char* key) {
    int v1 = hash_function(key, mphf->seed1, mphf->g_size);
    int v2 = hash_function(key, mphf->seed2, mphf->g_size);
    int key_count = mphf->g_size / 2;
    return (mphf->g_value[v1] + mphf->g_value[v2]) % key_count;
}

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
#include <Windows.h>
// 使用随机字符串生成hash顶点并测试图
int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    srand(time(NULL));
    int key_count = 100000;  // 字符串数量
    int min_len = 1, max_len = 16;  // 字符串最小/最大长度
    int table_size = key_count * 2;  // 顶点数量
    
    // 生成随机字符串
    char **keys = (char **)malloc(key_count * sizeof(char *));
    for (int i = 0; i < key_count; i++) {
        keys[i] = (char *)malloc((max_len + 1) * sizeof(char));
    }
    generate_unique_random_strings(keys, key_count, min_len, max_len);

    printf("生成 %d 个随机字符串:\n", key_count);
    
    mphf_t mphf;
    if (mphf_build((const char**)keys, key_count, &mphf)) {
#if 1
        printf("\nMPHF构建成功！\n");
        printf("种子1: %llu\n", mphf.seed1);
        printf("种子2: %llu\n", mphf.seed2);
        
        // 验证哈希冲突
        printf("\n验证哈希冲突:\n");
        int *hash_values = (int *)calloc(key_count, sizeof(int));
        int conflict = 0;
        for (int i = 0; i < key_count; i++) {
            int h = mphf_hash(&mphf, keys[i]);
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
        free(mphf.g_value);
#endif
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