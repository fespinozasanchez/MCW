import random
import json
import csv


def generar_coordenadas(n):
    coordenadas = []
    for _ in range(n):
        x = random.random()
        y = random.random()
        coordenadas.append((x, y))
    return coordenadas


# Generar coordenadas
numero_datos = 50  # Cambia este valor según la cantidad de datos que desees generar
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
