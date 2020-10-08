#include <tuple>

#include "LEDA/core/list.h"
#include "LEDA/core/stack.h"
#include "LEDA/core/queue.h"
#include "LEDA/graph/graph.h"
#include "LEDA/graph/node_array.h"

using std::tuple;

using leda::list;
using leda::stack;
using leda::queue;
using leda::graph;
using leda::node;
using leda::edge;
using leda::node_array;

struct node_info {
    int component;
    int level;
    bool visited_a;
    bool visited_b;
    list<edge> set_alpha;
    list<edge> set_beta;
    list<edge> set_gamma;
};

enum action_names {i, d};
enum set_names {a, b, c};

struct BFS_change {
    action_names curr_action;
    edge target_edge;
    set_names curr_set;
    node list_owner;
};

void BFS(const graph &, node_array<node_info> &, node, list<node> &, int);
int init_BFS_struct(graph &, node_array<node_info> &, node, list<edge> &);
void process_B_case_1(const graph &, node_array<node_info> &, node, node, edge);
bool process_B_case_2_check(const graph &, node_array<node_info> &, node, node, edge);
bool process_B_case_2_2_step(const graph &, node_array<node_info> &, queue<node> &, 
        stack<BFS_change> &, stack<node> &);
void reverse_BFS_changes(node_array<node_info> &, stack<BFS_change> &, stack<node> &);
void update_components(graph &, node_array<node_info> &, list<edge> &, edge, int);
tuple<bool, bool, node> process_A_step(const graph &, node_array<node_info> &, node, node, 
        stack<node> &, stack<node> &, list<node> &, list<node> &);
bool dynamic_query(const node_array<node_info> &, node, node);
bool static_query(const graph &, node, node);
