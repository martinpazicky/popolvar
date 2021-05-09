#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INFINITY 999999 //value used as initial distance of a vertex
#define MAX_TOCOLLECT 6 //total objects to collect (princesses + dragon)
typedef struct vertex
{
    char fieldType;
    int row;
    int column;
    int distance;
    struct vertex* previous;
}VERTEX;

typedef struct priorityQueue
{
    VERTEX* heap[1000];
    int heapSize;
}PRIORITYQUEUE;

typedef struct vertexMap
{
    VERTEX*** map;
    int r;
    int c;
    char princessCount;
    VERTEX* toCollect[MAX_TOCOLLECT];
}VERTEXMAP;

/**FUNCTIONS TO TEST & PRINT**/
/**
 * Prints the position data
 * from path that was found
 */
void printPath(int* path, int size);

/**
 * Prints the loaded char map
 */
void printMap(int r, int c, char** map);

/**
 * Changes the original char map, visualizes
 * the found path
 * Map needs to be printed to see the changes
 */
void visualizePath(int* path, int pathLength, char** map);

/**
 * Generates random map
 */
char** generateMap(int r, int c);

/**
 * Loads map from file
 */
char** loadMap_file(int *r, int *c, int *t);

/*******************************/

/********PRIORITY QUEUE********/
/**
 * Allocates memory for priority queue structure
 * Inserts dummy item at 0th index of heap with distance = -1
 */
PRIORITYQUEUE* prQueueInit();

/**
 * Inserts vertex in heap
 */
void enqueue(PRIORITYQUEUE *prQueue, VERTEX* vertex);

/**
 * Deletes top vertex from heap
 */
VERTEX* dequeue(PRIORITYQUEUE *prQueue);

/*****************************/

/**UTILITY FUNCTIONS - SWAP**/
void vertexSwap(VERTEX **x, VERTEX **y);

void intSwap(int *a, int *b);

/****************************/

/***VERTEX MAP FUNCTIONS****/
/**
 * Creates 2D array of vertexes, which represents map
 * Also stores extra information about the map
 * (Dragon & Princesses positions, map dimensions)
 */
VERTEXMAP* createVertexMap(int r, int c, char** map);

/**
 * Restarts distances in map
 * Used between several calls of Dijkstra
 * in findprincesses function
 */
void resetMap(VERTEXMAP* vertexMap);

/**
 * Gets length of path from Linked List
 */
int LLgetPathLength(VERTEX* end);

/**
 * Transforms path from Linked List
 * to integer array of positions
 * in reverse order
 */
void LLpathToIntArray(VERTEX* goal, int *out_path, int i);

/**************************/

/** FUNCTIONS TO GET BEST ORDER TO COLLECT PRINCESSES FROM ADJ MATRIX **/
/**
 * Gets path cost from Adjacency Matrix
 * based on given order
 */
int getPathCost(int* order,int **adjacencyMatrix, int princessCount);

/**
 * Generates all possible orders (permutations) to collect princesses
 * On each order generated function counts the path cost
 * Each permutation is stored in currOrder
 * The order with minimum cost is stored in out_bestOrder
 */
void getBestOrder(int *currOrder, int perm_start, int *out_bestOrder, int **adjacencyMatrix, int princessCount);

/***********************************************************************/

/**
 * Given integer array of positions
 * function counts the time taken to
 * get through the road
 * Usage:
 * to test if dragon was killed earlier than the limit
 * to count total time of path (testing)
 */
int getPathTime(int* path, int pathLength, char** map);

/**
 * Concatenates 2 paths (int arrays)
 * note: paths contain the same position twice
 * (end of path1, beginning of path2 is the same)
 * end of path1 is excluded from the concatenation
 */
int* pathsConcat(int* path1, int path1Size, int* path2, int path2size, int *out_pathLength);

/**
 * Gets edge value
 * Value is calculated based on the fieldType of the vertex
 * that is visited
 */
int getEdgeValue(VERTEX* vertex);

/**
 * PART OF DIJKSTRA ALGORITHM
 * Based on given position (curr vertex) checks all neighbours and
 * calculates distances, if new distance is shorter than the old one
 * distance is updated and the updated vertex is enqueued
 */
void visitNeighbours(VERTEX *curr, VERTEXMAP* vertexMap, PRIORITYQUEUE* prQueue);

