#include <stdio.h>
#include <math.h>
#include "./heap.h"

#define X_LEN 360 // 96302, 96571 rez = 269
#define Y_LEN 270

#define SQRT2 1.414213562373095

#define true (1)
#define false (0)

#define WEIGHT 1

#define MANHATTAN(A, B) A+B

#define EUCLIDIAN(A, B) sqrt(A*A + B*B)

#define DIAGONAL_MOVEMENT true

typedef unsigned char bool;

typedef struct {
    int minX;
    int minY;

    int maxX;
    int maxY;
} Area;

typedef struct 
{
    int X ,Y;
} Point;

typedef struct
{
    int x, y;
    double g, f, h;
    bool walkable: 1;
    bool closed: 1;
    bool opened: 1;
} Tile;

Tile map[Y_LEN][X_LEN];

Area wall1GT = {89, 39, 261, 71};
Area wall2GT = {232, 39, 261, 135};
Area wall3GT = {232, 147, 261, 241};
Area wall4GT = {89, 210, 261, 241};
Area wall5GT = {89, 147, 119, 241};
Area wall6GT = {89, 39, 119, 135};
Area wall7GT = {156, 124, 203, 157};

Point startPoint = {301, 131};
Point endPoint = {61, 244};
Point* path = NULL;

Node_heap_s heap;

// seed maps
void seed_maps()
{
    for(int i = 0; i < Y_LEN; i++)
    {
        for(int j = 0; j < X_LEN; j++)
        {
            map[i][j].x = j;
            map[i][j].y = i;
            map[i][j].walkable = true;
            map[i][j].closed = false;
            map[i][j].opened = false;
            map[i][j].h = -1;
        }
    }
}

// register zones
void register_zone(Area area)
{
    /*
        typedef struct {
            int minX;
            int minY;

            int maxX;
            int maxY;
        } Area;
    */
    for(int i = area.minY-1; i < area.maxY; i++)
    {
        for(int j = area.minX-1; j < area.maxX; j++)
        {
            map[i][j].walkable = false;
        }
    }
}

// initialize the world
void init_world()
{
    seed_maps();
    register_zone(wall1GT);
    register_zone(wall2GT);
    register_zone(wall3GT);
    register_zone(wall4GT);
    register_zone(wall5GT);
    register_zone(wall6GT);
    register_zone(wall7GT);
}

float slope(Point p1, Point p2) {
    if ((p1.X - p2.X) == 0) return 0;
    return (float)(fabs((float)p2.Y - (float)p1.Y) / fabs((float)p2.X - (float)p1.X));
}

bool castRay(Point p1, Point p2)
{
    float funcSlope = slope(p1, p2);
    int w = abs(p1.X-p2.X), h = abs(p1.Y-p2.Y);
    int stepLenght = w > h ? round((float)w/(float)h) : round((float)h/(float)w);
    if(stepLenght > (w > h ? w : h))
        stepLenght = 0;
    int smallerIncrementer = 0;

    for(int i = 0; i <= (w > h ? w : h); i++)
    {
        smallerIncrementer += (i%stepLenght == 0 && stepLenght != 0);
        int y = ((w > h) * smallerIncrementer + (!(w > h)) * i), x = ((w > h) * i + (!(w > h)) * smallerIncrementer),
            smallerX = p1.X < p2.X ? p1.X : p2.X, smallerY = p1.Y < p2.Y ? p1.Y : p2.Y;
        //printf("T(%d, %d), Incrementer: %d, X: %d, Y: %d, Step length: %d\n", smallerX + x, smallerY + y, smallerIncrementer, x, y, stepLenght);
        if(map[smallerY + y][smallerX + x].walkable == false) return false;
    }
    
    return true;
}

