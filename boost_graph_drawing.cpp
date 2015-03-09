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
typedef graph_traits<CommunicationGraph>::out_edge_iterator ei, ei_end;
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

void printEdges(const CommunicationGraph & g){
    auto es = boost::edges(g); // actually a pair of edge_iterators
    for (auto eit = es.first; eit != es.second; ++eit) {
        vertex_t u = boost::source(*eit, g);
        vertex_t w =boost::target(*eit, g);
        std::cout <<g[u].id << " <--->  " <<g[w].id << std::endl;
    }
 }

void writeDotFile(const CommunicationGraph & g, string filename){
    //  dot -Tpng graph.dot > graph.png
    ofstream dotfile(filename.c_str());
    write_graphviz(dotfile, g,
        boost::make_label_writer(boost::get(&Robot::id, g)),
        boost::make_label_writer(boost::get(&Distance::d, g)));
    
}

struct SpanningTreeNode{
    int id;
    int steps_from_root;
    SpanningTreeNode * parent;
    SpanningTreeNode() {
        id = -1;
        steps_from_root = -1;
        parent = NULL;
    }
    SpanningTreeNode(int _i, int _s, SpanningTreeNode * p){
        id = _i;
        steps_from_root = _s;
        parent = p;
    }
};

// BFS
void constructSpanningTree(const CommunicationGraph & g, SpanningTreeNode *& root){
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
    root = new SpanningTreeNode(max_id, 0, nullptr);
    queue<vertex_t> Q;
    set<vertex_t> visited;
    Q.push(vertex_max_id);
    visited.insert(vertex_max_id);
    while(!Q.empty()){
        vertex_t v = Q.front();
        Q.pop();
        for(boost::tie(ei, ei_end) = out_edges(v, g); ei != ei_end; ++ei){
            vertex_t w = boost::target(*ei, g);
            if(visited.find(w)==visited.end()){
                Q.push(w);
                visited.insert(w);
            }
        }
    }

}

int main(int argc, char *argv[]){
    vector<Robot> robots{ Robot(2, 5, 6),
            Robot(1, 0, 1),
            Robot(4, 10, 8),
            Robot(3, 4, -4),
            Robot(5, -3, -4)};
    int radius = 10;
    CommunicationGraph g;
    constructCommunicationGraph(g, robots, radius);
    printEdges(g);
    string dot_file = "graph.dot";
    writeDotFile(g, dot_file);
    return 0;
}
