# Project MWC

### 📋 Languages

[![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)](#) [![Python](https://img.shields.io/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54)](#)

## Description

This project aims to develop an algorithm to find the minimum winning coalition. It involves generating random coordinates within a range of 0 to 1 to test the algorithm. Eventually, these coordinates will be obtained from another source and will be kept within the range of 0 to 1. The main idea is to organize the coordinates into a grid to allow partitioning into n x n subsections. The algorithm will calculate the centroid of each subsection and determine the distance between all points.

## Installation

To run the project, you need to have the following dependencies installed:

- Compiler: GCC
- Libraries: libjansson (JSON library)

Follow the steps below to install the necessary dependencies:

1. **GCC**: Install the GCC compiler for the C programming language based on your operating system. You can download it from the official GCC website or use a package manager specific to your OS.

2. **libjansson**: Install the libjansson library, which is used to handle JSON data. You can follow the installation instructions specific to your operating system from the official libjansson documentation.

## Usage

1. Clone the repository to your local machine or download the source code files.

2. Compile the C code:

   ```sh
   gcc -o main main.c -ljansson
   ```

3. Generate random coordinates (Python script):

- Make sure you have Python installed on your machine.
- Run the following command to generate random coordinates:
  ```sh
  python main.py
  ```
- This will generate a JSON file (`coordinates.json`), a CSV file (`coordinates.csv`), and a TXT file (`coordinates.txt`) containing the random coordinates.

4. Run the following command to execute the C program:
   ```sh
   ./main
   ```

## Project Structure

The project files are organized as follows:

- `main.c`: This file contains the C code that loads a JSON file, extracts the coordinates, and prints them.

- `main.py`: This file contains a Python script that generates random coordinates and saves them in JSON, CSV, and TXT formats.

- `data/`: This directory stores the generated coordinate files (`coordinates.json`, `coordinates.csv`, `coordinates.txt`).

## Contributions

Contributions to this project are welcome. If you have any suggestions, bug reports, or feature requests, please open an issue on the GitHub repository.

## License

This project is licensed under the MIT License.

## Contact

For any inquiries or additional information, please contact Felipe Andrés Espinoza Sánchez at felipe.espinoza2019@alu.uct.cl.

**GitHub**: [https://github.com/fespinozasanchez/](https://github.com/fespinozasanchez/)
