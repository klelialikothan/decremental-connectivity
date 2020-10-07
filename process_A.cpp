#include <tuple>

#include "LEDA/core/list.h"
#include "LEDA/core/stack.h"
#include "LEDA/graph/graph.h"
#include "LEDA/graph/node_array.h"

#include "decremental_connectivity.h"

using std::tuple;
using std::make_tuple;

using leda::list;
using leda::stack;
using leda::graph;
using leda::node;
using leda::edge;
using leda::node_array;

tuple<bool, bool, node> process_A_step(const graph &G, node_array<node_info> &node_props, node a, node b, stack<node> &dfs_stack_a, stack<node> &dfs_stack_b, list<node> &component_a, list<node> &component_b){

    // DFS(source -> node a, target -> node b)
    if (!dfs_stack_a.empty()){
        node v = dfs_stack_a.pop();
        if (!node_props[v].visited_a){
            // b not found, continue
            if (v != b){
                component_a.append(v);
                node_props[v].visited_a = true;
                edge e;
                forall_adj_edges(e, v){
                    node u = G.opposite(v, e);
                    if (!node_props[u].visited_a){
                        dfs_stack_a.push(u);
                    }
                }
            }
            // found b
            else {
                // process A should halt, success
                return make_tuple(false, true, a);
            }
        }
    }
    else {
        // process A should halt, failure
        return make_tuple(false, false, a);
    }

    // DFS(source -> node b, target -> node a)
    if (!dfs_stack_b.empty()){
        node v = dfs_stack_b.pop();
        if (!node_props[v].visited_b){
            // a not found, continue
            if (v != a){
                component_b.append(v);
                node_props[v].visited_b = true;
                edge e;
                forall_adj_edges(e, v){
                    node u = G.opposite(v, e);
                    if (!node_props[u].visited_b){
                        dfs_stack_b.push(u);
                    }
                }
            }
            // found a
            else {
                // process A should halt, success
                return make_tuple(false, true, b);
            }
        }
    }
    else {
        // process A should halt, failure
        return make_tuple(false, false, b);
    }

    // both searches should continue if this point has been reached
    return make_tuple(true, false, a);

}