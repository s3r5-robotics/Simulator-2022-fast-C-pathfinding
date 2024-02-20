#ifndef _PATHFINDING_
#define _PATHFINDING_

#include <stdio.h>
#include <math.h>
#include "./heap.h"

#define SQRT2 1.414213562373095

#define true (1)
#define false (0)

#define WEIGHT 1

#define MANHATTAN(A, B) A + B

#define EUCLIDIAN(A, B) sqrt(A * A + B * B)

#define DIAGONAL_MOVEMENT true

typedef unsigned char bool;

typedef struct
{
    int minX;
    int minY;

    int maxX;
    int maxY;
} Area;

typedef struct
{
    int X, Y;
} Point;

typedef struct
{
    int x, y;
    double g, f, h;
    bool walkable : 1;
    bool closed : 1;
    bool opened : 1;
} Tile;

typedef struct 
{
    int length;
    Point* path;
} Path;

typedef struct __attribute__((__packed__)) {
    uint32_t wall_up: 1;
    uint32_t wall_down: 1;
    uint32_t wall_right: 1;
    uint32_t wall_left: 1;
    uint32_t floor: 3;  // 0 = white (normal), 1 = black (hole), 2 = silver (checkpoint), 3 = blue (puddle/swamp)
    uint32_t ramp: 2;  // 0 = no ramp, 1 = ramp up, 2 = ramp down
    uint32_t obstacle: 1;  // Is there an obstacle on the tile preventing the robot to move further?
    uint32_t debris: 1;  // Is there debris on the tile?
    uint32_t deposited: 1;  // Has the robot detected a victim and deposited on the tile?
    uint32_t visited: 1;  // Has the robot visited the tile?
    uint32_t traversable: 1;  // Can the robot physically move onto that tile?
    // 👇 Pathfinding variables, ignore these! 👇
    int g: 8;
    int f: 8;
    int h: 8;
    uint32_t closed: 1;
    uint32_t opened: 1;
    uint32_t walkable: 1;
    int x: 12;
    int y: 12;
} TilePhysical;

typedef struct
{
    unsigned int width;
    unsigned int height;
    TilePhysical** map;
} Map;

int get_neighbours(Map map, Node_s node, Node_s *out)
{
    int count = 0;
    int px = node.x + 1, mx = node.x - 1, py = node.y + 1, my = node.y - 1;

    /*if (!(my < 0 || mx < 0) && map.map[my][mx].walkable == true) // x, y, 0, 0, 0
    {
        out[count].parent = NULL;
        out[count].f = map.map[my][mx].f;
        out[count].h = map.map[my][mx].h;
        out[count].g = map.map[my][mx].g;
        out[count].y = my;
        out[count++].x = mx;
    }
    if (!(map.height < py || mx < 0) && map.map[py][mx].walkable == true)
    {
        out[count].parent = NULL;
        out[count].f = map.map[py][mx].f;
        out[count].h = map.map[py][mx].h;
        out[count].g = map.map[py][mx].g;
        out[count].y = py;
        out[count++].x = mx;
    }
    if (!(my < 0 || map.width < px) && map.map[my][px].walkable == true)
    {
        out[count].parent = NULL;
        out[count].f = map.map[my][px].f;
        out[count].h = map.map[my][px].h;
        out[count].g = map.map[my][px].g;
        out[count].y = my;
        out[count++].x = px;
    }
    if (!(map.height < py || map.width < px) && map.map[py][px].walkable == true)
    {
        out[count].parent = NULL;
        out[count].f = map.map[py][px].f;
        out[count].h = map.map[py][px].h;
        out[count].g = map.map[py][px].g;
        out[count].y = py;
        out[count++].x = px;
    }*/
    if (!(map.width < px) && map.map[node.y][px].wall_right && map.map[node.y][px].traversable)
    {
        out[count].parent = NULL;
        out[count].f = map.map[node.y][px].f;
        out[count].h = map.map[node.y][px].h;
        out[count].g = map.map[node.y][px].g;
        out[count].y = node.y;
        out[count++].x = px;
    }
    if (!(my < 0) && map.map[my][node.x].wall_up && map.map[my][node.x].traversable)
    {
        out[count].parent = NULL;
        out[count].f = map.map[my][node.x].f;
        out[count].h = map.map[my][node.x].h;
        out[count].g = map.map[my][node.x].g;
        out[count].y = my;
        out[count++].x = node.x;
    }
    {
        out[count].parent = NULL;
        out[count].f = map.map[my][node.x].f;
        out[count].h = map.map[my][node.x].h;
        out[count].g = map.map[my][node.x].g;
        out[count].y = my;
        out[count++].x = node.x;
    }
    if (!(mx < 0) && map.map[node.y][mx].wall_left && map.map[node.y][mx].traversable)
    {
        out[count].parent = NULL;
        out[count].f = map.map[node.y][mx].f;
        out[count].h = map.map[node.y][mx].h;
        out[count].g = map.map[node.y][mx].g;
        out[count].y = node.y;
        out[count++].x = mx;
    }
    if (!(map.height < py) && map.map[py][node.x].wall_down && map.map[py][node.x].traversable)
    {
        out[count].parent = NULL;
        out[count].f = map.map[py][node.x].f;
        out[count].h = map.map[py][node.x].h;
        out[count].g = map.map[py][node.x].g;
        out[count].y = py;
        out[count++].x = node.x;
    }
    return count;
}

