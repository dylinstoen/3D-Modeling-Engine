/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef TEXTURE_H
#define TEXTURE_H

#include <animator.h>
#include <resource/asset.h>
#include <resource/cacheable.h>

class Texture : public Asset, public Cacheable {
public:

    // Relative Path to the asset on disk, should be set after loading from disk.
    // Some assets are stored in the scene file, in which case they will not have an external path.
    FileProperty ExternalPath;
    BooleanProperty Bilinear;

    // Creates a texture from bytes
    Texture(const std::string& name, unsigned int width, unsigned int height, const unsigned char* image);
    ~Texture();

    virtual AssetType GetType() const override { return AssetType::Texture; }
    unsigned int GetWidth() const { return width_; }
    unsigned int GetHeight() const { return height_; }
    const unsigned char* GetImage() const { return image_; }
    const glm::vec4 GetColor(unsigned int x,unsigned int y) {
        assert(x < width_);
        assert(y < height_);
        unsigned char* pixel = image_ + (y * width_ + x)*4;
        return (glm::vec4{pixel[0],pixel[1],pixel[2],pixel[3]})/255.0f;
    }
    const glm::vec4 GetColorUV(glm::vec2 uv) {
        if (Bilinear.Get()) {
            glm::vec2 puv(uv.x * width_, uv.y * height_);
            glm::uvec2 luv(puv.x, puv.y);
            glm::vec2 blenduv = puv - glm::vec2{luv};
            glm::vec4 lox = glm::lerp(GetColor(luv.x%width_,luv.y%height_),
                                       GetColor((luv.x+1)%width_,luv.y%height_),
                                       blenduv.x);
            glm::vec4 hix = glm::lerp(GetColor(luv.x%width_,(luv.y+1)%height_),
                                       GetColor((luv.x+1)%width_,(luv.y+1)%height_),
                                       blenduv.x);
            return glm::lerp(lox, hix, blenduv.y);
        } else {
            glm::vec2 puv(uv.x * width_, uv.y * height_);
            glm::uvec2 luv(puv.x+0.5, puv.y+0.5);
            return GetColor(luv.x%width_,luv.y%height_);
        }
    }

    void OnChangeBilinear(bool use);

protected:
    unsigned int width_;
    unsigned int height_;
    unsigned char* image_;
};

class RenderableTexture: public Texture {
public:
    RenderableTexture(const std::string& name, unsigned int width, unsigned int height)
        : Texture(name, width, height, nullptr) {}
    virtual ~RenderableTexture() {}

    virtual AssetType GetType() const override { return AssetType::RenderableTexture; }
    void SetResolution(unsigned int width, unsigned int height) {
        width_ = width;
        height_ = height;
        MarkDirty();
    }
    glm::ivec2 GetResolution() const {
        return glm::ivec2(width_, height_);
    }
};

#endif // TEXTURE_H
