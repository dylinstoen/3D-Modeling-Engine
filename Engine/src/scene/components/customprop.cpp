#include "customprop.h"

#include <scene/sceneobject.h>

REGISTER_COMPONENT(CustomProp, CustomProp)

CustomProp::CustomProp()
    : mp_root(NULL)
    , m_angle(0.0f, 0.0f, 359.0f, 1.0f) //for slider control (default value, min value, max value, step value). Remove this you don't want to the slider control
    //, m_your_prop(0.0f, 0.0f, 359.0f, 1.0f)
{
    AddProperty("Angle", &m_angle);
    m_angle.ValueChanged.Connect(this, &CustomProp::OnAngleChanged);

    /*
    AddProperty("Your Prop", &m_your_prop);
    m_your_prop.ValueChanged.Connect(this, &CustomProp::OnYourPropChanged);
    */
}

void CustomProp::OnAngleChanged(double angle)
{
    if (mp_root == NULL)
        return;

    std::cout << "angle = " << angle << std::endl;

    // Modify the code snippet below to realize your slider control
    /*////////////////////////////////////////////////////////////////////
    // Find the child node whose name is "PUT_YOUR_PART_NAME_HERE"
    SceneObject *p_part = mp_root->FindDescendantByName("PUT_YOUR_PART_NAME_HERE");}
    if (!p_part)
        return;

    // Get Transfrom Property object
    Transform* p_trans = p_part->GetComponent<Transform>();
    if (p_trans)
    {
        // Get Rotation Vector and set z-rotation as the specifed angle
        glm::vec3 rotate_vec = p_trans->Rotation.Get();
        rotate_vec.z = angle;
        p_trans->Rotation.Set(rotate_vec);
    }
    ////////////////////////////////////////////////////////////////////*/
}

/*
void CustomProp::OnYourPropChanged(double your_prop)
{
    if (mp_root == NULL)
        return;

    std::cout << "Your Prop = " << your_prop << std::endl;
}
*/
