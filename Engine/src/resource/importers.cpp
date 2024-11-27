/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include <animator.h>
#include <resource/cubemap.h>
#include <resource/texture.h>
#include <resource/importers.h>
#include <SOIL.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// Uses SOIL to load the image found at path and returns its width and height.
std::unique_ptr<Texture> Importers::ImportTexture(const std::string& name, const std::string& path) {
    int width, height, channels;
    unsigned char* image = SOIL_load_image(path.c_str(), &width, &height, &channels, SOIL_LOAD_RGBA);

    // Make sure SOIL was able to load the image
    if (image == nullptr) {
        throw FileIOException("Failed to import image \"" + path + "\": " + strerror(errno));
    }

    // Create the texture object
    std::unique_ptr<Texture> tex = std::make_unique<Texture>(name, width, height, image);
    tex->Get<FileProperty>("Path")->Set(path);

    // Free the image buffer
    SOIL_free_image_data(image);

    return std::move(tex);
}

std::unique_ptr<Cubemap> Importers::ImportCubemap(const std::string& name, const std::string& path) {
    static std::string suffixes[] = { "ft","bk","up","dn","rt","lf" };
    auto base_filename_length = path.rfind(".");
    std::string extension = path.substr(base_filename_length);
    std::string base_filename = path.substr(0, base_filename_length-2);

    int resolution = 0;


    unsigned char** images = new unsigned char*[Cubemap::NUM_CUBEMAP_FACES];

    for (int i = 0; i < Cubemap::NUM_CUBEMAP_FACES; i++) {
        std::string filepath = base_filename + suffixes[i] + extension;

        int width, height, channels;
        unsigned char* image = SOIL_load_image(filepath.c_str(), &width, &height, &channels, SOIL_LOAD_RGBA);

        // Make sure SOIL was able to load the image
        if (image == nullptr) {
            throw FileIOException("Failed to import image \"" + path + "\": " + strerror(errno));
        }

        assert(width == height);
        if (resolution) assert(resolution == width);
        else resolution = width;

        images[i] = image;
    }

    // Create the cubemap object
    std::unique_ptr<Cubemap> cubemap = std::make_unique<Cubemap>(name, resolution, (const unsigned char**) images);
    cubemap->Get<FileProperty>("Path")->Set(path);

    // Free memory
    for (int i = 0; i < Cubemap::NUM_CUBEMAP_FACES; i++) {
        SOIL_free_image_data(images[i]);
    }
    delete [] images;

    return std::move(cubemap);
}

void assimp2vector(const aiVector3D* a, unsigned int count, std::vector<float>& v, int dim=3) {
    for (unsigned int i = 0; i < count; i++) {
        v.push_back(a[i].x);
        if (dim > 1) v.push_back(a[i].y);
        if (dim > 2) v.push_back(a[i].z);
    }
}
void assimp2vector(const aiColor4D* a, unsigned int count, std::vector<float>& v, int dim=3) {
    for (unsigned int i = 0; i < count; i++) {
        v.push_back(a[i].r);
        if (dim > 1) v.push_back(a[i].g);
        if (dim > 2) v.push_back(a[i].b);
        if (dim > 3) v.push_back(a[i].a);
    }
}

// Uses assimp to load the mesh found at path
std::unique_ptr<Mesh> Importers::ImportMesh(const std::string& name, const std::string& path) {
    const struct aiScene* scene = aiImportFile(path.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);
    if (scene == nullptr) {
        throw FileIOException("Failed to import mesh \"" + path + "\"");
    }
    if (scene->mNumMeshes == 0) {
        throw FileIOException("Invalid mesh \"" + path + "\"");
    }

    // TODO: what if there is more than one mesh in the imported file?
    const struct aiMesh* aimesh = nullptr;
    for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
        if (scene->mMeshes[i]->mFaces[i].mNumIndices == 3) {
            aimesh = scene->mMeshes[i];
            break;
        }
    }
    if (aimesh == nullptr) {
        throw FileIOException("Mesh with non-triangular faces \"" + path + "\"");
    }
    if (aimesh->HasPositions() && aimesh->HasFaces()) {
        std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>(name);
        mesh->Get<FileProperty>("Path")->Set(path);
        std::vector<unsigned int> triangles;
        std::vector<float> positions, colors, uvs, normals;
        assimp2vector(aimesh->mVertices, aimesh->mNumVertices, positions);
        for (unsigned int i = 0; i < aimesh->mNumFaces; i++) {
            for (int j = 0; j < 3; j++) {
                triangles.push_back(aimesh->mFaces[i].mIndices[j]);
            }
        }
        mesh->SetPositions(positions);
        mesh->SetTriangles(triangles);
        if (aimesh->HasVertexColors(0)) {
            assimp2vector(aimesh->mColors[0], aimesh->mNumVertices, colors);
            mesh->SetColors(colors);
        }
        if (aimesh->HasTextureCoords(0)) {
            assimp2vector(aimesh->mTextureCoords[0], aimesh->mNumVertices, uvs, 2);
            mesh->SetUVs(uvs);
        }
        if (aimesh->HasNormals()) {
            assimp2vector(aimesh->mNormals, aimesh->mNumVertices, normals);
            mesh->SetNormals(normals);
        }
        aiReleaseImport(scene);
        return std::move(mesh);
    } else {
        throw FileIOException("Invalid mesh \"" + path + "\"");
    }
}