int rayPointReducer(unsigned int size)
{
    Point point1, point2;
    point1.X = path[0].X; point1.Y = path[0].Y;
    point2.X = path[1].X; point2.Y = path[1].Y;
    float gSlope = slope(point1, point2);
    unsigned int outArraySize = 1;
    Point* arrayOut = (Point*)malloc(sizeof(Point));
    arrayOut[0].X = point1.X;
    arrayOut[0].Y = point1.Y;

    for(unsigned int i = 1; i < size; i++)
    {
        Point p1 = point1;
        Point p2 = {path[i].X, path[i].Y};
        float slop = slope(p1, p2);
        if(slop != gSlope) // if the slope isnt equal anymore it indicates that we need to mark this as the las point
        {
            outArraySize++;
            arrayOut = (Point*)realloc(arrayOut, outArraySize * sizeof(Point)); // realocate the final path
            arrayOut[outArraySize-1] = path[i-1];

            if(!(i+1 > size))
            {
                point1 = p2;
                point2.X = path[i+1].X;
                point2.Y = path[i+1].Y;
                gSlope = slope(point1, point2);
            }
        }
    }

    outArraySize++;
    arrayOut = (Point*)realloc(arrayOut, outArraySize * sizeof(Point));
    arrayOut[outArraySize-1].X = path[size-1].X;
    arrayOut[outArraySize-1].Y = path[size-1].Y;

    free(path);
    path = (Point*)malloc(outArraySize * sizeof(Point));

    for(int i = 0; i < outArraySize; i++)
    {
        path[i].X = arrayOut[i].X;
        path[i].Y = arrayOut[i].Y;
    }
    free(arrayOut);

    printf("Len: %d\n", outArraySize);
    //printf("outArraySize: %d\n", outArraySize);
    return outArraySize;
}

int getNeighbours(Tile map[Y_LEN][X_LEN], Node_s node, Node_s* out)
{
    int count = 0;
    int px = node.x + 1, mx = node.x - 1, py = node.y + 1, my = node.y - 1;

    if(!(my < 0 || mx < 0) && map[my][mx].walkable == true) // x, y, 0, 0, 0
    {
        out[count].parent = NULL;
        out[count].f = map[my][mx].f;
        out[count].h = map[my][mx].h;
        out[count].g = map[my][mx].g;
        out[count].y = my;
        out[count++].x = mx;
    }
    if(!(Y_LEN < py || mx < 0) && map[py][mx].walkable == true)
    {
        out[count].parent = NULL;
        out[count].f = map[py][mx].f;
        out[count].h = map[py][mx].h;
        out[count].g = map[py][mx].g;
        out[count].y = py;
        out[count++].x = mx;
    }
    if(!(my < 0 || X_LEN < px) && map[my][px].walkable == true)
    {
        out[count].parent = NULL;
        out[count].f = map[my][px].f;
        out[count].h = map[my][px].h;
        out[count].g = map[my][px].g;
        out[count].y = my;
        out[count++].x = px;
    }
    if(!(Y_LEN < py || X_LEN < px) && map[py][px].walkable == true)
    {
        out[count].parent = NULL;
        out[count].f = map[py][px].f;
        out[count].h = map[py][px].h;
        out[count].g = map[py][px].g;
        out[count].y = py;
        out[count++].x = px;
    }
    if(!(X_LEN < px) && map[node.y][px].walkable == true)
    {
        out[count].parent = NULL;
        out[count].f = map[node.y][px].f;
        out[count].h = map[node.y][px].h;
        out[count].g = map[node.y][px].g;
        out[count].y = node.y;
        out[count++].x = px;
    }
    if(!(my < 0) && map[my][node.x].walkable == true)
    {
        out[count].parent = NULL;
        out[count].f = map[my][node.x].f;
        out[count].h = map[my][node.x].h;
        out[count].g = map[my][node.x].g;
        out[count].y = my;
        out[count++].x = node.x;
    }
    if(!(mx < 0) && map[node.y][mx].walkable == true)
    {
        out[count].parent = NULL;
        out[count].f = map[node.y][mx].f;
        out[count].h = map[node.y][mx].h;
        out[count].g = map[node.y][mx].g;
        out[count].y = node.y;
        out[count++].x = mx;
    }
    if(!(Y_LEN < py) && map[py][node.x].walkable == true)
    {
        out[count].parent = NULL;
        out[count].f = map[py][node.x].f;
        out[count].h = map[py][node.x].h;
        out[count].g = map[py][node.x].g;
        out[count].y = py;
        out[count++].x = node.x;
    }
    return count;
}

