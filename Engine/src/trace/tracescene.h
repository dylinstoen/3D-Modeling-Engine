#ifndef TRACESCENE_H
#define TRACESCENE_H

#include "bsptree.h"
#include "tracesceneobject.h"
#include "tracelight.h"

#include <vector>

class TraceScene
{
public:
    TraceScene(Scene* scene, bool use_acceleration);
    ~TraceScene();

    bool Intersect(const Ray& r, Intersection& i) const;

    std::vector<TraceSceneObject*> bounded_objects;
    std::vector<TraceSceneObject*> unbounded_objects;
    std::vector<TraceLight*> lights;
    //A good scene shouldn't use this and use diffuse interreflection instead
    bool uses_blinn_phong_ambient=false;

    TreeBox* tree;

private:
    void AddSceneObjects(SceneObject* obj, glm::mat4 model_matrix);
};

#endif // TRACESCENE_H
