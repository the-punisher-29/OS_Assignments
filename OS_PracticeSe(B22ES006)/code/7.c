#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_NODES 100

// Structure to represent the RAG
typedef struct {
    int numProcesses;
    int numResources;
    int adjMatrix[MAX_NODES][MAX_NODES];
} Graph;

// Function to initialize the graph
void initGraph(Graph* g, int p, int r) {
    g->numProcesses = p;
    g->numResources = r;
    int totalNodes = p + r;
    
    for(int i = 0; i < totalNodes; i++) {
        for(int j = 0; j < totalNodes; j++) {
            g->adjMatrix[i][j] = 0;
        }
    }
}

// DFS to detect cycle
bool dfsUtil(Graph* g, int v, bool* visited, bool* recStack, int* cycle, int* cycleLength) {
    if(!visited[v]) {
        visited[v] = true;
        recStack[v] = true;
        
        int totalNodes = g->numProcesses + g->numResources;
        for(int i = 0; i < totalNodes; i++) {
            if(g->adjMatrix[v][i]) {
                if(!visited[i] && dfsUtil(g, i, visited, recStack, cycle, cycleLength)) {
                    cycle[(*cycleLength)++] = v;
                    return true;
                }
                else if(recStack[i]) {
                    cycle[(*cycleLength)++] = v;
                    return true;
                }
            }
        }
    }
    recStack[v] = false;
    return false;
}

// Function to detect deadlock
bool detectDeadlock(Graph* g, int* cycle, int* cycleLength) {
    int totalNodes = g->numProcesses + g->numResources;
    bool* visited = (bool*)calloc(totalNodes, sizeof(bool));
    bool* recStack = (bool*)calloc(totalNodes, sizeof(bool));
    
    *cycleLength = 0;
    
    for(int i = 0; i < g->numProcesses; i++) {
        if(dfsUtil(g, i, visited, recStack, cycle, cycleLength)) {
            free(visited);
            free(recStack);
            return true;
        }
    }
    
    free(visited);
    free(recStack);
    return false;
}

int main() {
    Graph g;
    int p, r, edges;
    
    printf("Enter number of processes: ");
    scanf("%d", &p);
    printf("Enter number of resources: ");
    scanf("%d", &r);
    
    initGraph(&g, p, r);
    
    printf("Enter number of edges (connections): ");
    scanf("%d", &edges);
    
    printf("Enter edges (1 for Process->Resource, 2 for Resource->Process):\n");
    printf("Format: <type> <from> <to>\n");
    
    for(int i = 0; i < edges; i++) {
        int type, from, to;
        scanf("%d %d %d", &type, &from, &to);
        
        if(type == 1) { // Process -> Resource
            g.adjMatrix[from][p + to] = 1;
        } else { // Resource -> Process
            g.adjMatrix[p + from][to] = 1;
        }
    }
    
    int cycle[MAX_NODES];
    int cycleLength = 0;
    
    if(detectDeadlock(&g, cycle, &cycleLength)) {
        printf("Deadlock detected! Cycle: ");
        for(int i = cycleLength - 1; i >= 0; i--) {
            if(cycle[i] < p)
                printf("P%d ", cycle[i]);
            else
                printf("R%d ", cycle[i] - p);
        }
        printf("\n");
    } else {
        printf("No deadlock detected.\n");
    }
    
    return 0;
}