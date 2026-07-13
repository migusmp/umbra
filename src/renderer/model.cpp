#include "model.hpp"

#include <assimp/postprocess.h>

#include <assimp/Importer.hpp>
#include <iostream>

Model::Model(const std::string& path) {
    loadModel(path);
}

void Model::loadModel(const std::string& path) {
    Assimp::Importer importer;

    // Flags de post-procesado:
    // - Triangulate: si el modelo tiene quads o polígonos de más lados,
    //   los convierte a triángulos automáticamente (por eso el .obj de
    //   prueba puede tener caras de 4 vértices sin problema).
    // - GenNormals: si el modelo no trae normales calculadas, assimp las
    //   genera por nosotros (necesario para el .obj simple que no las trae).
    // - FlipUVs: OpenGL espera el origen de las UV en la esquina distinta
    //   a como muchos formatos las guardan — esto lo corrige automáticamente.
    // Ya no hace falta GenNormals: el .obj ahora trae normales explícitas
    // por cara (vn). Dejar el flag no rompería nada (assimp no sobreescribe
    // normales ya presentes), pero es más claro quitarlo si no se necesita.
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Error loading model '" << path << "': " << importer.GetErrorString()
                  << std::endl;
        std::exit(1);
    }

    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene) {
    // Un modelo assimp es un ÁRBOL de nodos (piensa en huesos/jerarquías,
    // aunque aquí no usemos animación). Cada nodo puede tener 0 o más
    // mallas asociadas, y 0 o más nodos hijos — hay que recorrerlo entero.

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* /*scene*/) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};

        vertex.position =
            glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

        if (mesh->HasNormals()) {
            vertex.normal =
                glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        }

        // Un modelo puede tener varios sets de UV (mTextureCoords[0..7]);
        // de momento solo nos interesa el primero, si existe.
        if (mesh->mTextureCoords[0]) {
            vertex.texCoords =
                glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        } else {
            vertex.texCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    // Cada "cara" (face) de assimp, tras Triangulate, es siempre un
    // triángulo: 3 índices que apuntan a vértices del array de arriba.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    return Mesh(vertices, indices);
}

void Model::draw() const {
    for (const auto& mesh : meshes) {
        mesh.draw();
    }
}
