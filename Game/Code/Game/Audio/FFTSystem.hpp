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

struct FFTBinData_t
{
	float binAverageOfAllChannels = 0.f;
	float isHigh = false;
	float timeIntoSong = 0.f;
};

struct FFTBinSpan_t
{
	std::vector<FFTBinData_t>	fftBinSamples;
	FloatRange					frequencyInterval;
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

	void						PlaySongAndCollectFFTData(const char* songPath);

	// FFT Mutators
	void						SetFFTGraphMaxXValue(float maxXValue);
	void						SetFFTGraphMaxYValue(float maxYValue);
	void						SetFFTWindowType(FMOD_DSP_FFT_WINDOW windowType);
	void						SetShouldRenderFFTGraph(bool shouldRender);

	// FFT Accessors
	bool						IsSetToRenderGraph();
	bool						IsPlaying() const;


private:
	//-----Private Methods-----

	// Initialization
	void CreateAndAddFFTDSPToMasterChannel();
	void SetupFFTGraphUI();
	
	// FFT Data
	bool CheckForNewFFTSample();
	void UpdateLastFFTSample(float* newData);
	void UpdateBarMesh();
	void UpdateGridAndPanelMesh();

	// Bin Data Collection
	void SetupForFFTPlayback();
	
	void AddCurrentFFTSampleToBinData();
	
	void WriteFFTBinDataToFile();
	void CleanUp();



private:
	//-----Private Data-----

	// FFT Data
	FMOD::DSP*							m_fftDSP = nullptr;
	FMOD_DSP_PARAMETER_FFT*				m_pointerToFMODFFTSpectrum = nullptr;
	float								m_sampleRate = -1.0f;
	FMOD::Channel*						m_musicChannel = nullptr;
	std::string							m_musicTitleBeingPlayed;
	Stopwatch*							m_playBackTimer = nullptr;
	float								m_songLength = 0.f;

	// FFT Settings
	const unsigned int					m_fftWindowSize = 1024;

	// FFT Data
	float								m_maxValueLastFrame = 0.f;
	float*								m_lastFFTSampleChannelAverages = nullptr;

	// FFT Bin Collection
	const float							m_maxFrequencyToSaveUpTo = 6000.f;
	int									m_numBinsToSaveUpTo = -1;
	std::vector<FFTBinSpan_t>			m_FFTBinSpans;

	// Rendering
	bool								m_renderFFTGraph = true;
	float								m_fftMaxYAxis = 1.0f;
	unsigned int						m_binsToDisplay = 128;
	Mesh*								m_barMesh = nullptr;
	Mesh*								m_gridMesh = nullptr;

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
