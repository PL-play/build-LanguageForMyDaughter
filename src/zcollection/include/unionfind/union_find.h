//
// Created by ran on 2024/1/25.
//

#ifndef ZCOLLECTION_UNIONFIND_UNION_FIND_H_
#define ZCOLLECTION_UNIONFIND_UNION_FIND_H_
#ifdef __cplusplus
extern "C" {
#endif
typedef struct UnionFind UnionFind;

UnionFind *create_uf(int size);
void free_uf(UnionFind *);

int uf_size(UnionFind *);

/**
 * Make p and q in same set.
 *
 * @param p
 * @param q
 * @return
 */
void uf_union(UnionFind *, int p, int q);

/**
 * Test if q and q is in same set.
 *
 * @param p
 * @param q
 * @return  0 if not in same set, otherwise 1.
 */
int uf_same_set(UnionFind *, int p, int q);
#ifdef __cplusplus
}
#endif
#endif //ZCOLLECTION_UNIONFIND_UNION_FIND_H_
