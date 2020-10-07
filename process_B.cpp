#include "LEDA/core/list.h"
#include "LEDA/core/stack.h"
#include "LEDA/core/queue.h"
#include "LEDA/graph/graph.h"
#include "LEDA/graph/node_list.h"
#include "LEDA/graph/node_array.h"

#include "decremental_connectivity.h"

using leda::list;
using leda::stack;
using leda::queue;
using leda::graph;
using leda::node;
using leda::edge;
using leda::node_list;
using leda::node_array;

void BFS(const graph &G, node_array<node_info> &node_props, node root, list<node> &remaining_nodes, int component){

    // BFS -> visit as many nodes of G as possible, starting from root
    node_list bfs_q;
	bfs_q.append(root);
	node v = bfs_q.head();
    while(v != nil){
        edge e;
        forall_adj_edges(e, v){
            node w = G.opposite(v, e);
            if(!bfs_q.member(w) && (remaining_nodes.search(w)!=nil)){
                // node w is an unexplored child node
                bfs_q.append(w);
                // newly discovered node belongs to current connected component
                node_props[w].component = component;
                // newly discovered node is a child, therefore belongs to next level
                node_props[w].level = node_props[v].level + 1;
            }
        }
        // node v belongs to current connected component, no need to explore again later
        remaining_nodes.remove(v);
        v = bfs_q.succ(v);
	}

}

int init_BFS_struct(graph &G, node_array<node_info> &node_props, node root, list<edge> &artificial_edges){

    // list of nodes that have not yet been assigned a connected component
    list<node> remaining_nodes;
    node v;
    forall_nodes(v, G){
        remaining_nodes.append(v);
    }
    remaining_nodes.remove(root);

    // start from root, discover first connected component and assign levels to nodes
    node_props[root].level = 0;
    node_props[root].component = 0;
	BFS(G, node_props, root, remaining_nodes, 0);

    // for each unexplored node v, repeat process
    int current_component = 1;
    while (!remaining_nodes.empty()){
        // add edge (root, v) to G and to artificial edges
        v = remaining_nodes.head();
        edge e = G.new_edge(root, v);
        artificial_edges.append(e);
        // v is now a level-1 node
        node_props[v].level = 1;
        // v belongs to a new, undiscovered component
        node_props[v].component = current_component;
        // search again in order to to assign level and connected component
        BFS(G, node_props, v, remaining_nodes, current_component);
        current_component++;
    }

    // now that all nodes have been explored, create edge sets
    edge e;
    forall_nodes(v, G){
        forall_adj_edges(e, v){
            node u = G.opposite(v, e);
            if (node_props[v].level == (node_props[u].level + 1)){
                // back edge (to L(v)-1)
                node_props[v].set_alpha.append(e);
            }
            else if (node_props[v].level == node_props[u].level){
                // local edge (to L(v))
                node_props[v].set_beta.append(e);
            }
            else if (node_props[v].level == (node_props[u].level - 1)){
                // forward edge (to L(v)+1)
                node_props[v].set_gamma.append(e);
            }
        }
    }

    // return next number to be used as component name
    return current_component;

}

void process_B_case_1(const graph &G, node_array<node_info> &node_props, node a, node b, edge e){

    // delete e from beta(a)
    node_props[a].set_beta.remove(e);

    // delete e from beta(b)
    node_props[b].set_beta.remove(e);

}

bool process_B_case_2_check(const graph &G, node_array<node_info> &node_props, node u, node v, edge e){

    // delete e from gamma(u)
    node_props[u].set_gamma.remove(e);

    // delete e from alpha(v)
    node_props[v].set_alpha.remove(e);

    // check if alpha(v) is empty in order to determine if case 2.1 or case 2.2
    return node_props[v].set_alpha.empty();

}

bool process_B_case_2_2_step(const graph &G, node_array<node_info> &node_props, queue<node> &Q, stack<BFS_change> &changes, stack<node> &drops){

    // node v drops a level, possibly triggering even more nodes to drop
    if (!Q.empty()){
        node w = Q.pop();
        node_props[w].level++;
        drops.push(w);
        edge f;
        node x;
        BFS_change temp_change;

        // all local edges become back edges
        // all forward edges become local edges
        forall(f, node_props[w].set_beta){
            x = G.opposite(w, f);

            temp_change.list_owner = x;
            temp_change.target_edge = f;

            node_props[x].set_gamma.append(f);

            temp_change.curr_action = i;
            temp_change.curr_set = c;
            changes.push(temp_change);

            node_props[x].set_beta.remove(f);

            temp_change.curr_action = d;
            temp_change.curr_set = b;
            changes.push(temp_change);
        }

        temp_change.list_owner = w;

        forall(f, node_props[w].set_beta){
            temp_change.target_edge = f;

            node_props[w].set_alpha.append(f);

            temp_change.curr_action = i;
            temp_change.curr_set = a;
            changes.push(temp_change);

            node_props[w].set_beta.remove(f);

            temp_change.curr_action = d;
            temp_change.curr_set = b;
            changes.push(temp_change);
        }

        forall(f, node_props[w].set_gamma){
            x = G.opposite(w, f);

            temp_change.list_owner = x;
            temp_change.target_edge = f;

            node_props[x].set_beta.append(f);

            temp_change.curr_action = i;
            temp_change.curr_set = b;
            changes.push(temp_change);

            node_props[x].set_alpha.remove(f);

            temp_change.curr_action = d;
            temp_change.curr_set = a;
            changes.push(temp_change);

            if (node_props[x].set_alpha.empty()){
                Q.append(x);
            }
        }

        temp_change.list_owner = w;

        forall(f, node_props[w].set_gamma){
            temp_change.target_edge = f;

            node_props[w].set_beta.append(f);

            temp_change.curr_action = i;
            temp_change.curr_set = b;
            changes.push(temp_change);

            node_props[w].set_gamma.remove(f);

            temp_change.curr_action = d;
            temp_change.curr_set = c;
            changes.push(temp_change);
        }

        if (node_props[w].set_alpha.empty()){
            Q.append(w);
        }

        // step done, process must continue
        return true;
    }
    else {
        // done, halt
        return false;
    }

}

void reverse_BFS_changes(node_array<node_info> &node_props, stack<BFS_change> &changes, stack<node> &drops){

    // reverse changes to sets alpha, beta, gamma
    node u;
    edge e;
    BFS_change temp_change;
    while (!changes.empty()){
        temp_change = changes.pop();
        u = temp_change.list_owner;
        e = temp_change.target_edge;
        switch (temp_change.curr_action){
            case i:
                // reverse insert operation => delete edge e from set
                switch (temp_change.curr_set){
                    case a:
                        node_props[u].set_alpha.remove(e);
                        break;

                    case b:
                        node_props[u].set_beta.remove(e);
                        break;

                    case c:
                        node_props[u].set_gamma.remove(e);
                        break;
                }
                break;

            case d:
                // reverse delete operation => insert edge e in set
                switch (temp_change.curr_set){
                    case a:
                        node_props[u].set_alpha.append(e);
                        break;

                    case b:
                        node_props[u].set_beta.append(e);
                        break;

                    case c:
                        node_props[u].set_gamma.append(e);
                        break;
                }
                break;

        }
    }

    // undo node level drops
    while (!drops.empty()){
        u = drops.pop();
        node_props[u].level--;
    }

}