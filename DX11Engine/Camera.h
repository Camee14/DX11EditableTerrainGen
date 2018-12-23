#ifndef CAMERA_H
#define CAMERA_H

#include "Transform.h"

class Camera {
	public:
		Camera();
		Camera(const Camera&);
		~Camera();

		Transform transform;

		void update();

		void getView(DirectX::XMMATRIX&);
	private:
		DirectX::XMMATRIX m_view_matrix;
};

#endif
