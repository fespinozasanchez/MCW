#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <jansson.h>



#define MAX_COORDINATES 100
#define GRID_SIZE 2 // nxn grid (2x2)


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

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            printf("Celda (%.16f, %.16f):\n", grid[i][j].x, grid[i][j].y);
            printf("Centroide: (%.16f, %.16f)\n", grid[i][j].centroid.x, grid[i][j].centroid.y);
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