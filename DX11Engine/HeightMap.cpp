#include "HeightMap.h"

#include <math.h>  

HeightMap::HeightMap(int width, int height) {
	m_width = width;
	m_height = height;

	m_data = new float*[width];
	for (int i = 0; i < width; i++) {
		m_data[i] = new float[height];
	}

	for (int x = 0; x < m_width; x++) {
		for (int z = 0; z < m_height; z++) {
			//data[x][z] = 1.f;
			m_data[x][z] = (float)(sin((float)z / (m_width / 32))*3.0f);
		}
	}
}
HeightMap::HeightMap(int width, int height, float** data) {
	m_width = width;
	m_height = height;

	m_data = data;
}
HeightMap::~HeightMap() {
	/*for (int i = 0; i < m_width; i++) {
	delete[] data[i];
	}*/
	//delete[] data;
}
float HeightMap::getValue(int x, int z) {
	if ((x >= 0 && x < m_width) && (z >= 0 && z < m_height)) {
		return m_data[x][z];
	}
	return 0.f;
}
int HeightMap::numPoints() {
	return m_width * m_height;
}