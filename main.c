#include <stdio.h>
#include <math.h>
#include "./pathfinding.h"

#define X_LEN 360 // 96302, 96571 rez = 269
#define Y_LEN 270

Area wall1GT = {89, 39, 261, 71};
Area wall2GT = {232, 39, 261, 135};
Area wall3GT = {232, 147, 261, 241};
Area wall4GT = {89, 210, 261, 241};
Area wall5GT = {89, 147, 119, 241};
Area wall6GT = {89, 39, 119, 135};
Area wall7GT = {156, 124, 203, 157};

Point startPoint = {301, 131};
Point endPoint = {61, 244};

// seed maps
void seed_maps(Map map)
{
    for (int i = 0; i < map.height; i++)
    {
        for (int j = 0; j < map.width; j++)
        {
            map.map[i][j].x = j;
            map.map[i][j].y = i;
            map.map[i][j].walkable = true;
            map.map[i][j].closed = false;
            map.map[i][j].opened = false;
            map.map[i][j].h = -1;
        }
    }
}

// register zones
void register_zone(Tile** map, Area area)
{
    /*
        typedef struct {
            int minX;
            int minY;

            int maxX;
            int maxY;
        } Area;
    */
    for (int i = area.minY - 1; i < area.maxY; i++)
    {
        for (int j = area.minX - 1; j < area.maxX; j++)
        {
            map[i][j].walkable = false;
        }
    }
}

// initialize the world
void init_world(Map map)
{
    seed_maps(map);
    register_zone(map.map, wall1GT);
    register_zone(map.map, wall2GT);
    register_zone(map.map, wall3GT);
    register_zone(map.map, wall4GT);
    register_zone(map.map, wall5GT);
    register_zone(map.map, wall6GT);
    register_zone(map.map, wall7GT);
}

float slope(Point p1, Point p2)
{
    if ((p1.X - p2.X) == 0)
        return 0;
    return (float)(fabs((float)p2.Y - (float)p1.Y) / fabs((float)p2.X - (float)p1.X));
}

/**
 * @brief reduces number of points only to only key points (does not work perfectly)
 * 
 * @param path path gotten from pathfinding function
 * 
 * @param size leght of array
 * 
 * @returns Path struct with reduced path
*/
Path ray_point_reducer(Point* path, unsigned int size)
{
    Point point1, point2;
    point1.X = path[0].X;
    point1.Y = path[0].Y;
    point2.X = path[1].X;
    point2.Y = path[1].Y;
    float gSlope = slope(point1, point2);
    unsigned int outArraySize = 1;
    Point *arrayOut = (Point *)malloc(sizeof(Point));
    arrayOut[0].X = point1.X;
    arrayOut[0].Y = point1.Y;

    for (unsigned int i = 1; i < size; i++)
    {
        Point p1 = point1;
        Point p2 = {path[i].X, path[i].Y};
        float slop = slope(p1, p2);
        if (slop != gSlope) // if the slope isnt equal anymore it indicates that we need to mark this as the las point
        {
            outArraySize++;
            arrayOut = (Point *)realloc(arrayOut, outArraySize * sizeof(Point)); // realocate the final path
            arrayOut[outArraySize - 1] = path[i - 1];

            if (!(i + 1 > size))
            {
                point1 = p2;
                point2.X = path[i + 1].X;
                point2.Y = path[i + 1].Y;
                gSlope = slope(point1, point2);
            }
        }
    }

    outArraySize++;
    arrayOut = (Point *)realloc(arrayOut, outArraySize * sizeof(Point));
    arrayOut[outArraySize - 1].X = path[size - 1].X;
    arrayOut[outArraySize - 1].Y = path[size - 1].Y;

    free(path);
    path = (Point *)malloc(outArraySize * sizeof(Point));

    for (int i = 0; i < outArraySize; i++)
    {
        path[i].X = arrayOut[i].X;
        path[i].Y = arrayOut[i].Y;
    }
    free(arrayOut);

    printf("Len: %d\n", outArraySize);
    // printf("outArraySize: %d\n", outArraySize);
    Path new_path = {
        .length = outArraySize,
        .path = path
    };
    
    return new_path;
}

int main()
{
    Tile** map = (Tile**)malloc(Y_LEN * sizeof(Tile*));

    for(int i = 0; i < Y_LEN; i++)
        map[i] = (Tile*)malloc(X_LEN * sizeof(Tile));

    Map wrapped_map = {
        .width = X_LEN,
        .height = Y_LEN,
        .map = map
    };

    init_world(wrapped_map);
    // Point start = {20, 20};
    // Point end = {60, 90};

    Path path = path_find(wrapped_map, startPoint, endPoint);

    printf("Lenght of path is: %d\n", path.length);

    if (path.length == -1)
    {
        printf("Failed to find path.\n");
        return 0;
    }
    else
    {
        printf("Path found:\n");
    }

    for (int i = 0; i < path.length; i++)
        printf("T(%d, %d)\n", path.path[i].X, path.path[i].Y);

    for (int i = 0; i < path.length; i++)
        printf("T(%d, %d)\n", path.path[i].X, path.path[i].Y);

    for (int i = 0; i < Y_LEN; i++)
    {
        for (int j = 0; j < X_LEN; j++)
        {
            if (map[i][j].walkable != true)
            {
                printf("E");
            }
            else
            {
                bool b = false;
                for (int k = 0; k < path.length; k++)
                    if (path.path[k].X == j && path.path[k].Y == i)
                        b = true;

                if (b)
                    printf("X");
                else
                    printf(" ");
            }
        }
        printf("\n");
    }

    return 0;
}