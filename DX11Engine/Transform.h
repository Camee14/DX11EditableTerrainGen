#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <directxmath.h>

class Transform {
public:
	Transform();
	//Transform(const Transform&);
	~Transform();

	void setPosition(float, float, float);
	void setRotationPYR(float, float, float);
	void setScale(float, float, float);

	void translate(float, float, float);
	void rotatePYR(float, float, float);
	void scale(float, float, float);

	DirectX::XMFLOAT3 getPosition();
	DirectX::XMFLOAT3 getRotation();
	DirectX::XMFLOAT3 getScale();

	DirectX::XMMATRIX getTransformMatrix();

private:
	DirectX::XMFLOAT3 m_pos;
	DirectX::XMFLOAT3 m_rot;
	DirectX::XMFLOAT3 m_scl;

	DirectX::XMMATRIX m_pos_mat;
	DirectX::XMMATRIX m_rot_mat;
	DirectX::XMMATRIX m_scl_mat;
};

#endif
