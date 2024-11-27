/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "cubemap.h"

const int Cubemap::NUM_CUBEMAP_FACES = 6;

Cubemap::Cubemap(const std::string& name, unsigned int resolution, const unsigned char* faces[NUM_CUBEMAP_FACES]) :
    Asset(name),
    ExternalPath(FileType::Image),
    resolution_(resolution)
{
    AddProperty("Path", &ExternalPath);
    ExternalPath.SetHidden(true);

    // Store a copy of each face
    const unsigned int CHANNELS = 4; // RGBA
    unsigned int bytes_w = resolution_ * CHANNELS;
    for (int i = 0; i < NUM_CUBEMAP_FACES; i++) {
        if (resolution > 0 && faces != nullptr) {
            image_[i] = new unsigned char[bytes_w * bytes_w];
            for (unsigned int y = 0; y < resolution_; y++) {
                for (unsigned int x = 0; x < resolution_; x++) {
                    unsigned int j = x * CHANNELS;
                    // Copy over a pixel
                    for (unsigned int ch = 0; ch < CHANNELS; ch++) {
                        int idx = y * bytes_w + j + ch;
                        image_[i][idx] = faces[i][idx];
                    }
                }
            }
        } else {
            image_[i] = nullptr;
        }
    }
}

Cubemap::~Cubemap() {
    for (int i = 0; i < NUM_CUBEMAP_FACES; i++) {
        if (image_[i]) delete [] image_[i];
    }
}
