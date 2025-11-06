#include "Text.h"
#include "../Game.h"

Text::Text()
{
	Scene& _scene = Game::GetScene();

	_text = _scene.CreateGameObject("Text");
	_scene.AddComponent<Frost::Transform>(
		_text,
		Frost::Transform::Vector3{ 0.0f, 5.0f, 3.0f },
		Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f },
		Frost::Transform::Vector3{ 1.0f, 1.0f, 1.0f }
	);
	_scene.AddComponent<Frost::WorldTransform>(_text, Frost::Transform::Vector3{ 0.0f, 0.0f, 0.0f });
	_scene.AddComponent<Frost::ModelRenderer>(_text, "./resources/meshes/text.fbx");
}