/**
 * Finds dragon & returns the path to dragon
 */
int* findDragon(VERTEXMAP* vertexMap, PRIORITYQUEUE* prQueue, int *out_pathLength);

/**
 * Finds princesses & returns the path to princesses
 */
int* findPrincesses(VERTEXMAP* vertexMap, PRIORITYQUEUE* heap, int *out_pathLength);

/**
 * Frees memory allocated for vertexMap
 */
void freeVertexMap(VERTEXMAP* vertexMap);

int* zachran_princezne(char **mapa, int r, int c, int t, int *dlzka_cesty) {
    PRIORITYQUEUE *heap = prQueueInit();
    VERTEXMAP *vertexMap = createVertexMap(r, c, mapa);

    int dragPathLength, *dragPath;
    dragPath = findDragon(vertexMap, heap, &dragPathLength);
    if(!dragPath) {
        printf("Cesta neexistuje\n");
        return NULL;
    }
    if (getPathTime(dragPath, dragPathLength, mapa) > t) {
        printf("Nestihol si zabit draka!\n");
        return NULL;
    }

    int princessesPathLength, *princesessPath;
    princesessPath = findPrincesses(vertexMap,heap,&princessesPathLength);
    if(!princesessPath)
    {
        printf("Cesta neexistuje\n");
        return NULL;
    }
    freeVertexMap(vertexMap);

    int pathFinLength, *pathFin;
    pathFin = pathsConcat(dragPath, dragPathLength, princesessPath, princessesPathLength, &pathFinLength);
    *dlzka_cesty = pathFinLength;
    return pathFin;
}

int main() {
    int c,r,t;
    char **map;
    map = loadMap_file(&r, &c, &t); //VO FUNKCII TREBA UPRAVIT MENO/CESTU NACITAVANEHO SUBORU
//    map = generateMap(r,c);       //PRE GENEROVANIE ROZNYCH MAP SA VO FUNKCII DA ZMENIT SEED
    int dlzka_cesty;
    int* cesta;
    cesta = zachran_princezne(map,r,c,t,&dlzka_cesty);
    printPath(cesta, dlzka_cesty);
    int time = getPathTime(cesta,dlzka_cesty,map);
    printf("%d\n",time);
//    visualizePath(cesta,dlzka_cesty,map);
//    printMap(r,c,map);
}

int* findDragon(VERTEXMAP* vertexMap, PRIORITYQUEUE* prQueue, int *out_pathLength)
{
    VERTEX* start = vertexMap->map[0][0];
    VERTEX* curr = start;
    curr->distance = 0;
    enqueue(prQueue, curr);
    //DIJKSTRA LOOP
    while(1) {
        curr = dequeue(prQueue);
        if (curr->fieldType == 'D') {
            break;
        }
        visitNeighbours(curr, vertexMap, prQueue);
        if (prQueue->heapSize == 0) { //WHOLE MAP SEARCHED BUT DRAGON NOT FOUND
            *out_pathLength = 0;
            return NULL;
        }
    }
    *out_pathLength = LLgetPathLength(curr);
    int* path = (int*)malloc(2 * (*out_pathLength) * sizeof(int));
    LLpathToIntArray(curr, path, 2 * *out_pathLength - 2);
    return path;
}

