#include "Transform.h"

#include <iostream>
using namespace DirectX;

Transform::Transform() {
	m_pos = XMFLOAT3(0.f, 0.f, 0.f);
	m_rot = XMFLOAT3(0.f, 0.f, 0.f);
	m_scl = XMFLOAT3(0.f, 0.f, 0.f);

	m_pos_mat = XMMatrixIdentity();
	m_rot_mat = XMMatrixIdentity();
	m_scl_mat = XMMatrixIdentity();
}
/*Transform::Transform(const Transform& other) {
	m_pos = other.m_pos;
	m_rot = other.m_rot;
	m_scl = other.m_scl;

	m_pos_mat = other.m_pos_mat;
	m_rot_mat = other.m_rot_mat;
	m_scl_mat = other.m_scl_mat;
}*/
Transform::~Transform() {

}
void Transform::setPosition(float x, float y, float z) {
	m_pos.x = x;
	m_pos.y = y;
	m_pos.z = z;

	m_pos_mat = XMMatrixTranslation(x, y, z);
}
void Transform::setRotationPYR(float p, float y, float r) {
	m_rot.x = p;
	m_rot.y = y;
	m_rot.z = r;

	m_rot_mat = XMMatrixRotationRollPitchYaw(p, y, r);
}
void Transform::setScale(float x, float y, float z) {
	m_scl.x = x;
	m_scl.y = y;
	m_scl.z = z;

	m_scl_mat = XMMatrixScaling(x, y, z);
}
void Transform::translate(float x, float y, float z) {
	m_pos.x += x;
	m_pos.y += y;
	m_pos.z += z;

	m_pos_mat = XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
}
void Transform::rotatePYR(float p, float y, float r) {
	m_rot.x += p;
	m_rot.y += y;
	m_rot.z += r;

	m_rot_mat = XMMatrixRotationRollPitchYaw(m_rot.x, m_rot.y, m_rot.z);
}
void Transform::scale(float x, float y, float z) {
	m_scl.x += x;
	m_scl.y += y;
	m_scl.z += z;

	m_scl_mat = XMMatrixScaling(m_scl.x, m_scl.y, m_scl.z);
}
DirectX::XMFLOAT3 Transform::getPosition() {
	return m_pos;
}
DirectX::XMFLOAT3 Transform::getRotation() {
	return m_rot;
}
DirectX::XMFLOAT3 Transform::getScale() {
	return m_scl;
}
DirectX::XMMATRIX Transform::getTransformMatrix() {
	return m_scl_mat * m_rot_mat * m_pos_mat;
}
