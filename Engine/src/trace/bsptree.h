/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
// The structure used in spatial subdivision to accelerate raytracing by
// reducing the number of intersection tests that need to be performed.
// Algorithm adapted from Graphics Gems III, "Ray Tracing with the BSP Tree",
// Kelvin Sung and Peter Shirley

#ifndef __BSPTREE_H__
#define __BSPTREE_H__

#include "tracesceneobject.h"

#include <vectors.h>
#include <list>
#include <algorithm>
#include <assert.h>

class TreeBox {
  public:
    TreeBox* left;
    TreeBox* right;
    TraceSceneObject* value;
    BoundingBox bounds;

    TreeBox(std::vector<TraceSceneObject*> objs) {
        left = nullptr;
        right = nullptr;
        if (objs.size() == 0) {
            value = nullptr;
            bounds = BoundingBox(glm::vec3(0,0,0), glm::vec3(0,0,0));
        } else if (objs.size() == 1) {
            value = objs[0];
            bounds = *(value->world_bbox);
        } else {
            value = nullptr;

            int bestIndex = -1;
            double bestSAH = 0;
            uint8_t bestAxis = 0;
            uint8_t bestSortBy = 0;

            int index = 0;
            double sah = 0;
            uint8_t axis = 0;
            uint8_t sortBy = 0;

            for (axis = 0; axis<3; axis++) {
                for (sortBy = 0; sortBy<2; sortBy++) { //dont try sortby=2 for speed right now
                    std::sort(objs.begin(), objs.end(), [this, axis, sortBy](const TraceSceneObject* a, const TraceSceneObject* b){ return compare(a, b, axis, sortBy); });
                    sah = CalcBestSAHCost(objs, index);
                    if (sah <= bestSAH || bestIndex == -1) {
                        bestIndex = index;
                        bestSAH = sah;
                        bestAxis = axis;
                        bestSortBy = sortBy;
                    }
                }
            }

            if (bestAxis != axis-1 || bestSortBy != sortBy-1) {
                std::sort(objs.begin(), objs.end(), [this, bestAxis, bestSortBy](const TraceSceneObject* a, const TraceSceneObject* b){ return compare(a, b, bestAxis, bestSortBy); });
            }

            auto mid = objs.begin() + bestIndex;
            left = new TreeBox(std::vector<TraceSceneObject*>(objs.begin(), mid));
            right = new TreeBox(std::vector<TraceSceneObject*>(mid, objs.end()));
            bounds = left->bounds;
            bounds += right->bounds;
        }
    }

    ~TreeBox() {
        if (value == nullptr) {
            delete left;
            delete right;
        }
    }

    bool Intersect(const Ray& r, Intersection& i) {
        double tMin;
        if (bounds.IntersectRay(r, tMin)) {
            return FastIntersect(r, i);
        }
        return false;
    }

    bool FastIntersect(const Ray& r, Intersection& i1) {
        // Fast on the assumption that the bounds have already been checked.
        if (value != nullptr) {
            return value->Intersect(r, i1);
        } else {
            double lMin, rMin;

            bool lHit = left->bounds.IntersectRay(r, lMin);
            bool rHit = right->bounds.IntersectRay(r, rMin);

            if (lHit && rHit) {
                Intersection i2;

                // Intersect closer box first
                if (lMin < rMin) {
                    lHit = left->FastIntersect(r, i1);
                    if (!lHit) {
                        return right->FastIntersect(r, i1);
                    } else if(i1.t > rMin) {
                        rHit = right->FastIntersect(r, i2);
                        if (rHit && i2.t < i1.t) {
                            i1 = i2;
                        }
                    }
                    return true;
                } else {
                    rHit = right->FastIntersect(r, i1);
                    if (!rHit) {
                        return left->FastIntersect(r, i1);
                    } else if(i1.t > lMin) {
                        lHit = left->FastIntersect(r, i2);
                        if (lHit && i2.t < i1.t) {
                            i1 = i2;
                        }
                    }
                    return true;
                }     
            } else if (lHit) {
                return left->FastIntersect(r, i1);
            } else if (rHit) {
                return right->FastIntersect(r, i1);
            } else {
                return false;
            }
        }
    }

    bool compare(const TraceSceneObject* a, const TraceSceneObject* b, uint8_t axis, uint8_t sortBy) {
        BoundingBox* boxA = a->world_bbox;
        BoundingBox* boxB = b->world_bbox;
        // Three cases
        // 1. Disjoint ranges
        // 2. One range is contained within the other
        // 3. Intersected ranges
        // All of them have the left-er range with a smaller max.

        glm::vec3 vec1 = boxA->max;
        glm::vec3 vec2 = boxB->max;
        if (sortBy == 1) {
            vec1 = boxA->min;
            vec2 = boxB->min;
        } else if (sortBy == 2) {
            vec1 += boxA->min;
            vec2 += boxB->min;
        }

        if (axis == 0) {
            return vec1.x < vec2.x;
        } else if (axis == 1) {
            return vec1.y < vec2.y;
        } else {
            return vec1.z < vec2.z;
        }
    }

    double CalcBestSAHCost(std::vector<TraceSceneObject*> &objs, int &bestIndex) {
        double bestCost = 0;
        double cost;
        BoundingBox l = *(objs[0]->world_bbox);
        BoundingBox r = *(objs[objs.size() - 1]->world_bbox);

        // Calculate the right side bounds' surface areas
        std::vector<double> rSurfaceAreas(objs.size());
        rSurfaceAreas[objs.size() - 1] = r.GetSurfaceArea();
        for (size_t j = objs.size() - 2; j != 0; j--) {
            r += *(objs[j]->world_bbox);
            rSurfaceAreas[j] = r.GetSurfaceArea();
        }

        for (size_t j = 1; j != objs.size(); j++) {
            cost = j * l.GetSurfaceArea() + (objs.size() - j) * rSurfaceAreas[j];

            if (cost < bestCost || bestCost == 0) {
                bestIndex = j;
                bestCost = cost;
            }

            // Update left bounds
            l += *(objs[j]->world_bbox);
        }
        return bestCost;
    }
};

#endif //__BSPTREE_H__
