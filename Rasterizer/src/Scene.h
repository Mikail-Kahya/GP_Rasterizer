#pragma once
#include <string>
#include <vector>

#include "DataTypes.h"
#include "Camera.h"

namespace dae
{
	class Texture;

	//Scene Base Class
	class Scene
	{
	public:
		Scene();
		virtual ~Scene() = default; // No cleanup needed for statics

		Scene(const Scene&) = delete;
		Scene(Scene&&) noexcept = delete;
		Scene& operator=(const Scene&) = delete;
		Scene& operator=(Scene&&) noexcept = delete;

		virtual void Initialize() = 0;
		virtual void Update(dae::Timer* pTimer)
		{
			m_Camera.Update(pTimer);
		}

		const std::string& GetName() const { return sceneName; }
		Camera& GetCamera() { return m_Camera; }
		const std::vector<Vertex>& GetVertices() const { return m_VertexVec; }
		std::vector<Mesh>& GetMeshes() { return m_MeshVec; }
		static Material* GetMaterial(size_t materialIdx);

	protected:
		Texture* AddTexture(const std::string& path);
		Mesh& AddMesh(const std::string& path);
		size_t AddMaterial(Material* materialPtr);

		std::string	sceneName;
		Camera m_Camera{};

		std::vector<Vertex> m_VertexVec{};
		std::vector<Mesh> m_MeshVec{};

		inline static std::vector<Texture*> m_TexturePtrVec{};
		inline static std::vector<Material*> m_MaterialPtrVec{};
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 6 Test Scene
	class Scene_W6 final : public Scene
	{
	public:
		Scene_W6() = default;
		~Scene_W6() override = default;

		Scene_W6(const Scene_W6&) = delete;
		Scene_W6(Scene_W6&&) noexcept = delete;
		Scene_W6& operator=(const Scene_W6&) = delete;
		Scene_W6& operator=(Scene_W6&&) noexcept = delete;

		void Initialize() override;
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 7 List Scene
	class Scene_W7_List final : public Scene
	{
	public:
		Scene_W7_List() = default;
		~Scene_W7_List() override = default;

		Scene_W7_List(const Scene_W7_List&) = delete;
		Scene_W7_List(Scene_W7_List&&) noexcept = delete;
		Scene_W7_List& operator=(const Scene_W7_List&) = delete;
		Scene_W7_List& operator=(Scene_W7_List&&) noexcept = delete;

		void Initialize() override;
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 7 Strip Scene
	class Scene_W7_Strip final : public Scene
	{
	public:
		Scene_W7_Strip() = default;
		~Scene_W7_Strip() override = default;

		Scene_W7_Strip(const Scene_W7_Strip&) = delete;
		Scene_W7_Strip(Scene_W7_Strip&&) noexcept = delete;
		Scene_W7_Strip& operator=(const Scene_W7_Strip&) = delete;
		Scene_W7_Strip& operator=(Scene_W7_Strip&&) noexcept = delete;

		void Initialize() override;
	};


	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 8 Scene
	class Scene_W8_TukTuk final : public Scene
	{
	public:
		Scene_W8_TukTuk() = default;
		~Scene_W8_TukTuk() override = default;

		Scene_W8_TukTuk(const Scene_W8_TukTuk&) = delete;
		Scene_W8_TukTuk(Scene_W8_TukTuk&&) noexcept = delete;
		Scene_W8_TukTuk& operator=(const Scene_W8_TukTuk&) = delete;
		Scene_W8_TukTuk& operator=(Scene_W8_TukTuk&&) noexcept = delete;

		void Initialize() override;
	};

	//+++++++++++++++++++++++++++++++++++++++++
	//WEEK 8 Scene
	class Scene_W8_Vehicle final : public Scene
	{
	public:
		Scene_W8_Vehicle() = default;
		~Scene_W8_Vehicle() override = default;

		Scene_W8_Vehicle(const Scene_W8_Vehicle&) = delete;
		Scene_W8_Vehicle(Scene_W8_Vehicle&&) noexcept = delete;
		Scene_W8_Vehicle& operator=(const Scene_W8_Vehicle&) = delete;
		Scene_W8_Vehicle& operator=(Scene_W8_Vehicle&&) noexcept = delete;

		void Initialize() override;
	};
	
}
