#include <cmath>
#include <chrono>
#include <iostream>

#include "LEDA/core/list.h"
#include "LEDA/graph/graph.h"
#include "LEDA/graph/node_array.h"

#include "decremental_connectivity.h"

using std::cout;
using std::endl;

using leda::list;
using leda::graph;
using leda::node;
using leda::edge;
using leda::node_array;

int main() {

    // init graph
    int n = 100;
    int m = 2 * n * int(std::log10(n));
    int times = int(0.75 * m);
    graph G;
    random_simple_undirected_graph(G, n, m);
    G.make_undirected();
    cout<<"Random Graph G -> |V| = "<<n<<", |E| = "<<m<<endl;

    // init node property array and BFS structure
    node_array<node_info> node_props(G);
    node root = G.choose_node();
    list<edge> artificial_edges;
    int next_component = init_BFS_struct(G, node_props, root, artificial_edges);

    // time dynamic and static algorithms using a randomly generated
    // sequence of edge deletions and queries 
    node a;
    node b;
    edge e;
    bool result;
    std::chrono::duration<double> mean_dynamic;
    std::chrono::duration<double> mean_static;
    std::chrono::duration<double> diff;
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point finish;
    int i = 0;
    while(i < times){
        e = G.choose_edge();
        // make sure to skip chosen edge if it is an artificial one
        if(artificial_edges.search(e) == nil){
            a = G.source(e);
            b = G.opposite(a, e);

            // dynamic algorithm
            start = std::chrono::high_resolution_clock::now();
            update_components(G, node_props, artificial_edges, e, next_component);
            result = dynamic_query(node_props, a, b);
            finish = std::chrono::high_resolution_clock::now();
            diff = finish - start;
            mean_dynamic += diff;

            // static algorithm
            G.hide_edges(artificial_edges);
            start = std::chrono::high_resolution_clock::now();
            result = static_query(G, a, b);
            finish = std::chrono::high_resolution_clock::now();
            diff = finish - start;
            mean_static += diff;
            G.restore_edges(artificial_edges);

            i++;
        }
    }

    mean_dynamic /= double(times);
    mean_static /= double(times);

    cout<<"Number of edge deletions: "<<times<<endl;
    cout<<"Dynamic algorithm mean time: "<<std::chrono::duration<double,std::milli>(mean_dynamic).count()<<"ms"<<endl;
    cout<<"Static algorithm mean time: "<<std::chrono::duration<double,std::milli>(mean_static).count()<<"ms"<<endl;

    return 0;

}
