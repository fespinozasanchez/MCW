#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <jansson.h>

#define MAX_COORDINATES 100
#define GRID_SIZE 2 // nxn grid (2x2)
#define EDGE_SIZE 1.0 // la grid se dividen en GRID_SIZE (NXN) y EDGE_SIZE es el tamaño de la grid


typedef struct {
    double x;
    double y;
} Coordinate;


typedef struct {
    Coordinate coordinate;
    double distance;
} Distance;

typedef struct {
    double x;
    double y;
    Coordinate* coordinates[MAX_COORDINATES];
    int numCoordinates;
    Coordinate centroid;
    Distance* distances;
    double sumDistances; // Sumatoria de las distancias
} GridCell;

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


void print_coordinates(Coordinate *coordinates, size_t numCoordinates) {
    printf("Coordenadas:\n");
    for (size_t i = 0; i < numCoordinates; i++) {
        printf("(%.16f, %.16f)\n", coordinates[i].x, coordinates[i].y);
    }
}



void assign_coordinates_to_grid(Coordinate *coordinates, size_t numCoordinates, GridCell*** grid, int gridSize) {
    // Calcula el tamaño de la celda de la grid
    double cellSize = EDGE_SIZE / gridSize;

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
