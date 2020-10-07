#include <tuple>

#include "LEDA/core/list.h"
#include "LEDA/core/stack.h"
#include "LEDA/core/queue.h"
#include "LEDA/graph/graph.h"
#include "LEDA/graph/node_array.h"

#include "decremental_connectivity.h"

using std::tie;
using std::tuple;

using leda::list;
using leda::stack;
using leda::queue;
using leda::graph;
using leda::node;
using leda::edge;
using leda::node_array;

void update_components(graph &G, node_array<node_info> &node_props, list<edge> &artificial_edges, edge e, int next_component){

    node a = G.source(e);
    node b = G.opposite(a, e);

    // init queue Q used by process B
    queue<node> Q;

    if (node_props[a].level == node_props[b].level){
        // case 1
        process_B_case_1(G, node_props, a, b, e);
        // no further actions required
        G.del_edge(e);
        return;
    }
    else if (node_props[a].level < node_props[b].level){
        // a in L_i-1 and b in L_i => [a <-> u] and [b <-> v]
        if (!process_B_case_2_check(G, node_props, a, b, e)){
            // case 2.1 (alpha(v) not empty), delete e and return
            return;
        }
        else {
            // case 2.2 (alpha(v) empty), continue
            Q.append(b);
        }
    }
    else if (node_props[a].level > node_props[b].level){
        // b in L_i-1 and a in L_i => [b <-> u] and [a <-> v]
        if (!process_B_case_2_check(G, node_props, b, a, e)){
            // case 2.1 (alpha(v) not empty), delete e and return
            G.del_edge(e);
            return;
        }
        else {
            // case 2.2 (alpha(v) empty), continue
            Q.append(a);
        }
    }

    // case 2.2, v drops at least one level, may cause many changes

    // potential deletion of edge e will be handled after both processes halt
    G.hide_edge(e);

    // mark all nodes (except a, b) as not visited by process A DFS
    node w;
    forall_nodes(w, G){
        node_props[w].visited_a = false;
        node_props[w].visited_b = false;
    }

    // init stacks to simulate DFS recursion
    stack<node> dfs_stack_a;
    dfs_stack_a.push(a);
    stack<node> dfs_stack_b;
    dfs_stack_b.push(b);

    // if current component of nodes a, b needs to be split when
    // process A finishes, two node lists will be needed
    list<node> component_a;
    component_a.append(a);
    list<node> component_b;
    component_b.append(b);

    // init a stack that stores changes to BFS structure,
    // in case they need to be reversed after both processes halt
    stack<BFS_change> changes;

    // init a stack that stores node level drops, to be used
    // along with changes stack
    stack<node> drops;

    // run processes A, B step by step until exit condition
    bool continue_A = true;
    bool found_A = false;
    node term_A = nil;
    bool continue_B;
    bool continue_B_only = false;
    bool continue_processes = true;
    bool delete_e = true;
    while (continue_processes){
        if (!continue_B_only){
            // hide artificial edges from process A
            G.hide_edges(artificial_edges);
            tie(continue_A, found_A, term_A) = process_A_step(G, node_props, a, b, dfs_stack_a, dfs_stack_b, component_a, component_b);
            // restore artificial edges for process B
            G.restore_edges(artificial_edges);
            if (continue_A){
                continue_B = process_B_case_2_2_step(G, node_props, Q, changes, drops);
                if (!continue_B){
                    // process B halts => everything up-to-date, return
                    continue_processes = false;
                }
                // else continue both processes
            }
            else if (found_A){
                // process A was successful, continue with process B to update BFS structure
                continue_B = process_B_case_2_2_step(G, node_props, Q, changes, drops);
                if (!continue_B){
                    // process B halts => everything up-to-date, return
                    continue_processes = false;
                }
                else {
                    continue_B_only = true;
                }
            }
            else if (!found_A){
                // process A was unsuccessful, => halt process B
                continue_processes = false;
                // reverse changes to BFS structure
                reverse_BFS_changes(node_props, changes, drops);
                // split connected component of a, b
                if (term_A == a){
                    forall(w, component_a){
                        node_props[w].component = next_component;
                    }
                }
                else if (term_A == b){
                    forall(w, component_b){
                        node_props[w].component = next_component;
                    }
                }
                next_component++;
                // e is now an artificial edge
                delete_e = false;
            }

        }
        else {
            // continue with process B in order to complete BFS structure maintenance
            continue_B = process_B_case_2_2_step(G, node_props, Q, changes, drops);
            if (!continue_B){
                // process B halts => everything up-to-date, return
                continue_processes = false;
            }
        }
    }
    // e does not break a component, it can be safely deleted
    if (delete_e){
        G.restore_edge(e);
        G.del_edge(e);
    }
    // e breaks a component, it is marked as artificial in order to maintain the BFS structure
    else {
        G.restore_edge(e);
        artificial_edges.append(e);
    }

}