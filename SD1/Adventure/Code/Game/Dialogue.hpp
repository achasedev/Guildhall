#pragma once
#include <string>
#include "Engine/Math/AABB2.hpp"

class Dialogue
{
public:

	Dialogue(const std::string& text, const AABB2& textBox=DEFAULT_TEXT_BOX);
	void Update(float deltaTime);
	void Render() const;


public:

	static const AABB2 DEFAULT_TEXT_BOX;

private:

private:

	std::string m_text;
	AABB2 m_textBox;

	float m_ageInSeconds;
};