#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

class HeightMap {
public:
	HeightMap(int, int);
	HeightMap(int, int, float**);
	~HeightMap();

	float getValue(int, int);
	int numPoints();
	int getWidth() const {
		return m_width;
	}
	int getHeight() const {
		return m_height;
	}
private:
	int m_width, m_height;
	float** m_data;
};

#endif