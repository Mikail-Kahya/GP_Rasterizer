#include "Scene.h"
#include "Utils.h"

namespace dae {

#pragma region SCENE W6
	void Scene_W6::Initialize()
	{
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
		m_Camera.Initialize(60.f, { .0f,.0f,-10.f });

        m_MeshVec = {
            Mesh
            {
                {
                    Vertex{{-3, 3, -2}},
                    Vertex{{ 0,  3, -2}},
                    Vertex{{ 3,  3, -2}},
                    Vertex{{-3,  0, -2}},
                    Vertex{{ 0,  0, -2}},
                    Vertex{{ 3,  0, -2}},
                    Vertex{{-3, -3, -2}},
                    Vertex{{ 0, -3, -2}},
                    Vertex{{ 3, -3, -2}}
                },
                {
                    3, 0, 1,    1, 4, 3,    4, 1, 2,
                    2, 5, 4,    6, 3, 4,    4, 7, 6,
                    7, 4, 5,    5, 8, 7
                },
                PrimitiveTopology::TriangleList
            }
        };
	}

	void Scene_W7_Strip::Initialize()
	{
        m_Camera.Initialize(60.f, { .0f,.0f,-10.f });

		m_MeshVec = {
            Mesh
            {
                {
                    Vertex{{-3, 3, -2}},
                    Vertex{{ 0,  3, -2}},
                    Vertex{{ 3,  3, -2}},
                    Vertex{{-3,  0, -2}},
                    Vertex{{ 0,  0, -2}},
                    Vertex{{ 3,  0, -2}},
                    Vertex{{-3, -3, -2}},
                    Vertex{{ 0, -3, -2}},
                    Vertex{{ 3, -3, -2}}
                },
                {
                    3, 0, 4,
                    1, 5, 2,
                    2, 6,
                    6, 3, 7,
                    4, 8, 5
                },
                PrimitiveTopology::TriangleStrip
            }
        };
	}
#pragma endregion
}