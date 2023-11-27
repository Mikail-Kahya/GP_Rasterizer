#pragma once
#include <list>

namespace dae
{
	class Timer;
	class Scene;
}

class SceneManager final
{
public:
	SceneManager();
	~SceneManager();

	SceneManager(const SceneManager& other) = delete;
	SceneManager(SceneManager&& other) = delete;
	SceneManager& operator=(const SceneManager& other) = delete;
	SceneManager& operator=(SceneManager&& other) = delete;

	dae::Scene* Next();
	dae::Scene* GetScene() const;

	void Update(dae::Timer* timerPtr);

private:
	typedef std::list<dae::Scene*> ScenePtrs;

	ScenePtrs m_ScenePtrList{};
	ScenePtrs::iterator m_It{};
};

