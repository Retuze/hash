#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

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

        int* edge_to_key = (int*)malloc(key_count * sizeof(int));
        for (int i = 0; i < key_count; i++) edge_to_key[i] = -1;

        bool self_loop = false;
        for (int i = 0; i < key_count; i++) {
            int v1 = hash_function(keys[i], seed1, table_size);
            int v2 = hash_function(keys[i], seed2, table_size);
            if (v1 == v2) {
                self_loop = true;
                break;
            }
            if (!graph_add_edge(&g, v1, v2)) {
                self_loop = true;
                break;
            }
            edge_to_key[g.edge_num - 1] = i;
        }
        if (self_loop) {
            free(edge_to_key);
            graph_free(&g);
            continue;
        }

        removed_count = 0;
        bool has_cycle = graph_edge_removal_order(&g, removed_order, &removed_count);

        if (!has_cycle && removed_count == key_count) {
            // 分配g_value数组
            int* g_value = (int*)calloc(table_size, sizeof(int));
            int* used = (int*)calloc(table_size, sizeof(int));

            // 逆序处理剥离顺序，分配g_value
            for (int i = removed_count - 1; i >= 0; i--) {
                int edge_idx = removed_order[i];
                int v1 = g.edges[edge_idx][0];
                int v2 = g.edges[edge_idx][1];
                int key_idx = edge_to_key[edge_idx];
                if (key_idx == -1) continue;
                if (!used[v1]) {
                    g_value[v1] = (key_idx - g_value[v2] + key_count) % key_count;
                    used[v1] = 1;
                } else {
                    g_value[v2] = (key_idx - g_value[v1] + key_count) % key_count;
                    used[v2] = 1;
                }
            }

            mphf->g_value = g_value;
            mphf->g_size = table_size;
            mphf->seed1 = seed1;
            mphf->seed2 = seed2;

            free(used);
            free(edge_to_key);
            free(removed_order);
            graph_free(&g);
            return true;
        }
        free(edge_to_key);
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

// 示例用法
int main() {
    srand(12345);
    mphf_t mphf;
    int random_key_count = 10;
    int min_len = 6, max_len = 16;
    char **random_keys = (char **)malloc(random_key_count * sizeof(char *));
    for (int i = 0; i < random_key_count; i++) {
        random_keys[i] = (char *)malloc((max_len + 1) * sizeof(char));
    }
    generate_unique_random_strings(random_keys, random_key_count, min_len, max_len);

    for(int i = 0; i < random_key_count; i++) {
        printf("Random key %d: %s\n", i, random_keys[i]);
    }

    if (mphf_build((const char **)random_keys, random_key_count, &mphf)) {
        printf("MPHF build success!\n");

        for(int i = 0; i < random_key_count; i++) {
            int hash_value = mphf_hash(&mphf, random_keys[i]);
            printf("Key: %s, Hash: %d\n", random_keys[i], hash_value);
        }
        // 检查是否有冲突
        int *hash_used = (int *)calloc(random_key_count, sizeof(int));
        int conflict = 0;
        for (int i = 0; i < random_key_count; i++) {
            int h = mphf_hash(&mphf, random_keys[i]);
            if (hash_used[h]) {
                printf("Conflict at hash %d for key %s\n", h, random_keys[i]);
                conflict = 1;
                break;
            }
            hash_used[h] = 1;
        }
        if (!conflict) {
            printf("No conflict for 10000 random strings.\n");
        }
        free(hash_used);
        free(mphf.g_value);
    } else {
        printf("MPHF build failed!\n");
    }

    for (int i = 0; i < random_key_count; i++) {
        free(random_keys[i]);
    }
    free(random_keys);

    return 0;
}