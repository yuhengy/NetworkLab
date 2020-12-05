#ifndef __DIJKSTRA_H__
#define __DIJKSTRA_H__

#include <stdint.h>

#define INT_MAX 10000

static int minDist(int* dist, bool* visited, int numNode)
{
  int nearestIndex = -1;
  int nearestDist = INT_MAX;
  for (int index = 0; index < numNode; index++) {
    if (!visited[index] && dist[index] < nearestDist) {
      nearestIndex = index;
      nearestDist = dist[index];
    }
  }

  if (nearestIndex == -1) {
    printf("Error: cannot find a node in dijkstra.\n");
  }

  return nearestIndex;
}

static void dijkstra(int* graph, int numNode, int* nextIndex)
{
  int dist[numNode];
  bool visited[numNode];
  int prevIndex[numNode];

  for (int index = 0; index < numNode; index++) {
    dist[index]      = INT_MAX;
    visited[index]   = false;
    prevIndex[index] = -1;
  }
  dist[0] = 0;

  for (int index = 0; index < numNode; index++) {
    int u = minDist(dist, visited, numNode);
    visited[u] = true;

    for (int v = 0; v < numNode; v++) {
      if (!visited[v] && graph[u * numNode + v] > 0 &&
        dist[u] + graph[u * numNode + v] < dist[v]) {

        dist[v] = dist[u] + graph[u * numNode + v];
        prevIndex[v] = u;
      }
    }
  }

  nextIndex[0] = 0;
  for (int index = 1; index < numNode; index++) {
    int nearestIndexToRoot = index;
    while (prevIndex[nearestIndexToRoot] != -1) {
      if (prevIndex[nearestIndexToRoot] == 0) break;
      nearestIndexToRoot = prevIndex[nearestIndexToRoot];
    }

    if (prevIndex[nearestIndexToRoot] == -1) {
      printf("Error: node is unreachable in dijkstra.\n");
    }

    nextIndex[index] = nearestIndexToRoot;
  }
}



#endif
