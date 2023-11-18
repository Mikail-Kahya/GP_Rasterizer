#pragma once
#include <string>
#include <vector>

#include "Math.h"
#include "DataTypes.h"
#include "Camera.h"

namespace dae
{
	//Scene Base Class
	class Scene
	{
	public:
		Scene() = default;
		virtual ~Scene() = default;

		Scene(const Scene&) = delete;
		Scene(Scene&&) noexcept = delete;
		Scene& operator=(const Scene&) = delete;
		Scene& operator=(Scene&&) noexcept = delete;

		virtual void Initialize() = 0;
		virtual void Update(dae::Timer* pTimer)
		{
			m_Camera.Update(pTimer);
		}

		const Camera& GetCamera() { return m_Camera; }
		const std::vector<Vertex>& GetVertices() const { return m_VertexVec; }
		const std::vector<Mesh>& GetMeshes() const { return m_MeshVec; }

	protected:
		std::string	sceneName;

		std::vector<Vertex> m_VertexVec{};
		std::vector<Mesh> m_MeshVec{};
		Camera m_Camera{};
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

	
}
