//
// Created by ran on 2024/1/25.
//
#include "unionfind/union_find.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

static int find(UnionFind *uf, int p);

struct UnionFind {
  int *parent; // parent[i] means the i-th element's parent node
  int *rank;  // rank[i] means the rank of tree rooted by i
  int size;   // size of elements
};

UnionFind *create_uf(int size) {
  UnionFind *uf = malloc(sizeof(UnionFind));
  if (!uf) return NULL;
  uf->size = size;
  int *p = malloc(sizeof(int) * size);
  if (!p) {
    free(uf);
    return NULL;
  }
  int *r = malloc(sizeof(int) * size);
  if (!r) {
    free(p);
    free(uf);
    return NULL;
  }
  for (int i = 0; i < size; ++i) {
    p[i] = i;
    r[i] = 1;
  }
  uf->parent = p;
  uf->rank = r;
  return uf;
}

void free_uf(UnionFind *uf) {
  if (uf) {
    free(uf->rank);
    free(uf->parent);
    free(uf);
  }
}

int uf_size(UnionFind *uf) {
  return uf->size;
}

/**
 * Make p and q in same set.
 *
 * @param p
 * @param q
 * @return
 */
void uf_union(UnionFind *uf, int p, int q) {
  int p_root = find(uf, p);
  int q_root = find(uf, q);
  // merge low rank set to high rank set
  if (uf->rank[p_root] < uf->rank[q_root]) {
    uf->parent[p_root] = q_root;
  } else if (uf->rank[p_root] > uf->rank[q_root]) {
    uf->parent[q_root] = p_root;
  } else {
    uf->parent[p_root] = q_root;
    (uf->rank[q_root]) += 1;
  }
}

/**
 * Test if q and q is in same set.
 *
 * @param p
 * @param q
 * @return  0 if not in same set, otherwise 1.
 */
int uf_same_set(UnionFind *uf, int p, int q) {
  return find(uf, p) == find(uf, q);
}

/**
 * find the set id of element p
 *
 * @param uf
 * @param p
 * @return
 */
static int find(UnionFind *uf, int p) {
  assert(p >= 0 && p < uf->size);
  while (p != uf->parent[p]) {
    uf->parent[p] = uf->parent[uf->parent[p]];
    p = uf->parent[p];
  }
  return p;
}