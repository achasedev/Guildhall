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

enum eFFTSystemState
{
	STATE_IDLE,
	STATE_JUST_PLAYING,
	STATE_COLLECTING_FFT_DATA,
	STATE_BEAT_DATA_PLAYBACK,
};

struct FFTBin_t
{
	float binAverageOfAllChannels = 0.f;
	float isHigh = false;
	float timeIntoSong = 0.f;
};

struct FFTBinSet_t
{
	std::vector<FFTBin_t>		fftBinSamples;
	FloatRange					frequencyInterval;

	float						periodMedian;
	float						periodMedianThreshold;
	int							periodsWithinThreshold;
	int							totalPeriods;
	float						periodConfidence;

	float						phaseMedian;
	float						phaseMedianThreshold;
	int							phasesWithinThreshold;
	int							totalPhases;
	float						phaseConfidence;

	float						averageBinExpressivity;
	float						averageBinExpressivityNormalized;
};

class File;

class FFTSystem : public AudioSystem
{
public:
	//-----Public Methods-----

	FFTSystem();
	virtual ~FFTSystem();

	virtual void	BeginFrame() override;
	void			ProcessInput();
	void			Render() const;

	void						PlaySongAndNothingElse(const char* songPath);
	void						CollectFFTDataFromSong(const char* songPath);
	void						PlaySongWithBeatAnalysisData(const char* songPath);

	// FFT Mutators
	void						SetFFTGraphMaxXValue(float maxXValue);
	void						SetFFTGraphMaxYValue(float maxYValue);
	void						SetFFTWindowType(FMOD_DSP_FFT_WINDOW windowType);

	// FFT Accessors
	bool						IsPlaying() const;

	// FFT Data File Processing
	void						PeformBeatDetectionAnalysis(const std::string& filename, float beatWindowDuration, float beatThresholdScalar, float delayAfterBeatDetected, float periodMedianThreshold, float phaseMedianThreshold);


private:
	//-----Private Methods-----

	void UpdateCollecting();
	void UpdateBeatPlayback();

	void RenderFFTGraph() const;
	void RenderBeatPlayback() const;

	bool LoadSoundTrackAndPlay();

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
	void FinalizeBinCollection();

	void WriteFFTBinDataToFile();
	void CleanUp();

	// Bin Data Analysis
	File*	LoadFFTDataFile(const std::string& filename) const;
	void	SetupForFFTBeatAnalysis(File* file);
	void	WriteFFTBeatAnalysisToFile();

	// Beat testing
	void LoadFFTBeatAnalysis(const std::string& filepath);


private:
	//-----Private Data-----

	eFFTSystemState m_systemState = STATE_IDLE;

	// FFT Data
	FMOD::DSP*							m_fftDSP = nullptr;
	FMOD_DSP_PARAMETER_FFT*				m_pointerToFMODFFTSpectrum = nullptr;
	float								m_sampleRate = -1.0f;
	FMOD::Channel*						m_musicChannel = nullptr;
	std::string							m_musicDataPath;
	float								m_songLength = 0.f;

	// FFT Settings
	const unsigned int					m_fftWindowSize = 1024;

	// FFT Data
	float								m_maxValueLastFrame = 0.f;
	float*								m_lastFFTSampleChannelAverages = nullptr;

	float								m_minBinExpressivityAverage = 10.f;
	int									m_minBinAverageBinIndex = 0;
	float								m_maxBinExpressivityAverage = -1;
	int									m_maxBinAverageBinIndex = 0;

	// FFT Bin Collection
	const float							m_maxFrequencyToSaveUpTo = 6000.f;
	int									m_numBinsToSaveUpTo = -1;
	std::vector<FFTBinSet_t>			m_FFTBinSets;

	// Beat Detection

	// Rendering
	float								m_fftMaxYAxis = 1.0f;
	unsigned int						m_binsToDisplay = 128;
	Mesh*								m_fftBarMesh = nullptr;
	Mesh*								m_fftGridMesh = nullptr;

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