int* findPrincesses(VERTEXMAP* vertexMap, PRIORITYQUEUE* heap, int *out_pathLength)
{
    int princessCount = vertexMap->princessCount;

    //ADJACENCY MATRIX TO REPRESENT COMPLETE GRAPH
    //OF VERTEXES FROM TO_COLLECT ARRAY
    //+1 IS FOR DRAGON
    int **adjacencyMatrix = (int**)malloc((princessCount+1) * sizeof(int*));
    for(int i = 0;i<princessCount+1;i++)
        adjacencyMatrix[i] = (int*)malloc((princessCount+1) * sizeof(int));

    //ARRAYS TO STORE PATHS & PATH LENGTHS ACCORDING TO ADJ MATRIX
    int *pathMatrix[princessCount+1][princessCount+1];
    int pathLMatrix[princessCount+1][princessCount+1];

    int k = 0;

    VERTEX *start;
    //LOOP TO GET MIN DISTANCES FROM EACH PRINCESS TO EACH PRINCESS (DRAGON)
    //AND STORE THEM IN ADJ MATRIX
    while(k <= princessCount) {
        resetMap(vertexMap);
        start = vertexMap->toCollect[k];
        heap->heapSize = 0;
        start->distance = 0;
        enqueue(heap, start);
        //DIJKSTRA LOOP FOR WHOLE MAP
        while (1) {
            start = dequeue(heap);
            visitNeighbours(start, vertexMap, heap);
            if (heap->heapSize == 0)
                break;
        }
        //CHECKS IF EVERY PRINCESS IS ACCESSIBLE, IF SHE IS NOT
        //RETURNS NULL
        if(k == 0) {
            for (int i = 0; i <= princessCount; i++) {
                if (vertexMap->toCollect[i]->distance == INFINITY) {
                    return NULL;
                }
            }
        }
        //FILLS MIN DISTANCES IN ADJ MATRIX & STORES PATHS/PATH LENGTHS
        for(int i = 0; i<=princessCount;i++) {
            adjacencyMatrix[k][i] = vertexMap->toCollect[i]->distance;
            int pathLength = LLgetPathLength(vertexMap->toCollect[i]);
            int* path = (int*)malloc(2*pathLength * sizeof(int));
            LLpathToIntArray(vertexMap->toCollect[i], path, 2 * pathLength - 2);
            pathMatrix[k][i] = path;
            pathLMatrix[k][i] = pathLength;
        }
        k++;
    }

    //PERMUTATIONS PART
    int toPermutate[princessCount];
    int bestOrder[princessCount];
    //INITIALIZATION
    for(int i = 0;i<princessCount;i++) {
        toPermutate[i] = i + 1;
        bestOrder[i] = i + 1;
    }
    getBestOrder(toPermutate, 0, bestOrder, adjacencyMatrix,princessCount);

    //USE OF THE BEST ORDER TO CONCAT THE PATHS IN CORRECT ORDER
    int* out_path = NULL;
    int* path,pathLength;
    int temp = 0;
    for(int i = 0;i<princessCount;i++)
    {
        path = pathMatrix[temp][bestOrder[i]];
        pathLength = pathLMatrix[temp][bestOrder[i]];
        temp = bestOrder[i];
        if(!out_path) {
            out_path = path;
            *out_pathLength = pathLength;
        } else{
            out_path = pathsConcat(out_path, *out_pathLength, path, pathLength, out_pathLength);
        }
    }
    return out_path;
}

void getBestOrder(int *currOrder, int perm_start, int *out_bestOrder, int **adjacencyMatrix, int princessCount)
{
    if(perm_start == princessCount) { //ONE PERMUTATION GENERATED
        int currentPathCost = getPathCost(currOrder, adjacencyMatrix, princessCount);
        int minPathCost = getPathCost(out_bestOrder, adjacencyMatrix, princessCount);

        if (currentPathCost < minPathCost) {
            for(int i = 0; i<princessCount;i++)
                out_bestOrder[i] = currOrder[i];
        }
        return;
    }
    for(int i=perm_start; i < princessCount; i++)
    {
        intSwap((currOrder + i), (currOrder + perm_start));
        getBestOrder(currOrder, perm_start + 1, out_bestOrder, adjacencyMatrix, princessCount);
        intSwap((currOrder + i), (currOrder + perm_start));
    }
}

int getPathCost(int* order,int **adjacencyMatrix, int princessCount)
{
    int pathCost = 0;
    int temp = 0;
    for(int i = 0;i<princessCount;i++)
    {
        pathCost += adjacencyMatrix[temp][order[i]];
        temp = order[i];
    }
    return pathCost;
}

void visitNeighbours(VERTEX *curr, VERTEXMAP* vertexMap, PRIORITYQUEUE* prQueue)
{
    VERTEX *neighbours[4];
    for(int i = 0;i<4;i++)
        neighbours[i]=NULL;
    // top neighbour
    if(curr->row > 0)
        neighbours[0] = vertexMap->map[curr->row - 1][curr->column];
    // right neighbour
    if(curr->column < vertexMap->c-1)
        neighbours[1] = vertexMap->map[curr->row][curr->column + 1];
    // bottom neighbour
    if(curr->row < vertexMap->r-1)
        neighbours[2] = vertexMap->map[curr->row + 1][curr->column];
    // left neighbour
    if(curr->column > 0)
        neighbours[3] = vertexMap->map[curr->row][curr->column - 1];

    for(int i = 0;i<4;i++)
    {
        if(neighbours[i])
        {
            int edgeValue = getEdgeValue(neighbours[i]);
            if(neighbours[i]->distance > curr->distance + edgeValue){
                neighbours[i]->distance = edgeValue + curr->distance;
                neighbours[i]->previous = curr;
                enqueue(prQueue, neighbours[i]);
            }
        }
    }
}

