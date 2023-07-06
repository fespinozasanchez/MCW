#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <jansson.h>

#define MAX_COORDINATES 100
#define GRID_SIZE 2 // nxn grid (2x2)

/**
 * The above code defines a struct type called Coordinate with two double fields, x and y.
 * @property {double} x - A double value representing the x-coordinate of a point in a coordinate
 * system.
 * @property {double} y - The "y" property is a double data type, which means it can store decimal
 * numbers. It represents the y-coordinate of a point in a two-dimensional coordinate system.
 */
typedef struct {
    double x;
    double y;
} Coordinate;

/**
 * The Distance type represents a coordinate and its corresponding distance.
 * @property {Coordinate} coordinate - The coordinate property is a struct that represents a point in a
 * two-dimensional space. It typically contains two properties, x and y, which represent the x and y
 * coordinates of the point.
 * @property {double} distance - The distance property is a double data type, which represents a
 * numerical value indicating the distance between two points.
 */
typedef struct {
    Coordinate coordinate;
    double distance;
} Distance;

/**
 * The GridCell struct represents a cell in a grid, containing coordinates, a centroid, distances, and
 * a sum of distances.
 * @property {double} x - A double representing the x-coordinate of the grid cell.
 * @property {double} y - The y-coordinate of the grid cell.
 * @property {Coordinate} coordinates - coordinates is an array of pointers to Coordinate objects. It
 * has a maximum size of MAX_COORDINATES.
 * @property {int} numCoordinates - The variable "numCoordinates" represents the number of coordinates
 * stored in the "coordinates" array.
 * @property {Coordinate} centroid - The centroid property represents the center point of the grid
 * cell. It is a Coordinate struct that contains the x and y coordinates of the centroid.
 * @property {Distance} distances - The "distances" property is a pointer to an array of Distance
 * objects. It is used to store the distances between the GridCell and other GridCells in the grid.
 * @property {double} sumDistances - The sumDistances property is a double that represents the sum of
 * all the distances in the GridCell. It is used to keep track of the total distance within the cell.
 */
typedef struct {
    double x;
    double y;
    Coordinate* coordinates[MAX_COORDINATES];
    int numCoordinates;
    Coordinate centroid;
    Distance* distances;
    double sumDistances; // Sumatoria de las distancias
} GridCell;

/**
 * The function `load_json_file` reads a JSON file, loads its contents into a JSON object, and returns
 * the JSON object.
 * 
 * @param filename The filename parameter is a string that represents the name of the JSON file that
 * you want to load.
 * 
 * @return a pointer to a json_t object.
 */
json_t *load_json_file(const char *filename) {
    FILE *jsonFile = fopen(filename, "r");
    if (jsonFile == NULL) {
        printf("Error al abrir el archivo JSON\n");
        return NULL;
    }

    fseek(jsonFile, 0, SEEK_END);
    long fileSize = ftell(jsonFile);
    fseek(jsonFile, 0, SEEK_SET);

    char *buffer = (char *)malloc(fileSize + 1);
    if (buffer == NULL) {
        fclose(jsonFile);
        printf("Error de memoria al leer el archivo JSON\n");
        return NULL;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, jsonFile);
    fclose(jsonFile);

    if (bytesRead != fileSize) {
        free(buffer);
        printf("Error al leer el archivo JSON\n");
        return NULL;
    }

    buffer[fileSize] = '\0';

    json_error_t error;
    json_t *root = json_loads(buffer, 0, &error);
    free(buffer);

    if (!root) {
        printf("Error al cargar el archivo JSON: %s\n", error.text);
        return NULL;
    }

    return root;
}


/**
 * The function `extract_coordinates` takes a JSON array of coordinates and returns an array of
 * `Coordinate` structs.
 * 
 * @param root A pointer to a JSON object that represents an array of coordinates.
 * @param numCoordinates A pointer to a variable that will store the number of coordinates extracted
 * from the JSON array.
 * 
 * @return a pointer to an array of Coordinate structures.
 */
Coordinate *extract_coordinates(json_t *root, size_t *numCoordinates) {
    if (!json_is_array(root)) {
        printf("El archivo JSON no contiene un array\n");
        return NULL;
    }

    size_t count = json_array_size(root);
    if (count > MAX_COORDINATES) {
        printf("El número de coordenadas excede el tamaño máximo\n");
        return NULL;
    }

    Coordinate *coordinates = (Coordinate *)malloc(count * sizeof(Coordinate));
    if (coordinates == NULL) {
        printf("Error de memoria al almacenar las coordenadas\n");
        return NULL;
    }

    size_t i;
    for (i = 0; i < count; i++) {
        json_t *coord = json_array_get(root, i);
        if (!json_is_array(coord) || json_array_size(coord) != 2) {
            printf("Coordenada inválida en el archivo JSON\n");
            free(coordinates);
            return NULL;
        }

        json_t *xJson = json_array_get(coord, 0);
        json_t *yJson = json_array_get(coord, 1);

        if (!json_is_number(xJson) || !json_is_number(yJson)) {
            printf("Coordenada inválida en el archivo JSON\n");
            free(coordinates);
            return NULL;
        }

        coordinates[i].x = json_number_value(xJson);
        coordinates[i].y = json_number_value(yJson);
    }

    *numCoordinates = count;
    return coordinates;
}



