#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

/* `#define MAX_COORDINATES 100` is a preprocessor directive that defines a constant named
`MAX_COORDINATES` with a value of 100. This constant is used to limit the maximum number of
coordinates that can be read from the JSON file. */
#define MAX_COORDINATES 100

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
 * The function extracts coordinates from a JSON array and returns them as a Coordinate struct pointer.
 * 
 * @param root A pointer to a JSON object that contains an array of coordinates.
 * @param numCoordinates A pointer to a size_t variable that will store the number of coordinates
 * extracted from the JSON array.
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

    print_coordinates(coordinates, numCoordinates);

    free(coordinates);
    json_decref(root);

    // Pausa para mantener la ventana abierta
    printf("Presiona Enter para salir...");
    getchar();

    return 0;
}