Path backtrace(Node_s *endNode)
{
    int len = 1;
    Point* endPath = (Point*)malloc(len * sizeof(Point));
    Node_s* current = endNode;
    endPath[len - 1].X = current->x;
    endPath[len - 1].Y = current->y;
    while (current->parent != NULL)
    {
        len++;
        current = current->parent;
        endPath = (Point *)realloc(endPath, sizeof(Point) * len);
        endPath[len - 1].X = current->x;
        endPath[len - 1].Y = current->y;
    }

    Point* path = (Point *)malloc(len * sizeof(Point));
    for (int i = 0; i < len; i++)
    {
        path[i].X = endPath[len - 1 - i].X;
        path[i].Y = endPath[len - 1 - i].Y;
    }

    // it gets returned in the reverse order
    Path out = {
        .length = len,
        .path = path
    };

    return out;
}

/**
 * @brief find path between two points
 * 
 * @param map map of the maze
 * 
 * @param start starting x and y coordinates
 * 
 * @param end ending x and y coordinates
 * 
 * @returns Path struct with all points that have to be visited to get from start to end
*/
Path path_find(Map map, Point start, Point end)
{
    Node_heap_s heap;

    heap.first = NULL;
    heap.last = NULL;
    Node_s *startNode = (Node_s *)malloc(sizeof(Node_s));
    Node_s *node;
    Node_s endNode;
    int x, y, i, l;
    double ng;
    endNode.x = end.X;
    endNode.y = end.Y;
    startNode->x = start.X;
    startNode->y = start.Y;

    Path out = {
        .length = -1,
        .path = NULL
    };

    startNode->f = 0.0f;
    map.map[startNode->y][startNode->x].g = 0.0f;

    startNode->parent = NULL;

    push(&heap, startNode);

    while (!is_empty(&heap))
    {
        node = pop(&heap);
        if (node == NULL)
        {
            free_heap(&heap);

            return out;
        }

        map.map[node->y][node->x].closed = true;

        if (equal_nodes(*node, endNode))
        {
            Path final_path = backtrace(node);

            free_heap(&heap);

            return final_path;
        }

        Node_s neighbours[8];
        int numNodes = get_neighbours(map, *node, neighbours);

        for (i = 0; i < numNodes; i++)
        {
            Node_s *neighbour = (Node_s *)malloc(sizeof(Node_s));
            neighbour->x = neighbours[i].x;
            neighbour->y = neighbours[i].y;
            neighbour->f = neighbours[i].f;
            neighbour->g = neighbours[i].g;
            neighbour->h = neighbours[i].h;
            neighbour->parent = neighbours[i].parent;

            x = neighbour->x;
            y = neighbour->y;

            if (map.map[neighbour->y][neighbour->x].closed == true)
            {
                free(neighbour);
                continue;
            }

            ng = neighbour->g + ((x - node->x == 0 || y - node->y == 0) ? 1 : SQRT2);

            // printf("B: %d\n", (!map[y][x].opened || ng < neighbour->g));
            if ((!map.map[y][x].opened) || ng < neighbour->g)
            {
                neighbour->g = ng;
                neighbour->h = neighbour->h == -1 ? WEIGHT * MANHATTAN(abs(x - endNode.x), abs(y - endNode.y)) : neighbour->h;
                neighbour->f = neighbour->g + neighbour->h;
                neighbour->parent = node;

                map.map[y][x].f = neighbour->f;
                map.map[y][x].g = neighbour->g;
                map.map[y][x].h = neighbour->h;

                if (!map.map[y][x].opened)
                {
                    // printf("P\n");
                    map.map[y][x].opened = true;
                    push(&heap, neighbour);
                    continue;
                }
                else
                {
                    // printf("U\n");
                    update(&heap, neighbour);
                    continue;
                }
                free(neighbour);
            }
        }
    }

    free_heap(&heap);

    return out;
}

#endif