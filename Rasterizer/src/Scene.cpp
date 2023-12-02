#include "Scene.h"

#include "Material.h"
#include "Texture.h"
#include "Utils.h"

namespace dae {

#pragma region SCENE W6
	Scene::Scene()
	{
        // Add default material on index 0 for every scene
        if (m_TexturePtrVec.empty())
            m_MaterialPtrVec.push_back(new Material_SolidColor{ AddTexture("uv_grid_2.png"), nullptr });
	}

	void Scene::Update(dae::Timer* timerPtr)
	{
        m_Camera.Update(timerPtr);

        if (!m_UpdateMeshes || m_MeshVec.empty())
            return;

        for (Mesh& mesh : m_MeshVec)
        {
            mesh.rotation.y += ROT_SPEED * timerPtr->GetElapsed();
            mesh.worldMatrix = Matrix::CreateRotationY(mesh.rotation.y) * Matrix::CreateTranslation(mesh.position);
        }
	}

	void Scene::ToggleMeshUpdates()
	{
        m_UpdateMeshes = !m_UpdateMeshes;
	}

	Material* Scene::GetMaterial(size_t materialIdx)
	{
        try
        {
            return m_MaterialPtrVec[materialIdx];
        }
        catch(...) // vector out of range exception
        {
            // 0 will always be the default texture
            return m_MaterialPtrVec[0];
        }
	}

	void Scene::DeleteResources()
	{
        for (Texture* texturePtr : m_TexturePtrVec)
            delete texturePtr;

        for (Material* materialPtr : m_MaterialPtrVec)
            delete materialPtr;
	}

	Texture* Scene::AddTexture(const std::string& path)
	{
        Texture* texturePtr{ Texture::LoadFromFile(path) };
        m_TexturePtrVec.push_back(texturePtr);
        return texturePtr;
	}

    Mesh& Scene::AddMesh(const std::string& path)
	{
        Mesh mesh{};

        Utils::ParseOBJ("Resources/" + path, mesh.vertices, mesh.indices);

        m_MeshVec.push_back(mesh);
        return m_MeshVec.back();
	}

    size_t Scene::AddMaterial(Material* materialPtr)
    {
        m_MaterialPtrVec.push_back(materialPtr);
        return m_MaterialPtrVec.size() - 1;
    }

    void Scene_W6::Initialize()
	{
        sceneName = "Scene W6: Test scene";
		m_Camera.Initialize(60.f, { .0f,.0f,-10.f });

		//default: Material id0 >> SolidColor Material (RED)
		m_VertexVec = {
			// Triangle 1
			{{0.f, 2.f, 0.f}, {1, 0, 0}},
			{{1.5f, -1.f, 0.f}, {1, 0, 0}},
			{{-1.5f, -1.f, 0.f}, {1, 0 ,0}},

			// Triangle 2
			{{0.f, 4.f, 2.f}, {1, 0, 0}},
			{{3.f, -2.f, 2.f}, {0, 1, 0}},
			{{-3.f, -2.f, 2.f}, {0, 0 ,1}}
		};
	}

	void Scene_W7_List::Initialize()
	{
        sceneName = "Scene W7: List mesh";
		m_Camera.Initialize(60.f, { .0f,.0f,-10.f });

        m_MeshVec = {
            Mesh
            {
                {
                    Vertex{{-3,  3, -2}, {colors::White}, {0.0f, 0.0f}},
                    Vertex{{ 0,  3, -2}, {colors::White}, {0.5f, 0.0f}},
                    Vertex{{ 3,  3, -2}, {colors::White}, {1.0f, 0.0f}},
                    Vertex{{-3,  0, -2}, {colors::White}, {0.0f, 0.5f}},
                    Vertex{{ 0,  0, -2}, {colors::White}, {0.5f, 0.5f}},
                    Vertex{{ 3,  0, -2}, {colors::White}, {1.0f, 0.5f}},
                    Vertex{{-3, -3, -2}, {colors::White}, {0.0f, 1.0f}},
                    Vertex{{ 0, -3, -2}, {colors::White}, {0.5f, 1.0f}},
                    Vertex{{ 3, -3, -2}, {colors::White}, {1.0f, 1.0f}}
                },
                {
                    3, 0, 1,    1, 4, 3,    4, 1, 2,
                    2, 5, 4,    6, 3, 4,    4, 7, 6,
                    7, 4, 5,    5, 8, 7
                },
                PrimitiveTopology::TriangleList
            }
        };

        m_MeshVec[0].materialIdx = 0;
	}

	void Scene_W7_Strip::Initialize()
	{
        sceneName = "Scene W7: Strip mesh";
        m_Camera.Initialize(60.f, { .0f,.0f,-10.f });

        m_MeshVec = {
            Mesh
            {
                {
                    Vertex{{-3,  3, -2}, {colors::White}, {0.0f, 0.0f}},
                    Vertex{{ 0,  3, -2}, {colors::White}, {0.5f, 0.0f}},
                    Vertex{{ 3,  3, -2}, {colors::White}, {1.0f, 0.0f}},
                    Vertex{{-3,  0, -2}, {colors::White}, {0.0f, 0.5f}},
                    Vertex{{ 0,  0, -2}, {colors::White}, {0.5f, 0.5f}},
                    Vertex{{ 3,  0, -2}, {colors::White}, {1.0f, 0.5f}},
                    Vertex{{-3, -3, -2}, {colors::White}, {0.0f, 1.0f}},
                    Vertex{{ 0, -3, -2}, {colors::White}, {0.5f, 1.0f}},
                    Vertex{{ 3, -3, -2}, {colors::White}, {1.0f, 1.0f}}
                },
                {
                    3, 0, 4,
                    1, 5, 2,
                    2, 6,
                    6, 3, 7,
                    4, 8, 5
                },
                PrimitiveTopology::TriangleStrip,
            }
        };


        m_MeshVec[0].materialIdx = 0;
        
	}

	void Scene_W8_TukTuk::Initialize()
	{
        sceneName = "Scene W8: Tuk Tuk";
        m_Camera.Initialize(60.f, { .0f,.0f,-10.f });

        Mesh& mesh{ AddMesh("tuktuk.obj") };

        mesh.primitiveTopology = PrimitiveTopology::TriangleList;
        mesh.materialIdx = AddMaterial(new Material_Lambert{ AddTexture("tuktuk.png"), nullptr, nullptr, nullptr, 7.f });
	}

	void Scene_W8_Vehicle::Initialize()
	{
        sceneName = "Scene W8: Vehicle";
        m_Camera.Initialize(45.f, { .0f,.0f,0.f });

        Mesh& mesh{ AddMesh("vehicle.obj") };

        mesh.primitiveTopology = PrimitiveTopology::TriangleList;
        mesh.materialIdx = AddMaterial(new Material_Lambert{
        	AddTexture("vehicle_diffuse.png"), 
            AddTexture("vehicle_normal.png"),
        	AddTexture("vehicle_specular.png"),
            AddTexture("vehicle_gloss.png"),
        	10.f
        });

        //mesh.position = { 0,0,50 };
	}
#pragma endregion
}