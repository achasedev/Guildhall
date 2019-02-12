/************************************************************************/
/* File: FFTSystem.hpp
/* Author: Andrew Chase
/* Date: February 6th 2019
/* Description: Class to represent an AudioSystem with FFT functionality
/************************************************************************/
#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVector2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Rendering/Meshes/Mesh.hpp"

class Stopwatch;

struct ThresholdDetectData_t
{
	Stopwatch							delayTimer = nullptr;
	float								delayInterval;

	FloatRange							frequencyRange;
	std::vector<float>					onceSecondSampleAverages;

	float								lastSampleRangeAverage = 0.f;
	float								historyAverage = -1.0f;
	float								historyVariance = -1.0f;

	bool								thresholdBrokenLastSample = false;

};

struct FFTBinData_t
{
	float binAverageOfAllChannels = 0.f;
	float isHigh = false;
};

struct FFTBinSpan_t
{
	FFTBinData_t*	binSpanData = nullptr;
	FloatRange		frequencySpan;
};

 

class FFTSystem : public AudioSystem
{
public:
	//-----Public Methods-----

	FFTSystem();
	virtual ~FFTSystem();

	virtual void	BeginFrame() override;
	void			ProcessInput();
	void			Render() const;

	void			PlayMusicTrackForFFT(SoundID soundID, float volume = 1.f);

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

	// Initialization
	void CreateAndAddFFTDSPToMasterChannel();
	void SetupFFTGraphUI();
	
	// Data collection
	void StartFFTDataCollection(const char* songPath);
	void SetupInitialFFTData();
	
	void UpdateFFTDataCollection();
	bool CheckForNewFFTSample();
	void UpdateLastFFTSample(float* newData);
	
	void CleanUpFFTData();

	void UpdateBarMesh();
	void UpdateGridAndPanelMesh();


private:
	//-----Private Data-----

	// FFT Data
	FMOD::DSP*							m_fftDSP = nullptr;
	FMOD_DSP_PARAMETER_FFT*				m_fmodCurrentFFTData = nullptr;
	float								m_sampleRate = -1.0f;
	const float							m_nyquistFreq = m_sampleRate * 0.5f;
	FMOD::Channel*						m_musicChannel = nullptr;

	// FFT Settings
	unsigned int						m_fftWindowSize = 1024;

	// FFT Calculated Data
	bool								m_collectingFFTData = false;
	float								m_maxValueLastFrame = 0.f;
	float*								m_lastFFTSample = nullptr;

	ThresholdDetectData_t				m_bassDrumData;
	ThresholdDetectData_t				m_snareDrumData;

	FFTBinSpan_t*						m_FFTData = nullptr;

	// Rendering
	bool								m_renderFFTGraph = true;
	float								m_fftMaxYAxis = 1.0f;
	unsigned int						m_binsToDisplay = 128;
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
