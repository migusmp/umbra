#pragma once
#include <assimp/scene.h>

#include <string>
#include <vector>

#include "mesh.hpp"

class Model {
   public:
    explicit Model(const std::string& path);

    void draw() const;

   private:
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);

    // Un Model puede tener varias Mesh (un modelo real suele estar
    // dividido en submallas, por ejemplo una por material). vector<Mesh>
    // funciona ahora que Mesh tiene semántica de movimiento definida.
    std::vector<Mesh> meshes;
};
