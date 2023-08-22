#include "Model.h"

Model::Model(std::string path, std::shared_ptr<Program> program)
	: program{ program }
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
	
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cerr << "Failed to load scene at: " << path << std::endl;
		return;
	}

	std::cout << "Loading model: " << scene->mRootNode->mName.C_Str() << std::endl;

	directory = path.substr(0, path.find_last_of('/'));
	materials = std::vector<std::shared_ptr<Material>>{ scene->mNumMaterials, nullptr };

	load_node(scene->mRootNode, scene);
}

void Model::draw()
{
	for (Mesh m : meshes) {
		m.draw();
	}
}

void Model::load_node(aiNode* node, const aiScene* scene)
{
	// create meshes and materials referenced in current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		unsigned int idx = node->mMeshes[i];
		
		aiMesh* ai_mesh = scene->mMeshes[idx];
		
		std::vector<Vertex> vertices;
		std::vector<index> indices;

		if (!ai_mesh->HasPositions() || !ai_mesh->HasNormals()) {
			std::cerr << "Missing vertex information in model " << idx << std::endl;
			continue;
		}

		// vertices
		for (unsigned int j = 0; j < ai_mesh->mNumVertices; j++) {
			Vertex v;

			v.position = glm::vec3();

			v.position.x = ai_mesh->mVertices[j].x;
			v.position.y = ai_mesh->mVertices[j].y;
			v.position.z = ai_mesh->mVertices[j].z;

			v.normal = glm::vec3();
			v.normal.x = ai_mesh->mNormals[j].x;
			v.normal.y = ai_mesh->mNormals[j].y;
			v.normal.z = ai_mesh->mNormals[j].z;

			v.texture_coord = glm::vec2();
			if (ai_mesh->mTextureCoords[0]) {
				v.texture_coord.x = ai_mesh->mTextureCoords[0][j].x;
				v.texture_coord.y = ai_mesh->mTextureCoords[0][j].y;
			}
			else {
				v.texture_coord.x = 0.0f;
				v.texture_coord.y = 0.0f;
			}

			vertices.push_back(v);
		}
		
		// indices
		for (unsigned int j = 0; j < ai_mesh->mNumFaces; j++) {
			aiFace face = ai_mesh->mFaces[j];

			if (face.mNumIndices != 3u) {
				std::cerr << "Expect mesh to be triangulated" << std::endl;
				continue;
			}

			for (unsigned int k = 0; k < face.mNumIndices; k++)
				indices.push_back(face.mIndices[k]);
		}

		// material, optimize by having hash map of materials
		std::shared_ptr<Material> material;
		;
		if (!materials.at(ai_mesh->mMaterialIndex)) {
			aiMaterial* ai_material = scene->mMaterials[ai_mesh->mMaterialIndex];
			std::vector < std::pair<aiTextureType, std::string >> types = { {aiTextureType_DIFFUSE, "diffuse"}, {aiTextureType_SPECULAR, "specular"} };
			
			material = std::make_shared<Material>(ai_material, types, program, directory);
			materials.at(ai_mesh->mMaterialIndex) = material;
		}
		else {
			material = materials.at(ai_mesh->mMaterialIndex);
		}

		Mesh mesh {vertices, indices, material};
		meshes.push_back(mesh);
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		load_node(node->mChildren[i], scene);
	}
}
