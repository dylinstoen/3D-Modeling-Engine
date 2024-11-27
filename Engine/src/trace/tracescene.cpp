#include "tracescene.h"

#include <scene/scene.h>
#include <scene/sceneobject.h>

#include <scene/components/triangleface.h>

TraceScene::TraceScene(Scene *scene, bool use_acceleration)
{
    AddSceneObjects(&(scene->GetSceneRoot()), glm::mat4());

    if (!use_acceleration) {
        for (auto obj : bounded_objects) {
            unbounded_objects.push_back(obj);
        }
        bounded_objects.clear();
    }

    tree = new TreeBox(bounded_objects);
}

TraceScene::~TraceScene()
{
    for (auto obj : bounded_objects)
        delete obj;
    bounded_objects.clear();

    for (auto obj : unbounded_objects)
        delete obj;
    unbounded_objects.clear();

    for (auto light : lights)
        delete light;
    lights.clear();

    delete tree;
}

void TraceScene::AddSceneObjects(SceneObject* obj, glm::mat4 model_matrix) {
    if (obj->IsInternal() || !obj->IsEnabled()) {
        return;
    }

    model_matrix = model_matrix * obj->GetTransform().GetMatrix();

    Geometry* geo = obj->GetComponent<Geometry>();

    if (geo != nullptr && geo->RenderMaterial.Get() != nullptr && geo->RenderMaterial.Get()->PrepareToTrace()) {
        if (geo->UseCustomTrace()) {
            TraceGeometry* tso = new TraceGeometry(geo, model_matrix);
            if (tso->world_bbox == nullptr) {
                unbounded_objects.push_back(tso);
            } else {
                bounded_objects.push_back(tso);
            }
        } else {
            Mesh* mesh = geo->GetRenderMesh();
            if (mesh != nullptr) {

                glm::mat4 world2local = glm::inverse(model_matrix);
                glm::mat3 normallocal2world = glm::transpose(glm::mat3(world2local));

                std::vector<unsigned int> tris = mesh->GetTriangles();
                std::vector<float> positions = mesh->GetPositions();
                std::vector<float> uvs = mesh->GetUVs();
                std::vector<float> normals = mesh->GetNormals();

                for (size_t i=0; i<tris.size(); i+=3) {
                    //cringe
                    Geometry* tgeo = new TriangleFace(
                            glm::vec3(model_matrix*glm::vec4(positions[tris[i]*3],positions[tris[i]*3+1],positions[tris[i]*3+2],1)),
                            glm::vec3(model_matrix*glm::vec4(positions[tris[i+1]*3],positions[tris[i+1]*3+1],positions[tris[i+1]*3+2],1)),
                            glm::vec3(model_matrix*glm::vec4(positions[tris[i+2]*3],positions[tris[i+2]*3+1],positions[tris[i+2]*3+2],1)),
                            normals.size()>0 ? normallocal2world*glm::vec3(normals[tris[i]*3],normals[tris[i]*3+1],normals[tris[i]*3+2]) : glm::vec3(0,1,0),
                            normals.size()>0 ? normallocal2world*glm::vec3(normals[tris[i+1]*3],normals[tris[i+1]*3+1],normals[tris[i+1]*3+2]) : glm::vec3(0,1,0),
                            normals.size()>0 ? normallocal2world*glm::vec3(normals[tris[i+2]*3],normals[tris[i+2]*3+1],normals[tris[i+2]*3+2]) : glm::vec3(0,1,0),
                            uvs.size()>0 ? glm::vec2(uvs[tris[i]*2],uvs[tris[i]*2+1]) : glm::vec2(0,0),
                            uvs.size()>0 ? glm::vec2(uvs[tris[i+1]*2],uvs[tris[i+1]*2+1]) : glm::vec2(0,0),
                            uvs.size()>0 ? glm::vec2(uvs[tris[i+2]*2],uvs[tris[i+2]*2+1]) : glm::vec2(0,0),
                            normals.size()>0
                    );
                    tgeo->RenderMaterial.Set(geo->RenderMaterial.Get());
                    bounded_objects.push_back(new TraceGeometry(tgeo, true));
                }

            }
        }
    }

    Light* light = obj->GetComponent<Light>();

    // we do not supprt spotlight in ray tracing
    if ((light != nullptr) && (dynamic_cast<SpotLight*>(light) == nullptr)){
        TraceLight* tso = new TraceLight(light, model_matrix);
        lights.push_back(tso);

        if (glm::length2(light->Ambient.GetRGB()) > 0.f) {
            uses_blinn_phong_ambient = true;
        }

        if (dynamic_cast<DirectionalLight*>(light) == nullptr) {
            bounded_objects.push_back(new TraceFlare(tso));
        }
    }

    for(SceneObject* child : obj->GetChildren()) {
        AddSceneObjects(child, model_matrix);
    }
}

bool TraceScene::Intersect(const Ray& r, Intersection& i) const {
    bool intersect_found = false;

    Intersection cur;

    // try the non-bounded objects
    for (auto j = unbounded_objects.begin(); j != unbounded_objects.end(); j++) {
        if( (*j)->Intersect( r, cur ) && cur.t>0) {
            if( !intersect_found || (cur.t < i.t) ) {
                i = cur;
                intersect_found = true;
            }
        }
    }

    // Use the BSP tree to quickly intersect the ray with bounded objects
    if (tree->Intersect(r, cur)) {
        if (!intersect_found || (cur.t < i.t) ) {
            i = cur;
            intersect_found = true;
        }
    }

    // go over the BSP tree's objects without using the BSP tree to do so (SLOW)
      /*  for (auto j = bounded_objects.begin(); j != bounded_objects.end(); j++) {
            Intersection cur;
            if( (*j)->Intersect( r, cur ) && cur.t>0 ) {
                if( !intersect_found || (cur.t < i.t) ) {
                    i = cur;
                    intersect_found = true;
                }
            }
        } */

    // TODO: This is where you can get an amazing speed-up by
    // using an acceleration data structure to make intersection testing
    // more efficient!

    // Test for intersection with each object in the scene
//	for( iter j = objects.begin(); j != objects.end(); ++j ) {
//		isect cur;
//		if( (*j)->intersect( r, cur ) ) {
//			if( !have_one || (cur.t < i.t) ) {
//				i = cur;
//				have_one = true;
//			}
//		}
//	}

    if(!intersect_found) i.t = (-100);

    // if debugging,
    // if (Settings.Debugging) intersectCache.push_back(std::make_pair(r, i));

    return intersect_found;
}
