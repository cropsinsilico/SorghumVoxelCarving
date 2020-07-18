#include "Renderable.h"

Renderable::Renderable(int order, const QMatrix4x4& worldMatrix) :
	m_order(order),
	m_worldMatrix(worldMatrix)
{
	
}
