/************************************************************************/
/* File: FlyoutText.hpp
/* Author: Andrew Chase
/* Date: March 19th, 2018
/* Description: Class to represent world-space 2D text
/************************************************************************/
#pragma once
#include <string>
#include "Engine/Core/Rgba.hpp"
#include "Engine/Math/Vector3.hpp"

class Stopwatch;

class FlyoutText
{
public:
	//-----Public Methods-----

	FlyoutText(const std::string& text, const Vector3& position, const Rgba& color, float m_duration = 1.0f);

	void Render() const;
	bool IsFinished() const;

private:
	//-----Private Data-----

	std::string m_text;
	float		m_duration;
	Vector3		m_position;
	Rgba		m_startColor;

	Stopwatch*	m_stopwatch;

};
