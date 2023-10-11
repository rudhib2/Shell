/**
 * deadlock_demolition
 * CS 341 - Fall 2023
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>

struct drm_t {
  pthread_mutex_t m;
};

bool checkForCycle(void* node);

graph *g = NULL;
pthread_mutex_t use = PTHREAD_MUTEX_INITIALIZER;
set *visited = NULL;

bool checkForCycle(void* node) {
  if (visited == NULL) {
    visited = shallow_set_create();
  }
  if (set_contains(visited, node)) {
      visited = NULL;
      return true;
  } else {
    set_add(visited, node);
    vector *neighbors = graph_neighbors(g, node);
    for (size_t i = 0; i < vector_size(neighbors); i++) {
      if (checkForCycle(vector_get(neighbors, i))) {
        return true;
      }
    }
  }
  visited = NULL;
  return false;
}


drm_t *drm_init() {
  /* Your code here */
  drm_t *tr = malloc(sizeof(drm_t));
  pthread_mutex_init(&tr->m, NULL);
  pthread_mutex_lock(&use);
  if (g == NULL) {
    g = shallow_graph_create();
  }
  graph_add_vertex(g, tr);
  pthread_mutex_unlock(&use);
  return tr;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    int retval = 1;
    pthread_mutex_lock(&use);
    if (!graph_contains_vertex(g, thread_id) || !graph_contains_vertex(g, drm)) {
      retval = 0;
    } else if (graph_adjacent(g, drm, thread_id)) {
      graph_remove_edge(g, drm, thread_id);
      pthread_mutex_unlock(&drm->m);
    }
    pthread_mutex_unlock(&use);
    return retval;
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    int retval = 0;
    pthread_mutex_lock(&use);
    graph_add_vertex(g, thread_id);
    if (graph_adjacent(g, drm, thread_id)) {
      pthread_mutex_unlock(&use);
      return retval;
    } else {
      graph_add_edge(g, thread_id, drm);
      if (checkForCycle(thread_id)) {
        graph_remove_edge(g, thread_id, drm);
        pthread_mutex_unlock(&use);
        return retval;
      } else {
        pthread_mutex_unlock(&use);
        pthread_mutex_lock(&drm->m);
        pthread_mutex_lock(&use);
        graph_remove_edge(g, thread_id, drm);
        graph_add_edge(g, drm, thread_id);
        pthread_mutex_unlock(&use);
        retval = 1;
      }
    }
    return retval;
}

void drm_destroy(drm_t *drm) {
    /* Your code here */
  pthread_mutex_destroy(&drm->m);
  pthread_mutex_destroy(&use);
  graph_remove_vertex(g, drm);
  free(drm);
}