int getEdgeValue(VERTEX* vertex)
{
    if(vertex->fieldType == 'N')
        return INFINITY;
    if(vertex->fieldType == 'H')
        return 2;
    if(vertex->fieldType == 'C')
        return 1;
    if(vertex->fieldType == 'D' || vertex->fieldType == 'P')
        return 1;
    return 0;
}

int LLgetPathLength(VERTEX* end)
{
    int pathLength = 0;
    while(end)
    {
        pathLength++;
        end = end->previous;
    }
    return pathLength;
}

void LLpathToIntArray(VERTEX* goal, int *out_path, int i) {
    if (goal->distance == 0) {
        out_path[i] = goal->column;
        out_path[i + 1] = goal->row;
        return;
    }
    LLpathToIntArray(goal->previous, out_path, i - 2);
    out_path[i] = goal->column;
    out_path[i + 1] = goal->row;
}

int* pathsConcat(int* path1, int path1Size, int* path2, int path2size, int *out_pathLength)
{
    *out_pathLength = path1Size + path2size - 1;
    int* pathOut = (int*)malloc(2 * (*out_pathLength) * sizeof(int));
    memcpy(pathOut,path1,2*(path1Size-1) * sizeof(int));
    memcpy(pathOut + 2*(path1Size-1),path2,2*path2size* sizeof(int));
    return pathOut;
}

VERTEXMAP* createVertexMap(int r, int c, char **map)
{
    VERTEXMAP* vertexMap = (VERTEXMAP*)malloc(sizeof(VERTEXMAP));
    vertexMap->princessCount = 0;
    //toCollect - dragon is at 0th index
    //princesses are on 1-MAX_TOCOLLECT indexes
    for(int i = 0;i<MAX_TOCOLLECT;i++)
        vertexMap->toCollect[i] = NULL;

    int index = 1;
    vertexMap->r = r;
    vertexMap->c = c;
    vertexMap->map = (VERTEX***)malloc(r * sizeof(VERTEX**));
    for(int i = 0;i<r;i++) {
        vertexMap->map[i] = (VERTEX**)malloc(c * sizeof(VERTEX*));
        for (int j = 0; j < c; j++) {
            vertexMap->map[i][j] = (VERTEX*)malloc(sizeof(VERTEX));
            vertexMap->map[i][j]->row = i;
            vertexMap->map[i][j]->column = j;
            vertexMap->map[i][j]->fieldType = map[i][j];
            vertexMap->map[i][j]->distance = INFINITY;
            vertexMap->map[i][j]->previous = NULL;
            if(map[i][j] == 'D')
                vertexMap->toCollect[0] = vertexMap->map[i][j];
            if(map[i][j] == 'P')
                vertexMap->toCollect[index++] = vertexMap->map[i][j];
        }
    }
    vertexMap->princessCount = (char)(index - 1);
    return vertexMap;
}

void resetMap(VERTEXMAP* vertexMap)
{
    for(int i = 0; i < vertexMap->r; i++)
        for(int j = 0; j < vertexMap->c; j++)
        {
            vertexMap->map[i][j]->distance = INFINITY;
            vertexMap->map[i][j]->previous = NULL;
        }
}

void freeVertexMap(VERTEXMAP* vertexMap)
{
    for(int i = 0;i<vertexMap->r;i++)
    {
        for(int j = 0;j<vertexMap->c;j++)
            free(vertexMap->map[i][j]);
    }
    free(vertexMap->map);
    free(vertexMap);
}

void intSwap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

void vertexSwap(VERTEX **x, VERTEX **y)
{
    VERTEX* temp = *x;
    *x = *y;
    *y = temp;
}

PRIORITYQUEUE* prQueueInit()
{
    PRIORITYQUEUE* heap = (PRIORITYQUEUE*)malloc(sizeof(PRIORITYQUEUE));
    VERTEX* dummy = (VERTEX*)malloc(sizeof(VERTEX));
    dummy->distance = -1;
    heap->heap[0] = dummy;
    heap->heapSize = 0;
    return heap;
}

