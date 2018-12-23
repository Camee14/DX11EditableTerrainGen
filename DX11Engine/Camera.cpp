#include "Camera.h"

using namespace DirectX;

Camera::Camera() {

}
Camera::Camera(const Camera& other) {

}
Camera::~Camera() {

}

void Camera::update() {
	XMFLOAT3 up, pos, look_at;
	XMVECTOR up_vec, pos_vec, look_vec;
	float pitch, yaw, roll;
	XMMATRIX rot_matrix;

	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;
	up_vec = XMLoadFloat3(&up);

	pos_vec = XMLoadFloat3(&transform.getPosition());

	//look along positive z axis (into screen)
	look_at.x = 0.0f;
	look_at.y = 0.0f;
	look_at.z = 1.0f;
	look_vec = XMLoadFloat3(&look_at);

	pitch = transform.getRotation().x *  0.0174532925f;
	yaw = transform.getRotation().y *  0.0174532925f;
	roll = transform.getRotation().z *  0.0174532925f;
	rot_matrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	look_vec = XMVector3TransformCoord(look_vec, rot_matrix);
	up_vec = XMVector3TransformCoord(up_vec, rot_matrix);

	look_vec = XMVectorAdd(pos_vec, look_vec);

	m_view_matrix = XMMatrixLookAtLH(pos_vec, look_vec, up_vec);
}
void Camera::getView(XMMATRIX& mat) {
	mat = m_view_matrix;
}