int backtrace(Node_s* endNode)
{
    int len = 1;
    Point* endPath = (Point*)malloc(len*sizeof(Point));
    Node_s* current = endNode;
    endPath[len-1].X = current->x;
    endPath[len-1].Y = current->y;
    while(current->parent != NULL)
    {
        len++;
        current = current->parent;
        endPath = (Point*)realloc(endPath, sizeof(Point)*len);
        endPath[len-1].X = current->x;
        endPath[len-1].Y = current->y;
    }

    path = (Point*)malloc(len*sizeof(Point));
    for(int i = 0; i < len; i++)
    {
        path[i].X = endPath[len-1-i].X;
        path[i].Y = endPath[len-1-i].Y;
    }
    // it gets returned in the reverse order
    return len;
}

int path_find(Point start, Point end)
{
    heap.first = NULL;
    heap.last = NULL;
    Node_s* startNode = (Node_s*)malloc(sizeof(Node_s));
    Node_s* node;
    Node_s endNode;
    int x, y, i, l;
    double ng;
    endNode.x = end.X; endNode.y = end.Y;
    startNode->x = start.X; startNode->y = start.Y;

    startNode->f = 0.0f;
    map[startNode->y][startNode->x].g = 0.0f;

    startNode->parent = NULL;

    push(&heap, startNode);

    while(!is_empty(&heap))
    {
        node = pop(&heap);
        if(node == NULL)
        {
            return -1;
        }

        //printf("P(%d, %d), F: %f\n", node->x, node->y, node->f);

        map[node->y][node->x].closed = true;

        if(equal_nodes(*node, endNode))
        {
            return backtrace(node);
        }

        Node_s neighbours[8];
        int numNodes = getNeighbours(map, *node, neighbours);
        //printf("NL: %d\n", numNodes);
        for(i = 0; i < numNodes; i++)
        {
            Node_s* neighbour = (Node_s*)malloc(sizeof(Node_s));
            neighbour->x = neighbours[i].x;
            neighbour->y = neighbours[i].y;
            neighbour->f = neighbours[i].f;
            neighbour->g = neighbours[i].g;
            neighbour->h = neighbours[i].h;
            neighbour->parent = neighbours[i].parent;

            x = neighbour->x;
            y = neighbour->y;

            if(map[neighbour->y][neighbour->x].closed == true)
            {
                //printf("NC\n");
                free(neighbour);
                continue;
            }

            ng = neighbour->g + ((x - node->x == 0 || y - node->y == 0) ? 1 : SQRT2);

            //printf("B: %d\n", (!map[y][x].opened || ng < neighbour->g));
            if((!map[y][x].opened) || ng < neighbour->g)
            {
                neighbour->g = ng;
                neighbour->h = neighbour->h == -1 ? WEIGHT * MANHATTAN(abs(x - endNode.x), abs(y - endNode.y)) : neighbour->h;
                neighbour->f = neighbour->g + neighbour->h;
                neighbour->parent = node;

                map[y][x].f = neighbour->f;
                map[y][x].g = neighbour->g;
                map[y][x].h = neighbour->h;

                if(!map[y][x].opened)
                {
                    //printf("P\n");
                    map[y][x].opened = true;
                    push(&heap, neighbour);
                    continue;
                }
                else
                {
                    //printf("U\n");
                    update(&heap, neighbour);
                    continue;
                }
                free(neighbour);
            }
        }
    }

    return -1;
}

int main()
{
    init_world();   
    //Point start = {20, 20};
    //Point end = {60, 90};

    int len = path_find(startPoint, endPoint);

    printf("Lenght of path is: %d\n", len);

    if(len == -1)
    {
        printf("Failed to find path.\n");
        free_heap(&heap);
        return 0;
    }
    else
    {
        printf("Path found:\n");
    }

    for(int i = 0; i < len; i++)
        printf("T(%d, %d)\n", path[i].X, path[i].Y);

    len = rayPointReducer(len);

    for(int i = 0; i < len; i++)
        printf("T(%d, %d)\n", path[i].X, path[i].Y);

    for(int i = 0; i < Y_LEN; i++)
    {
        for(int j = 0; j < X_LEN; j++)
        {
            if(map[i][j].walkable != true)
            {
                printf("EE");
            }
            else
            {
                bool b = false;
                for(int k = 0; k < len; k++)
                    if(path[k].X == j && path[k].Y == i)
                        b = true;

                if(b)
                    printf("XX");
                else
                    printf("  ");
            }
        }
        printf("\n");
    }
    // maybe have multiple heaps and just free them at the end for efficiency
    free_heap(&heap);
    return 0;
}