void enqueue(PRIORITYQUEUE *prQueue, VERTEX* vertex)
{
    prQueue->heapSize+=1;
    int index = prQueue->heapSize;
    prQueue->heap[index] = vertex;

    while(prQueue->heap[index / 2] && prQueue->heap[index / 2]->distance > prQueue->heap[index]->distance) {
        vertexSwap(&prQueue->heap[index / 2], &prQueue->heap[index]);
        index/=2;
    }
}

VERTEX* dequeue(PRIORITYQUEUE *prQueue)
{
    VERTEX* out = prQueue->heap[1];
    vertexSwap(&prQueue->heap[1], &prQueue->heap[prQueue->heapSize]);
    int i = 1;
    while(1)
    {
        int left = 2*i;
        int right = 2*i+1;
        int toSwap;
        int swapped = 0;

        if(left >= prQueue->heapSize)
            break;

        if(right >= prQueue->heapSize || prQueue->heap[left]->distance < prQueue->heap[right]->distance)
            toSwap = left;
        else
            toSwap = right;

        if(prQueue->heap[toSwap]->distance < prQueue->heap[i]->distance) {
            vertexSwap(&prQueue->heap[i], &prQueue->heap[toSwap]);
            swapped = 1;
        }
        if(!swapped)
            break;
        i=toSwap;
    }
    prQueue->heapSize-=1;
    return out;
}

int getPathTime(int* path, int pathLength, char** map)
{
    int time = 0;
    for(int i=0; i<pathLength; i++){
        int a = path[i*2+1];
        int b = path[i*2];
        char c = map[a][b];
        if(c == 'C' || c == 'D' || c == 'P')
            time += 1;
        else if(c == 'H')
            time+=2;
        else {
            printf("N - Error!\n");
            return -1;
        }
    }
    return time;
}

char** loadMap_file(int *r, int *c, int *t)
{
    FILE *f;
    if((f = fopen("mapy/test1-ukazkovy.txt","r"))==NULL) //ZADAJTE NAZOV SUBORU
    {
        printf("NENAJDENY SUBOR");
        return NULL;
    }
    fscanf(f, "%d %d %d", r, c, t);
    char **map = (char**)malloc(*r * sizeof(char*));
    for(int i = 0;i<*r;i++)
    {
        map[i] = (char*)malloc(*c * sizeof(char));
        for(int j = 0;j<*c;j++)
        {
            char temp = (char)fgetc(f);
            if(temp =='\r')
                temp = (char)fgetc(f);
            if(temp =='\n')
                temp = (char)fgetc(f);
            map[i][j] = temp;
        }
    }
    fclose(f);
    return map;
}

void printPath(int* path, int size)
{
    for(int i=0; i<size; i++){
        printf("%d %d\n", path[i*2], path[i*2+1]);
    }
}

void printMap(int r, int c, char** map)
{
    for(int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            printf("%c", map[i][j]);
        }
        printf("\n");
    }
}

void visualizePath(int* path, int pathLength, char** map)
{
    for(int i=0; i<pathLength; i++){
        char c = map[path[i*2+1]][path[i*2]];
        if(c == 'C')
            map[path[i*2+1]][path[i*2]] = '.';
        if(c == 'H')
            map[path[i*2+1]][path[i*2]] = ',';
        if(c == 'D')
            map[path[i*2+1]][path[i*2]] = 'd';
        if(c == 'P')
            map[path[i*2+1]][path[i*2]] = 'p';
        if(c == 'N')
            map[path[i*2+1]][path[i*2]] = '?';
    }
}

char** generateMap(int r, int c)
{
    srand(12);
    char pool[3] = {'C','H','N'};
    char **map = (char**)malloc(r * sizeof(char*));
    for(int i = 0;i<r;i++)
    {
        map[i] = (char*)malloc(c * sizeof(char));
        for(int j = 0;j<c;j++)
        {
            char temp = pool[rand()%3];
            map[i][j] = temp;
        }
    }
    for(int i = 0;i<5;i++)
    {
        map[rand()%r][rand()%c] = 'P';
    }
    map[rand()%r][rand()%c] = 'D';
    map[0][0] = 'C';
    return map;
}