/**
 * The function `print_coordinates` prints the x and y coordinates of an array of `Coordinate`
 * structures.
 * 
 * @param coordinates The "coordinates" parameter is a pointer to an array of Coordinate structures.
 * @param numCoordinates The parameter `numCoordinates` is the number of coordinates in the
 * `coordinates` array. It is of type `size_t`, which is an unsigned integer type used for representing
 * sizes and counts.
 */
void print_coordinates(Coordinate *coordinates, size_t numCoordinates) {
    printf("Coordenadas:\n");
    for (size_t i = 0; i < numCoordinates; i++) {
        printf("(%.16f, %.16f)\n", coordinates[i].x, coordinates[i].y);
    }
}


/**
 * The function assigns coordinates to cells in a grid based on their position.
 * 
 * @param coordinates An array of Coordinate structures, representing the coordinates to be assigned to
 * the grid.
 * @param numCoordinates The number of coordinates in the "coordinates" array.
 * @param grid A pointer to a pointer to a GridCell. This will be used to store the grid.
 * @param gridSize The gridSize parameter represents the size of the grid. It determines the number of
 * cells in each row and column of the grid.
 */
void assign_coordinates_to_grid(Coordinate *coordinates, size_t numCoordinates, GridCell*** grid, int gridSize) {
    // Calcula el tamaño de la celda de la grid
    double cellSize = 1.0 / gridSize;

    // Crea la grid
    *grid = (GridCell**)malloc(gridSize * sizeof(GridCell*));
    for (int i = 0; i < gridSize; i++) {
        (*grid)[i] = (GridCell*)malloc(gridSize * sizeof(GridCell));
        for (int j = 0; j < gridSize; j++) {
            (*grid)[i][j].x = i * cellSize + cellSize / 2.0;
            (*grid)[i][j].y = j * cellSize + cellSize / 2.0;
            (*grid)[i][j].numCoordinates = 0;
        }
    }

    // Asigna las coordenadas a las celdas de la grid
    for (size_t i = 0; i < numCoordinates; i++) {
        double x = coordinates[i].x;
        double y = coordinates[i].y;

        int cellX = (int)(x * gridSize);
        int cellY = (int)(y * gridSize);

        if (cellX >= 0 && cellX < gridSize && cellY >= 0 && cellY < gridSize) {
            GridCell* cell = &((*grid)[cellX][cellY]);
            if (cell->numCoordinates < MAX_COORDINATES) {
                cell->coordinates[cell->numCoordinates] = &coordinates[i];
                cell->numCoordinates++;
            }
        }
    }
}



/**
 * The function calculates the centroids of each cell in a grid based on the coordinates stored in each
 * cell.
 * 
 * @param grid The parameter "grid" is a 2D array of GridCell pointers. It represents a grid of cells,
 * where each cell contains a set of coordinates.
 * @param gridSize The gridSize parameter represents the size of the grid, which is the number of rows
 * and columns in the grid.
 */
void calculate_cell_centroids(GridCell** grid, int gridSize) {
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            GridCell* cell = &grid[i][j];
            double sumX = 0.0;
            double sumY = 0.0;

            for (int k = 0; k < cell->numCoordinates; k++) {
                sumX += cell->coordinates[k]->x;
                sumY += cell->coordinates[k]->y;
            }

            cell->centroid.x = sumX / cell->numCoordinates;
            cell->centroid.y = sumY / cell->numCoordinates;
        }
    }
}



/**
 * The function calculates the distances between the centroid of each grid cell and its coordinates.
 * 
 * @param grid The grid is a 2D array of GridCell pointers. Each GridCell represents a cell in the grid
 * and contains information such as the number of coordinates it has and an array of Coordinate
 * pointers.
 * @param gridSize The gridSize parameter represents the size of the grid, which is the number of rows
 * and columns in the grid.
 */
void calculate_distances(GridCell** grid, int gridSize) {
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            GridCell* cell = &grid[i][j];
            int numCoordinates = cell->numCoordinates;
            cell->distances = (Distance*)malloc(numCoordinates * sizeof(Distance));

            for (int k = 0; k < numCoordinates; k++) {
                cell->distances[k].coordinate = *(cell->coordinates[k]);
                cell->distances[k].distance = sqrt(pow(cell->centroid.x - cell->coordinates[k]->x, 2) +
                                                   pow(cell->centroid.y - cell->coordinates[k]->y, 2));
            }
        }
    }
}

