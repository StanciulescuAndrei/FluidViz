#pragma once
#include <stdlib.h>
#include <vector>

class Simulator {
private:
	float* heightMap = NULL;
	float* velocities = NULL;
	int gridSize = 0;

	int get2DPos(int x, int z) {
		return z * gridSize + x;
	}

	void fillGrid() {
		// Peste tot 0
		for (int x = 0; x < gridSize; x++) {
			for (int y = 0; y < gridSize; y++) {
				heightMap[get2DPos(x, y)] = 0;
				velocities[get2DPos(x, y)] = 0;
			}
		}

		// Introducem o 'picatura' initiala
		for (int x = 3 * gridSize / 8; x < 5 * gridSize / 8; x++) {
			for (int y = 3 * gridSize / 8; y < 5 * gridSize / 8; y++) {
				heightMap[get2DPos(x, y)] = 0.1;
				velocities[get2DPos(x, y)] = 0;
			}
		}

	}

	// Verificam sa fim in interiorul gridului
	bool checkBoundaries(int x, int y) {
		if (x < 1 || x >= gridSize-1)
			return false;
		if (y < 1 || y >= gridSize-1)
			return false;
		return true;
	}

public:
	float* getHeightMap() {
		return heightMap;
	}

	// Adaugam o picatura cat de cat rotunda
	void addDrop(int x, int y, int dropSize) {
		for (int dx = x - dropSize / 2; dx < x + dropSize / 2; dx++) {
			for (int dy = y - dropSize / 2; dy < y + dropSize / 2; dy++) {
				if (checkBoundaries(dx, dy) && (dx-x)*(dx-x) + (dy-y)*(dy-y) < dropSize*dropSize) {
					heightMap[get2DPos(dx, dy)] += 0.3;
				}
			}
		}
	}

	// Mai facem un pas al simularii
	// Propagarea se face prin calcularea mediei intaltimii din jur si ajustarea vitezei pe verticala pentru a se indrepta spre medie
	void step() {
		float kernel[3][3] = {
			{1, 1, 1},
			{1, 0, 1},
			{1, 1, 1}
		};
		
		for (int x = 1; x < gridSize-1; x++) {
			for (int y = 1; y < gridSize-1; y++) {
				float convSum = 0.0f;
				float kernelSum = 0.0f;
				int nx, ny;
				for (int dx = 0; dx < 3; dx++) {
					for (int dy = 0; dy < 3; dy++) {
						nx = x + dx - 1;
						ny = y + dy - 1;
						if (checkBoundaries(nx, ny)) {
							convSum += (kernel[dx][dy] * heightMap[get2DPos(nx, ny)]);
							kernelSum += kernel[dx][dy];
						}
					}
				}
				if (kernelSum > 0) {
					convSum /= kernelSum;
				}

				// Ajustam viteza ca sa ne apropiem de medie
				velocities[get2DPos(x, y)] += (convSum - heightMap[get2DPos(x, y)]);
				// Damping pe viteza ca altfel nu se mai opresc valurile
				velocities[get2DPos(x, y)] *= 0.98;
			}
		}

		// Facem update inaltimii in functie de viteza verticala
		for (int x = 1; x < gridSize - 1; x++) {
			for (int y = 1; y < gridSize - 1; y++) {
				heightMap[get2DPos(x, y)] += velocities[get2DPos(x, y)];
			}
		}
	}

	Simulator(int _gridSize) {
		this->gridSize = _gridSize;
		this->heightMap = new float[gridSize * gridSize];
		this->velocities = new float[gridSize * gridSize];
		this->fillGrid();

	}

	~Simulator() {
		if (heightMap != NULL) {
			delete[] heightMap;
		}
		if (velocities != NULL) {
			delete[] velocities;
		}
	}
};
