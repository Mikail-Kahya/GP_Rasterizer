#include "SceneManager.h"

#include "Scene.h"

SceneManager::SceneManager()
{
	m_ScenePtrList.push_back(new dae::Scene_W8_Vehicle());
	m_ScenePtrList.push_back(new dae::Scene_W8_TukTuk());
	m_ScenePtrList.push_back(new dae::Scene_W7_List());
	m_ScenePtrList.push_back(new dae::Scene_W7_Strip());

	for (dae::Scene* scenePtr : m_ScenePtrList)
		scenePtr->Initialize();

	m_It = m_ScenePtrList.begin();
}

SceneManager::~SceneManager()
{
	for (dae::Scene* scenePtr : m_ScenePtrList)
		delete scenePtr;
}

dae::Scene* SceneManager::Next()
{
	++m_It;
	if (m_ScenePtrList.end() == m_It)
		m_It = m_ScenePtrList.begin();

	return *m_It;
}

dae::Scene* SceneManager::GetScene() const
{
	return *m_It;
}

void SceneManager::Update(dae::Timer* timerPtr)
{
	(*m_It)->Update(timerPtr);
}
