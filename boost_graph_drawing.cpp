/*
 * Author: Yang Song
 * Email: ysong.sc@gmail.com
 * Date: March 9, 2015
 */

#include <set>
#include <map>
#include <queue>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <algorithm>
#include <math.h>  
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/tuple/tuple.hpp> // tie
#include <boost/graph/graphviz.hpp>
#include <boost/graph/directed_graph.hpp> 
#include <boost/graph/undirected_graph.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/graph_traits.hpp>

using namespace boost;
using namespace std;

// bundled properties
struct Robot{
    int id;
    int x;
    int y;
    Robot(){}
    Robot(int _id, int _x, int _y){
        id = _id;
        x = _x;
        y = _y;
    }
};

int dist(const int & r1x, const int & r1y, const int & r2x, const int & r2y){
    return (int) sqrt(pow((r1x-r2x), 2) + pow((r1y-r2y), 2));
}
// bundled properties
struct Distance{
    int d;
};

/*
  adjacency_list<OutEdgeList, VertexList, Directed,
  VertexProperties, EdgeProperties,
  GraphProperties, EdgeList>
*/
typedef adjacency_list<vecS, vecS, undirectedS, Robot, Distance> CommunicationGraph;
typedef graph_traits<CommunicationGraph>::vertex_iterator vertex_iter;
typedef graph_traits<CommunicationGraph>::edge_iterator edge_iter;
typedef graph_traits<CommunicationGraph>::vertex_descriptor vertex_t;
typedef graph_traits<CommunicationGraph>::edge_descriptor edge_t;
typedef typename graph_traits<CommunicationGraph>::directed_category comm_graph_cat_t;

void constructCommunicationGraph(CommunicationGraph & g, vector<Robot> & robots, int radius){
    int n = robots.size();
    for_each(robots.begin(), robots.end(), [&g](Robot &r){
            vertex_t u = add_vertex(g);
            g[u].id = r.id;
            g[u].x = r.x;
            g[u].y = r.y;
        });
    // use vertex iterator
    // to get all vertices
    // vrange has two iters, points to 1st and last vertices
    pair<vertex_iter, vertex_iter> vrange = vertices(g);
    for(vertex_iter it = vrange.first; it != vrange.second; ++it){
        vertex_t u = *(it);
        //cout<<"Vertex id "<<g[u].id<<endl;
        for(vertex_iter jt = vrange.first; jt != vrange.second; ++jt){
            if(it != jt){
                vertex_t w = *(jt);
                int d = dist(g[u].x, g[u].y, g[w].x, g[w].y);
                if(d<radius && !boost::edge(u, w, g).second){
                    //make an edge if it does not exist
                    edge_t e; bool b;
                    boost::tie(e, b) = add_edge(u, w, g);
                    g[e].d = d;
                }
            }
        }
    }
}
// print edges for directed graph
template<typename GRAPHTYPE>
void printEdges(const GRAPHTYPE & g, boost::directed_tag){
    auto es = boost::edges(g); // actually a pair of edge_iterators
    for (auto eit = es.first; eit != es.second; ++eit) {
        auto u = boost::source(*eit, g);
        auto w =boost::target(*eit, g);
        std::cout <<g[u].id << " --->  " <<g[w].id << std::endl;
    }
}

// print edges for undirected graph
template<typename GRAPHTYPE>
void printEdges(const GRAPHTYPE & g, boost::undirected_tag){
    auto es = boost::edges(g); // actually a pair of edge_iterators
    for (auto eit = es.first; eit != es.second; ++eit) {
        auto u = boost::source(*eit, g);
        auto w =boost::target(*eit, g);
        std::cout <<g[u].id << " ---  " <<g[w].id << std::endl;
    }
}

void writeGraphDotFile(const CommunicationGraph & g, string filename){
    //  dot -Tpng graph.dot > graph.png
    ofstream dotfile(filename.c_str());
    write_graphviz(dotfile, g,
        boost::make_label_writer(boost::get(&Robot::id, g)),
        boost::make_label_writer(boost::get(&Distance::d, g)));
}

struct SpanningTreeNode{
    int id;
    int steps_from_root;
    int parent_id;
};

typedef adjacency_list<vecS, vecS, directedS, SpanningTreeNode> SpanningTree;
typedef graph_traits<SpanningTree>::vertex_iterator tree_vertex_iter;
typedef graph_traits<SpanningTree>::edge_iterator tree_edge_iter;
typedef graph_traits<SpanningTree>::vertex_descriptor tree_vertex_t;
typedef graph_traits<SpanningTree>::edge_descriptor tree_edge_t;
typedef typename graph_traits<SpanningTree>::directed_category spanning_tree_cat_t;

