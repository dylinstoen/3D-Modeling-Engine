/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include <resource/texture.h>

Texture::Texture(const std::string &name, unsigned int width, unsigned int height, const unsigned char* image) :
    Asset(name),
    ExternalPath(FileType::Image),
    Bilinear(true),
    width_(width),
    height_(height)
{
    AddProperty("Path", &ExternalPath);
    AddProperty("Bilinear", &Bilinear);
    ExternalPath.SetHidden(true);
    Bilinear.ValueSet.Connect(this, &Texture::OnChangeBilinear);

    // Store a copy of the image
    const unsigned int CHANNELS = 4; // RGBA
    unsigned int bytes_width = width * CHANNELS;
    image_ = new unsigned char[bytes_width * height];
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            unsigned int i = x * CHANNELS;
            // Copy over a pixel
            for (unsigned int ch = 0; ch < CHANNELS; ch++)
                image_[y * bytes_width + i + ch] = image[y * bytes_width + i + ch];
        }
    }
}

void Texture::OnChangeBilinear(bool use) {
    MarkDirty();
}

Texture::~Texture() {
    delete [] image_;
}
