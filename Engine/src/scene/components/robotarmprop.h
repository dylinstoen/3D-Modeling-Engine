#ifndef ROBOTARMPROP_H
#define ROBOTARMPROP_H

#include <scene/components/component.h>

class RobotArmProp : public Component
{
public:
    RobotArmProp();

    void SetRoot(SceneObject *p_root){ mp_root = p_root; }

protected:
    DoubleProperty m_angle;

    SceneObject *mp_root;

    void OnAngleChanged(double angle);
};

#endif // ROBOTARMPROP_H
