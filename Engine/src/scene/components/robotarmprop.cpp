#include "robotarmprop.h"

#include <scene/sceneobject.h>

REGISTER_COMPONENT(RobotArmProp, RobotArmProp)

RobotArmProp::RobotArmProp()
    : mp_root(NULL)
    , m_angle(0.0f, 0.0f, 359.0f, 1.0f)
{
    AddProperty("Angle", &m_angle);

    m_angle.ValueChanged.Connect(this, &RobotArmProp::OnAngleChanged);
}

void RobotArmProp::OnAngleChanged(double angle)
{
    if (mp_root == NULL)
        return;

    // Find the child node whose name is "Lower Arm"
    SceneObject *p_lower_arm = mp_root->FindDescendantByName("Lower Arm");
    if (!p_lower_arm)
        return;

    // Get Transfrom Property object
    Transform* p_trans = p_lower_arm->GetComponent<Transform>();
    if (p_trans)
    {
        // Get Rotation Vector and set z-rotation as the specifed angle
        glm::vec3 rotate_vec = p_trans->Rotation.Get();
        rotate_vec.z = angle;
        p_trans->Rotation.Set(rotate_vec);
    }
}
