/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#ifndef IMPORTERS_H
#define IMPORTERS_H

#include <animator.h>
#include <resource/texture.h>
#include <resource/mesh.h>

// Handles loading of disk-stored external resources.
class Importers {
public:
    // ImportTexture into memory as an unsigned char array (32 bit color depth).
    // I believe the reason 8 bits per color channel is used is most monitors operate with 32 bit color depth anyway,
    // and any HDR or higher bit depth images would need to be downsampled.
    // Readable Image Formats:
    //    BMP - non-1bpp, non-RLE (from stb_image documentation)
    //    PNG - non-interlaced (from stb_image documentation)
    //    JPG - JPEG baseline (from stb_image documentation)
    //    TGA - greyscale or RGB or RGBA or indexed, uncompressed or RLE
    //    DDS - DXT1/2/3/4/5, uncompressed, cubemaps (can't read 3D DDS files yet)
    //    PSD - (from stb_image documentation)
    //    HDR - converted to LDR, unless loaded with *HDR* functions (RGBE or RGBdivA or RGBdivA2)
    // Throws an exception if the texture was unable to be imported.
    static std::unique_ptr<Texture> ImportTexture(const std::string& name, const std::string& path);

    // Import Cubemap into memory as an array of 6 unsigned char arrays (32 bit color depth)
    // Cubemaps as downloaded from e.g. http://www.custommapmakers.org/skyboxes.php are
    // provided as 6 separate files, with the suffixes _ft, _bk, _up, _dn, _rt, and _lf
    // for the 6 faces. This function assumes that any one of these files was selected
    // as the path, and imports all of them.
    static std::unique_ptr<Cubemap> ImportCubemap(const std::string& name, const std::string& path);

    static std::unique_ptr<Mesh> ImportMesh(const std::string& name, const std::string& path);
};

#endif // IMPORTERS_H
