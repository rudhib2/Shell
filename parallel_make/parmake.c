/**
 * parallel_make
 * CS 341 - Fall 2023
 */

//used ChatGPT for initial design, debugging, and help with writing functions - worked well

#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"
#include "set.h"
#include "vector.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "queue.h"
#include "dictionary.h"


set* already_visited;
graph* full_graph;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
queue *q;
int failed = 0;


bool cycle(dictionary *history, void *target) {
    int *var = dictionary_get(history, target);
    if (*var == 1) {
        return true;
    }
    if (*var == 2) {
        return false;
    }
    *var = 1;
    vector *neighbors = graph_neighbors(full_graph, target);
    bool hasCycle = false;
    for (size_t i = 0; i < vector_size(neighbors); ++i) {
        if (cycle(history, vector_get(neighbors, i))) {
            hasCycle = true;
            break;
        }
    }
    *var = 2;
    vector_destroy(neighbors);
    return hasCycle;
}


bool detection(void *target) {
    if (full_graph == NULL || !graph_contains_vertex(full_graph, target)) {
        return false;
    }
    dictionary *dict = string_to_int_dictionary_create();
    vector *keys = graph_vertices(full_graph);
    for (size_t i = 0; i < vector_size(keys); ++i) {
        dictionary_set(dict, vector_get(keys, i), &(int){0});
    }
    vector_destroy(keys);
    bool result = cycle(dict, target);
    dictionary_destroy(dict);
    return result;
}


int all_satisfied(vector *neighbors) {
  size_t i;
  for (i = 0; i < vector_size(neighbors); i++) {
    rule_t *rule_nbr = (rule_t *) graph_get_vertex_value(full_graph, vector_get(neighbors, i));
    if ((rule_nbr->state) == 0) {
        return 0;
    } 
  }
  return 1;
}


int runningCommands(char* vtx) {
    int flag = access(vtx, F_OK) < 0 ? 1 : 0;
    vector *neighbors = graph_neighbors(full_graph, vtx);
    for (size_t i = 0; i < vector_size(neighbors); i++) {
        char* neighbor = vector_get(neighbors, i);
        int neighbor_exists = access(neighbor, F_OK);
        int vtx_exists = access(vtx, F_OK);
        if (!flag && neighbor_exists < 0) {
            flag = 1;
        } else if (flag == 0 && neighbor_exists == 0 && vtx_exists == 0) {
            struct stat vtx_stat, neighbor_stat;
            if (stat(vtx, &vtx_stat) != 0 || stat(neighbor, &neighbor_stat) != 0) {
                flag = 1;
            } else if ((vtx_stat.st_mtime-neighbor_stat.st_mtime) < 0) {
                flag = 1;
            }
        }
        rule_t *rule_nbr = (rule_t *) graph_get_vertex_value(full_graph, neighbor);
        if (!(rule_nbr->state) && runningCommands(neighbor)) {
            return 1;
        }
    }
    if (vector_size(neighbors) > 0) {
        while (!all_satisfied(neighbors)) {
            pthread_cond_wait(&cv, &m);
        }
    }
    vector_destroy(neighbors);
    if (failed) {
        return 1;
    }
    rule_t *rule = (rule_t *) graph_get_vertex_value(full_graph, vtx);
    if (flag && !(rule->state)) {
        queue_push(q, rule);
    } else {
        rule->state = 1;
    }
    return 0;
}


void *myfunc() {
    rule_t *rule;
    while ((rule = queue_pull(q)) != NULL) {
        bool rule_failed = false;
        for (size_t i = 0; i < vector_size(rule->commands); i++) {
            if (system(vector_get(rule->commands, i)) != 0) {
                failed = 1;
                rule_failed = true;
                break;
            }
        }
        if (!rule_failed) {
            rule->state = 1;
        }
        pthread_cond_signal(&cv);
    }
    return NULL;
}



int parmake(char *makefile, size_t num_threads, char **targets) {
    q = queue_create(-1);
    pthread_t tid[num_threads];
    for (size_t i = 0; i < num_threads; i++) {
        pthread_create(&tid[i], NULL, myfunc, NULL);
    }
    full_graph = parser_parse_makefile(makefile, targets);
    vector *goals = graph_neighbors(full_graph, "");
    for (size_t i = 0; i < vector_size(goals); i++) {
        char *goal = vector_get(goals, i);
        if (!detection(goal)) {
            if (runningCommands(goal)) {
                failed = 0;
            }
        } else {
            print_cycle_failure(goal);
        }
    }
    queue_push(q, NULL);
    for (size_t j = 0; j < num_threads; j++) {
        pthread_join(tid[j], NULL);
    }
    vector_destroy(goals);
    graph_destroy(full_graph);
    queue_destroy(q);
    return 0;
}


// vector *parentOfCyclic(char *vertex) {
//     vector* vect = shallow_vector_create();
//     for(size_t i = 0; i < vector_size(graph_antineighbors(full_graph, vertex)); ++i) {
//         vector_push_back(vect, vector_get(graph_antineighbors(full_graph, vertex),i));
//         parentOfCyclic(vector_get(graph_antineighbors(full_graph, vertex),i));
//     }  
//     return vect;
// }


// bool *noRelation(char* vertex) {
//     vector* temp = graph_antineighbors(full_graph, vertex);
//     vector* temp2 = graph_neighbors(full_graph, vertex);
//     if(vector_size(temp) == 0 && vector_size(temp2) == 0) {
//         return true;
//     }
//     return false;
// }

// int parmake(char *makefile, size_t num_threads, char **targets) {
//     // good luck!
//     graph *directed_graph = parser_parse_makefile(makefile, targets);
//     vector *vertices = graph_vertices(directed_graph);
//     for (size_t i = 0; i < vector_size(vertices); ++i) {
//         if(detectingCycles(vector_get(vertices, i)) == true) {
//             vector* vect = parentOfCyclic(vector_get(vertices, i));
//             for(size_t j = 0; j < vector_size(vect); ++j) {
//                 for (size_t k = 0; k < vector_size(vertices); ++k) {
//                     if(vector_get(vertices, k) == vector_get(vect,j)) {
//                         vector_erase(vertices, k);
//                     }
//                 }
//             }
//             print_cycle_failure(vector_get(vertices, i));
//             vector_erase(vertices, i);
//         }
//         if(noRelation(vector_get(vertices, i)) == true) {
//             vector_erase(vertices, i);
//         }
//     }
//     return 0;
// }