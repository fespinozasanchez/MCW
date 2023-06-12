#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

/* These are preprocessor directives that define two constants: `MAX_COORDINATES` with a value of 100
and `GRID_SIZE` with a value of N. These constants are used throughout the program to set the
maximum number of coordinates that can be loaded from the JSON file and to define the size of the
grid used to group the coordinates. */
#define MAX_COORDINATES 100
#define GRID_SIZE 2 // nxn grid (2x2)

/**
 * The above type defines a structure named "Coordinate" with two double precision variables x and y.
 * @property {double} x - A double precision floating point number representing the x-coordinate of a
 * point in a two-dimensional coordinate system.
 * @property {double} y - The "y" property is a double data type that represents the vertical
 * coordinate value of a point in a two-dimensional Cartesian coordinate system.
 */
typedef struct {
    double x;
    double y;
} Coordinate;

/**
 * The type GridCell represents a cell in a grid with a centroid and a list of coordinates.
 * @property {double} x - A double value representing the x-coordinate of the GridCell.
 * @property {double} y - `y` is a double precision floating point number representing the y-coordinate
 * of the GridCell. It is a part of the GridCell struct which is used to represent a cell in a grid
 * system.
 * @property {Coordinate} coordinates - `coordinates` is an array of pointers to `Coordinate` objects.
 * The size of the array is `MAX_COORDINATES`. This array is used to store the `Coordinate` objects
 * that belong to the `GridCell`.
 * @property {int} numCoordinates - numCoordinates is an integer variable that represents the number of
 * Coordinate pointers stored in the coordinates array of a GridCell struct.
 * @property {Coordinate} centroid - The centroid is a Coordinate struct that represents the center
 * point of the GridCell. It is calculated based on the average x and y values of all the coordinates
 * within the GridCell.
 */
typedef struct {
    double x;
    double y;
    Coordinate* coordinates[MAX_COORDINATES];
    int numCoordinates;
    Coordinate centroid;
} GridCell;

/**
 * This function loads a JSON file and returns a JSON object.
 * 
 * @param filename a string containing the name of the JSON file to be loaded.
 * 
 * @return The function `load_json_file` returns a pointer to a `json_t` object, which represents the
 * root of a JSON data structure loaded from a file. If there is an error during the loading process,
 * the function returns `NULL`.
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
 * The function extracts coordinates from a JSON array and returns them as a Coordinate pointer.
 * 
 * @param root A pointer to a JSON object that contains an array of coordinates.
 * @param numCoordinates A pointer to a size_t variable that will be used to store the number of
 * coordinates extracted from the JSON array. The function will set this variable to the actual number
 * of coordinates extracted.
 * 
 * @return a pointer to an array of Coordinate structs, which represent the coordinates extracted from
 * a JSON array. It also sets the value of the numCoordinates parameter to the number of coordinates
 * extracted. If there is an error during the extraction process, the function returns NULL.
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
 * The function prints out a list of coordinates.
 * 
 * @param coordinates A pointer to an array of Coordinate structs.
 * @param numCoordinates The parameter `numCoordinates` is of type `size_t` and represents the number
 * of elements in the `coordinates` array. It is used in the `for` loop to iterate over all the
 * elements of the array.
 */
void print_coordinates(Coordinate *coordinates, size_t numCoordinates) {
    printf("Coordenadas:\n");
    for (size_t i = 0; i < numCoordinates; i++) {
        printf("(%.16f, %.16f)\n", coordinates[i].x, coordinates[i].y);
    }
}

/**
 * This function assigns coordinates to cells in a grid based on their position and stores them in the
 * corresponding cell.
 * 
 * @param coordinates An array of Coordinate structs representing the coordinates to be assigned to the
 * grid.
 * @param numCoordinates The number of coordinates to be assigned to the grid.
 * @param grid A pointer to a pointer to a GridCell, which will be allocated and filled with
 * coordinates based on the input parameters.
 * @param gridSize The size of the grid, which determines the number of cells in each row and column of
 * the grid.
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
 * The function calculates the centroids of each cell in a grid based on the coordinates of its
 * vertices.
 * 
 * @param grid a 2D array of GridCell pointers representing the grid
 * @param gridSize The size of the grid, which is the number of rows and columns in the 2D array of
 * GridCell pointers called "grid".
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