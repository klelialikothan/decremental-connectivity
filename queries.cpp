#include "LEDA/core/list.h"
#include "LEDA/graph/graph.h"
#include "LEDA/graph/node_array.h"
#include "LEDA/graph/basic_graph_alg.h"

#include "decremental_connectivity.h"

using leda::list;
using leda::graph;
using leda::node;
using leda::node_array;

bool dynamic_query(const node_array<node_info> &node_props, node a, node b){

    return node_props[a].component == node_props[b].component;

}

bool static_query(const graph &G, node a, node b){

    node_array<bool> reached(G, false);
    list<node> component = DFS(G, a, reached);

    return reached[b];

}