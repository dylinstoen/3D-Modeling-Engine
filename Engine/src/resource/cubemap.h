/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef CUBEMAP_H
#define CUBEMAP_H

#include <animator.h>
#include <resource/asset.h>
#include <resource/cacheable.h>

class Cubemap: public Asset, public Cacheable {
public:
    static const int NUM_CUBEMAP_FACES;

    FileProperty ExternalPath;

    Cubemap(const std::string& name, unsigned int resolution, const unsigned char** faces);
    virtual ~Cubemap();

    virtual AssetType GetType() const override { return AssetType::Cubemap; }
    unsigned int GetResolution() const { return resolution_; }
    const unsigned char* GetFace(int face) const { return image_[face]; }

protected:
    unsigned int resolution_;
    unsigned char* image_[6];
};

class RenderableCubemap: public Cubemap {
public:
    RenderableCubemap(const std::string& name, unsigned int resolution)
        : Cubemap(name, resolution, nullptr) {}
    virtual ~RenderableCubemap() {}

    virtual AssetType GetType() const override { return AssetType::RenderableCubemap; }
    void SetResolution(unsigned int resolution) { resolution_ = resolution; MarkDirty(); }
};

#endif // CUBEMAP_H
