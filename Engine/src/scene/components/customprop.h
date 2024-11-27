#ifndef CUSTOMPROP_H
#define CUSTOMPROP_H

#include <scene/components/component.h>

class CustomProp : public Component
{
public:
    CustomProp();

    void SetRoot(SceneObject *p_root){ mp_root = p_root; }

protected:
    DoubleProperty m_angle;
    void OnAngleChanged(double angle);

    /*
    DoubleProperty m_your_prop;
    void OnYourPropChanged(double your_prop);
    */

    SceneObject *mp_root;
};

#endif // CUSTOMPROP_H