void graphVertexToTreeNode(const CommunicationGraph& g, const vertex_t & gv,
    SpanningTree & t, tree_vertex_t & tv, const tree_vertex_t& tv_parent){
    t[tv].id= g[gv].id;
    t[tv].steps_from_root = t[tv_parent].steps_from_root +1;
    t[tv].parent_id = t[tv_parent].id;
}

// BFS
void constructSpanningTree(const CommunicationGraph & g, SpanningTree & t){
    if(num_vertices(g)==0) return;
    // first get the root
    auto vs = boost::vertices(g);
    int max_id = - 1;
    vertex_t vertex_max_id;
    for(auto vit = vs.first; vit != vs.second; ++vit){
        vertex_t u = *(vit);
        if(g[u].id > max_id){
            max_id = g[u].id;
            vertex_max_id = u;
        } 
    }
    // insert root into the spanning tree
    tree_vertex_t root = add_vertex(t);
    t[root].id = max_id;
    t[root].steps_from_root = 0;
    t[root].parent_id = -1;
    // BFS traversal graph
    graph_traits<CommunicationGraph>::out_edge_iterator ei, ei_end;
    //queue<vertex_t> Q;
    // define comparison function for priority queue
    auto comp = [&g](const vertex_t & a, const vertex_t & b) -> bool {
        return g[a].id < g[b].id;
    };
    priority_queue<vertex_t, vector<vertex_t>, decltype(comp) > Q(comp);
    // key is the graph vertex
    // value is the tree node made from corresponding graph vertex
    map<vertex_t, tree_vertex_t> visited_nodes;
    Q.push(vertex_max_id);
    visited_nodes[vertex_max_id]=root;
    while(!Q.empty()){
        vertex_t v = Q.top();
        Q.pop();
        tree_vertex_t parent_node = visited_nodes[v]; 
        for(boost::tie(ei, ei_end) = out_edges(v, g); ei != ei_end; ++ei){
            vertex_t w = boost::target(*ei, g);
            if(visited_nodes.find(w)==visited_nodes.end()){
                Q.push(w);
                tree_vertex_t child_node = add_vertex(t);
                graphVertexToTreeNode(g, w, t, child_node, parent_node);
                visited_nodes[w] = child_node;
            }
        }
    }
    // now connect tree nodes with directed edges: pointing from parents to children
    // key is the robot id (not vertex_index), value is the tree_vertex descriptor
    map<int, tree_vertex_t> tree_nodes; 
    auto vi = vertices(t);
    for(auto vit = vi.first; vit != vi.second; ++vit){
        tree_vertex_t tv = *(vit);
        int rid = t[tv].id;
        tree_nodes[rid] = tv; 
    }
    for(auto vit= vi.first; vit!= vi.second; ++vit){
        tree_vertex_t child_v = *(vit);
        int pid = t[child_v].parent_id;
        if(pid>0){
            tree_vertex_t parent_v = tree_nodes[pid];
            if(!boost::edge(parent_v, child_v, t).second){
                tree_edge_t e;
                bool b;
                boost::tie(e, b) = add_edge(parent_v, child_v, t);
            }
        }
    }
}

void writeTreeDotFile(const SpanningTree & t, string filename){
    //  dot -Tpng graph.dot > graph.png
    ofstream dotfile(filename.c_str());
    write_graphviz(dotfile, t,
        boost::make_label_writer(boost::get(&SpanningTreeNode::id, t)));
}

int main(int argc, char *argv[]){
    vector<Robot> robots{ Robot(2, 0, 0),
            Robot(8, 0, 8),
            Robot(1, 6, 9),
            Robot(5, 2, 14),
            Robot(9, 10, 8),
            Robot(7, 8, 16),
            Robot(10, 5, 22)};
    int radius = 10;
    CommunicationGraph g;
    constructCommunicationGraph(g, robots, radius);
    printEdges(g, comm_graph_cat_t());
    string graph_dot_file = "graph.dot";
    writeGraphDotFile(g, graph_dot_file);
    SpanningTree t;
    constructSpanningTree(g, t);
    printEdges(t, spanning_tree_cat_t());
    string tree_dot_file = "sptree.dot";
    writeTreeDotFile(t, tree_dot_file);
    return 0;
}
