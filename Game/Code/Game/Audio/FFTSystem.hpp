/************************************************************************/
/* File: FFTSystem.hpp
/* Author: Andrew Chase
/* Date: February 6th 2019
/* Description: Class to represent an AudioSystem with FFT functionality
/************************************************************************/
#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Rendering/Meshes/Mesh.hpp"

class FFTSystem : public AudioSystem
{
public:
	//-----Public Methods-----

	FFTSystem();
	virtual ~FFTSystem();

	virtual void	BeginFrame() override;
	void			ProcessInput();
	void			Render() const;

	// FFT Mutators
	void						SetFFTGraphMaxXValue(float maxXValue);
	void						SetFFTGraphMaxYValue(float maxYValue);
	void						SetFFTWindowSize(int windowSize);
	void						SetFFTWindowType(FMOD_DSP_FFT_WINDOW windowType);
	void						SetShouldRenderFFTGraph(bool shouldRender);

	// FFT Accessors
	bool						IsSetToRenderGraph();


private:
	//-----Private Methods-----

	void SetupFFTGraphUI();
	void CreateAndAddFFTDSPToMasterChannel();

	void UpdateFFTData();
	void UpdateBarMesh();
	void UpdateGridAndPanelMesh();


private:
	//-----Private Data-----

	// FFT Data
	FMOD::DSP*							m_fftDSP = nullptr;
	FMOD_DSP_PARAMETER_FFT*				m_spectrumData = nullptr;
	const float							m_sampleRate = 48000.f; // TODO: Find out how to get FMOD to tell me the sample rate
	const float							m_nyquistFreq = m_sampleRate * 0.5f;

	// FFT Settings
	unsigned int						m_fftWindowSize = 4096;

	// FFT Calculated Data
	float								m_maxValueLastFrame = 0.f;

	// Rendering
	bool								m_renderFFTGraph = true;
	float								m_fftMaxYAxis = 1.0f;
	unsigned int						m_binsToDisplay = 512;
	mutable Mesh						m_barMesh;
	mutable Mesh						m_gridMesh;

	// UI Settings
	const float							m_graphHeight = 800.f;
	const float							m_fontHeight = 30.f;
	IntVector2							m_gridSegmentCount = IntVector2(20, 10);

	// Set during UI Setup
	AABB2	m_totalBounds;
	AABB2	m_maxValueBounds;
	AABB2	m_graphBounds;
	AABB2	m_headingBounds;

	AABB2	m_xAxisBounds;
	AABB2	m_yAxisBounds;
	AABB2	m_rightSidePanel;
	Vector2	m_screenEdgePadding;

	Rgba m_lineAndPanelColor	= Rgba(15, 60, 120, 200);
	Rgba m_backgroundColor		= Rgba(0, 0, 0, 100);
	Rgba m_fontColor			= Rgba(200, 200, 200, 200);


};
