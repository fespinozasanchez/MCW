import random
import json
import csv


def generar_coordenadas(n):
    """
    This function generates n random coordinates in the form of tuples with x and y values between 0 and
    1.

    :param n: The number of coordinates to generate
    :return: The function `generar_coordenadas(n)` returns a list of `n` tuples, where each tuple
    contains two random float values between 0 and 1 representing x and y coordinates.
    """
    coordenadas = []
    for _ in range(n):
        x = random.random()
        y = random.random()
        coordenadas.append((x, y))
    return coordenadas


# This code generates a list of 50 random coordinates in the form of tuples with x and y values
# between 0 and 1 using the `generar_coordenadas(n)` function. Then, it saves these coordinates in
# three different file formats: JSON, CSV, and TXT. Finally, it prints a message indicating that the
# coordinates have been saved in the three files.
numero_datos = 50
datos = generar_coordenadas(numero_datos)

# Guardar coordenadas en archivo JSON
with open('data/coordenadas.json', 'w') as json_file:
    json.dump(datos, json_file)

# Guardar coordenadas en archivo CSV
with open('data/coordenadas.csv', 'w', newline='') as csv_file:
    writer = csv.writer(csv_file)
    writer.writerow(['X', 'Y'])
    for coordenada in datos:
        writer.writerow(coordenada)

# Guardar coordenadas en archivo TXT
with open('data/coordenadas.txt', 'w') as txt_file:
    for coordenada in datos:
        txt_file.write(f"{coordenada[0]} {coordenada[1]}\n")

print("Coordenadas guardadas en archivos JSON, CSV y TXT.")