/**
 * The function heapify is used to maintain the heap property in a binary heap data structure.
 * 
 * @param arr The parameter "arr" is an array of type "Distance".
 * @param n The parameter `n` represents the size of the array `arr`. It indicates the number of
 * elements in the array that need to be heapified.
 * @param i The parameter "i" represents the index of the current node in the heap.
 */
void heapify(Distance arr[], int n, int i) {
    int largest = i;
    int l = 2 * i + 1;
    int r = 2 * i + 2;

    if (l < n && arr[l].distance > arr[largest].distance)
        largest = l;

    if (r < n && arr[r].distance > arr[largest].distance)
        largest = r;

    if (largest != i) {
        Distance temp = arr[i];
        arr[i] = arr[largest];
        arr[largest] = temp;

        heapify(arr, n, largest);
    }
}

/**
 * The function implements the heap sort algorithm to sort an array of Distance objects in ascending
 * order.
 * 
 * @param arr The arr parameter is an array of Distance objects.
 * @param n The parameter `n` represents the number of elements in the array `arr`.
 */
void heap_sort(Distance arr[], int n) {
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);

    for (int i = n - 1; i > 0; i--) {
        Distance temp = arr[0];
        arr[0] = arr[i];
        arr[i] = temp;

        heapify(arr, i, 0);
    }
}


/**
 * The function calculates the sum of distances for each cell in a grid.
 * 
 * @param grid The parameter "grid" is a pointer to a 2D array of GridCell objects. Each GridCell
 * object represents a cell in a grid and contains information such as the number of coordinates and an
 * array of distances.
 * @param gridSize The gridSize parameter represents the size of the grid, which is the number of rows
 * and columns in the grid.
 */
void calculate_sum_distances(GridCell** grid, int gridSize) {
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            GridCell* cell = &grid[i][j];
            int numCoordinates = cell->numCoordinates;
            cell->sumDistances = 0.0;

            for (int k = 0; k < numCoordinates; k++) {
                cell->sumDistances += cell->distances[k].distance;
            }
        }
    }
}

/**
 * The function finds the centroid with the minimum accumulated distance in a grid.
 * 
 * @param grid The parameter "grid" is a 2D array of GridCell pointers. Each element in the grid
 * represents a GridCell object.
 * @param gridSize The parameter `gridSize` represents the size of the grid, which is the number of
 * rows and columns in the grid. It indicates the dimensions of the 2D array `grid`.
 */
void find_centroid_with_min_distance(GridCell** grid, int gridSize) {
    double minDistance = -1;
    int minDistanceIndexX = -1;
    int minDistanceIndexY = -1;

    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            if (minDistance == -1 || grid[i][j].sumDistances < minDistance) {
                minDistance = grid[i][j].sumDistances;
                minDistanceIndexX = i;
                minDistanceIndexY = j;
            }
        }
    }

    if (minDistanceIndexX != -1 && minDistanceIndexY != -1) {
        GridCell minDistanceCell = grid[minDistanceIndexX][minDistanceIndexY];
        printf("Centroide con menor distancia acumulada: Celda (%.16f, %.16f)\n",
               minDistanceCell.x, minDistanceCell.y);
        printf("Distancia acumulada: %.16f\n", minDistance);
    }
}


int main() {
    const char *filename = "data/coordenadas.json";
    json_t *root = load_json_file(filename);
    if (root == NULL) {
        return 1;
    }

    size_t numCoordinates;
    Coordinate *coordinates = extract_coordinates(root, &numCoordinates);
    if (coordinates == NULL) {
        json_decref(root);
        return 1;
    }

    GridCell** grid;
    assign_coordinates_to_grid(coordinates, numCoordinates, &grid, GRID_SIZE);

    calculate_cell_centroids(grid, GRID_SIZE);
    calculate_distances(grid, GRID_SIZE);
    calculate_sum_distances(grid, GRID_SIZE);
    find_centroid_with_min_distance(grid, GRID_SIZE);

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            printf("Celda (%.16f, %.16f):\n", grid[i][j].x, grid[i][j].y);
            printf("Centroide: (%.16f, %.16f)\n", grid[i][j].centroid.x, grid[i][j].centroid.y);
            printf("Distancia acumulada: %.16f\n", grid[i][j].sumDistances);
            for (int k = 0; k < grid[i][j].numCoordinates; k++) {
                printf("(%.16f, %.16f)\n", grid[i][j].coordinates[k]->x, grid[i][j].coordinates[k]->y);
            }
            printf("\n");
        }
    }

    free(coordinates);
    json_decref(root);

    return 0;
}