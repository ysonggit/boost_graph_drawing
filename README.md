Draw Boost Graph
============

Objectives:

1) Given a set of robots' positions, create a undirected communication graph based on the communcation radius.

 - Each node is labeled with robot's id.
 - Each edge is labeled with the Euclidean distance betweeen two robots.

TODO:

2) From the constructed communicatoin graph, build the unique spanning tree:

 - The highest id robot is the root.
 - Each non-highest id robot chooses the neighbor with fewest hops/steps (not distance) to the root as its parent,
   if there are more than one neighbors who have the same steps to the root, then pick the one with highest id as its parent.