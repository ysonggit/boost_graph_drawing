Draw Boost Graph
============
## What does the code do

1) Given a set of robots' positions, construct a undirected communication graph based on the communcation radius.

 - Each node is labeled with a robot's id.
 - Each edge is labeled with the Euclidean distance between two robots.

2) Build the unique spanning tree from the constructed communicatoin graph.

 - The highest id robot is the root.
 - Each non-highest id robot selects the neighbor with fewest hops/steps (not distance) to the root as its parent.
   If there are more than one neighbors who have the same fewest steps to the root, then it picks the one with the highest id as its parent.
    (Implemented using priority queue)

3) Generate dot files for communication graph and spanning tree.

CommunicationGraph Example: A node label represents a robot ID, an edge label represents the distance between two neighbors.
![](http://i.imgur.com/KYkO2h1.png)


Spanning Tree Example: A node label represents a robot ID, the directed edge is from a parent robot to its descendant.
![](http://i.imgur.com/eM05HmX.png)

## How to compile

```bash
mkdir build

cd build

cmake ..

make

./boost_graph_drawing

dot -Tpng graph.dot > graph.png

dot -Tpng sptree.dot > sptree.png

```