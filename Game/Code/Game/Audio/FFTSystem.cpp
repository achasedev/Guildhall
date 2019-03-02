/************************************************************************/
/* File: FFTSystem.cpp
/* Author: Andrew Chase
/* Date: February 6th 2019
/* Description: Implementation of the FFTSystem class
/************************************************************************/
#include "Game/Audio/FFTSystem.hpp"
#include "Engine/Core/File.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/Resources/BitmapFont.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
FFTSystem::FFTSystem()
{
	CreateAndAddFFTDSPToMasterChannel();

	m_fftBarMesh = new Mesh();
	m_fftGridMesh = new Mesh();

	SetupFFTGraphUI();
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
FFTSystem::~FFTSystem()
{
	// Shouldn't have to delete these, as they're just convenience pointers
	m_fftDSP = nullptr;
	m_pointerToFMODFFTSpectrum = nullptr;
}


//-----------------------------------------------------------------------------------------------
// "Update"
//
void FFTSystem::BeginFrame()
{
	AudioSystem::BeginFrame();

	switch (m_systemState)
	{
	case STATE_IDLE:
		break;
	case STATE_JUST_PLAYING:
		break;
	case STATE_COLLECTING_FFT_DATA:
		UpdateCollecting();
		break;
	case STATE_BEAT_DATA_PLAYBACK:
		UpdateBeatPlayback();
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Checks for input to toggle graph settings
//
void FFTSystem::ProcessInput()
{
	InputSystem* input = InputSystem::GetInstance();

	int windowType;
	m_fftDSP->getParameterInt(FMOD_DSP_FFT_WINDOWTYPE, &windowType, 0, 0);

	// Only set the window type if the input this frame changed it
	int oldType = windowType;

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_LEFT_ARROW))
	{
		if (m_systemState == STATE_BEAT_DATA_PLAYBACK)
		{
			m_beatDisplayBinOffset--;
			m_beatDisplayBinOffset = ClampInt(m_beatDisplayBinOffset, 0, (int)m_FFTBinSets.size() - 4); // Display 4 windows at once
		}
		else if (m_systemState == STATE_COLLECTING_FFT_DATA)
		{
			windowType--;
		}
	}

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_RIGHT_ARROW))
	{
		if (m_systemState == STATE_BEAT_DATA_PLAYBACK)
		{
			m_beatDisplayBinOffset++;
			m_beatDisplayBinOffset = ClampInt(m_beatDisplayBinOffset, 0, (int)m_FFTBinSets.size() - 4); // Display 4 windows at once
		}
		else if (m_systemState == STATE_COLLECTING_FFT_DATA)
		{
			windowType++;
		}
	}

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_UP_ARROW))
	{
		if (m_systemState == STATE_BEAT_DATA_PLAYBACK)
		{
			int newValue = m_beatDisplayMode + 1;

			if (newValue >= NUM_BEAT_DISPLAY_MODES)
			{
				newValue = 0;
			}

			m_beatDisplayMode = (eBeatDisplayMode)newValue;
		}
	}

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_DOWN_ARROW))
	{
		if (m_systemState == STATE_BEAT_DATA_PLAYBACK)
		{
			int newValue = m_beatDisplayMode - 1;

			if (newValue < 0)
			{
				newValue = NUM_BEAT_DISPLAY_MODES - 1;
			}

			m_beatDisplayMode = (eBeatDisplayMode)newValue;
		}
	}

	// There are only 6 window types, 0 through 5
	if (windowType > 5)
	{
		windowType = 0;
	}
	else if (windowType < 0)
	{
		windowType = 5;
	}

	if (oldType != windowType)
	{
		SetFFTWindowType((FMOD_DSP_FFT_WINDOW)windowType);
	}
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Returns the string version of the given window type enumeration
//
std::string GetStringForWindowType(FMOD_DSP_FFT_WINDOW windowType)
{
	switch (windowType)
	{
	case FMOD_DSP_FFT_WINDOW_RECT:
		return "Rectangle";
		break;
	case FMOD_DSP_FFT_WINDOW_TRIANGLE:
		return "Triangle";
		break;
	case FMOD_DSP_FFT_WINDOW_HAMMING:
		return "Hamming";
		break;
	case FMOD_DSP_FFT_WINDOW_HANNING:
		return "Hanning";
		break;
	case FMOD_DSP_FFT_WINDOW_BLACKMAN:
		return "Blackman";
		break;
	case FMOD_DSP_FFT_WINDOW_BLACKMANHARRIS:
		return "Blackman-Harris";
		break;
	default:
		break;
	}

	return "";
}


//-----------------------------------------------------------------------------------------------
// Draws the FFT graph to the screen
//
void FFTSystem::Render() const
{
	switch (m_systemState)
	{
	case STATE_IDLE:
		break;
	case STATE_JUST_PLAYING:
		break;
	case STATE_COLLECTING_FFT_DATA:
		RenderFFTGraph();
		break;
	case STATE_BEAT_DATA_PLAYBACK:
		RenderBeatPlayback();
		break;
	default:
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Just plays a song, with no audio analysis
//
void FFTSystem::PlaySongAndNothingElse(const char* songPath)
{
	m_musicDataPath = songPath;
	LoadSoundTrackAndPlay();
}


//-----------------------------------------------------------------------------------------------
// Sets the max frequency value that is shown on the graph when rendererd (X-axis)
//
void FFTSystem::SetFFTGraphMaxXValue(float maxXValue)
{
	float frequencyPerSegment = m_sampleRate / (float)(m_fftWindowSize);
	m_binsToDisplay = Ceiling(maxXValue / frequencyPerSegment);

	SetupFFTGraphUI(); // Reset the graph info based on the changed window size
}


//-----------------------------------------------------------------------------------------------
// Sets the FFT Graph's max Y value to show, for scaling
//
void FFTSystem::SetFFTGraphMaxYValue(float maxYValue)
{
	m_fftMaxYAxis = maxYValue;

	SetupFFTGraphUI(); // Reset the graph info based on the changed window size
}


//-----------------------------------------------------------------------------------------------
// Sets the window type for the FFT DSP
//
void FFTSystem::SetFFTWindowType(FMOD_DSP_FFT_WINDOW windowType)
{
	FMOD_RESULT result = m_fftDSP->setParameterInt(FMOD_DSP_FFT_WINDOWTYPE, windowType);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't assign window type parameter");
}


//-----------------------------------------------------------------------------------------------
// Returns whether a song is currently being played (and analyzed)
//
bool FFTSystem::IsPlaying() const
{
	return (m_musicChannel != nullptr);
}


//-----------------------------------------------------------------------------------------------
// Loads and parses an FFT Data file and performs a beat analysis on the data
//
void FFTSystem::PeformBeatDetectionAnalysis(const std::string& filename, float beatWindowDuration, 
	float beatThresholdScalar, float delayAfterDetected, float periodMedianThreshold, float phaseMedianThreshold)
{
	if (IsPlaying())
	{
		ConsoleErrorf("Cannot perform analysis while a song is playing");
		return;
	}

	File* dataFile = LoadFFTDataFile(filename);
	
	if (dataFile == nullptr)
	{
		return;
	}

	dataFile->LoadFileToMemory();

	SetupForFFTBeatAnalysis(dataFile);

	dataFile->Close();
	delete dataFile;
	dataFile = nullptr;

	int numBins = (int) m_FFTBinSets.size();
	std::vector<FFTBin_t> rollingWindow;

	for (int binIndex = 0; binIndex < numBins; ++binIndex)
	{
		rollingWindow.clear();

		FFTBinSet_t& binSpan = m_FFTBinSets[binIndex];

		int numSamplesInBinSpan = (int) binSpan.fftBinSamples.size();

		// Setup initial history, cannot check for beats yet without one
		int sampleIndex = 0;

		for (sampleIndex = 0; sampleIndex < numSamplesInBinSpan; ++sampleIndex)
		{
			rollingWindow.push_back(binSpan.fftBinSamples[sampleIndex]);
			float durationStored = rollingWindow.back().timeIntoSong - rollingWindow.front().timeIntoSong;

			if (durationStored >= beatWindowDuration)
			{
				sampleIndex++;
				break;
			}
		}

		// We now have beatWindowDuration in our rolling window, begin checking for highs

		for (sampleIndex; sampleIndex < numSamplesInBinSpan; ++sampleIndex)
		{
			float windowAverage = 0.f;
			int samplesInHistory = (int)rollingWindow.size();

			for (int historyIndex = 0; historyIndex < samplesInHistory; ++historyIndex)
			{
				windowAverage += rollingWindow[historyIndex].binAverageOfAllChannels;
			}

			windowAverage /= (float)samplesInHistory;

			// If the current is beatThresholdScalar times bigger than the average, it's a beat
			if (binSpan.fftBinSamples[sampleIndex].binAverageOfAllChannels > (windowAverage * beatThresholdScalar))
			{
				binSpan.fftBinSamples[sampleIndex].isHigh = true;

				// Update the window to be a delay past the beat to avoid multiple hits
				float totalDurationAdvanced = 0.f;
				while (totalDurationAdvanced < delayAfterDetected)
				{
					rollingWindow.push_back(binSpan.fftBinSamples[sampleIndex]);

					float amountAdvancedFromThisSample = binSpan.fftBinSamples[sampleIndex].timeIntoSong - binSpan.fftBinSamples[sampleIndex - 1].timeIntoSong;
					totalDurationAdvanced += amountAdvancedFromThisSample;
					sampleIndex++;

					if (sampleIndex == (int)binSpan.fftBinSamples.size())
					{
						break;
					}
				}

				// Remove excess, not going under the window
				float durationStored = rollingWindow.back().timeIntoSong - rollingWindow.front().timeIntoSong;
				while ((durationStored > beatWindowDuration) && ((rollingWindow.size() > 1 && (durationStored - (rollingWindow[1].timeIntoSong - rollingWindow.front().timeIntoSong)) > beatWindowDuration)))
				{
					rollingWindow.erase(rollingWindow.begin());
					durationStored = rollingWindow.back().timeIntoSong - rollingWindow.front().timeIntoSong;
				}
			}
			else
			{
				// Else just advance the window forward one sample, removing enough to keep us just over the window duration
				rollingWindow.push_back(binSpan.fftBinSamples[sampleIndex]);
				sampleIndex++;

				// Remove excess, not going under the window
				float durationStored = rollingWindow.back().timeIntoSong - rollingWindow.front().timeIntoSong;
				while ((durationStored > beatWindowDuration) && ((rollingWindow.size() > 1 && (durationStored - (rollingWindow[1].timeIntoSong - rollingWindow.front().timeIntoSong)) > beatWindowDuration)))
				{
					rollingWindow.erase(rollingWindow.begin());
					durationStored = rollingWindow.back().timeIntoSong - rollingWindow.front().timeIntoSong;
				}
			}
		}

		// We now have all highs for this bin, so we can determine beat period, phase, and confidence
		float intervalStartTimeIntoSong = 0.f;
		float intervalEndTimeIntoSong = 0.f;
		std::vector<float> periodIntervals;

		// Setup time start at the first high
		for (sampleIndex = 0; sampleIndex < numSamplesInBinSpan; ++sampleIndex)
		{
			if (binSpan.fftBinSamples[sampleIndex].isHigh)
			{
				sampleIndex++;
				break;
			}
		}

		// Starting from the first high
		bool onBeatInterval = true;
		bool isStartTheEndOfThePreviousInterval = false;
		std::vector<float> beats;
		beats.push_back(binSpan.fftBinSamples[sampleIndex - 1].timeIntoSong);

		for (sampleIndex; sampleIndex < numSamplesInBinSpan; ++sampleIndex)
		{
			if (onBeatInterval)
			{
				intervalEndTimeIntoSong = binSpan.fftBinSamples[sampleIndex].timeIntoSong;

				float currentIntervalSeconds = intervalEndTimeIntoSong - intervalStartTimeIntoSong;

				// Set a limit to a beat we detect, so if we go over it then just throw it out (prevents outliers)
				if (currentIntervalSeconds > 0.75f)
				{
					if (!isStartTheEndOfThePreviousInterval)
					{
						beats.erase(beats.begin() + (beats.size() - 1));
					}

					onBeatInterval = false;
					isStartTheEndOfThePreviousInterval = false;
				}
				else if (binSpan.fftBinSamples[sampleIndex].isHigh) // We should already have a lower limit from the beat delay above, so no worry about small intervals
				{
					beats.push_back(intervalEndTimeIntoSong);
					periodIntervals.push_back(currentIntervalSeconds);

					intervalStartTimeIntoSong = intervalEndTimeIntoSong;
					isStartTheEndOfThePreviousInterval = true;
				}

				// Else do nothing in search of the end of the interval
			}	
			else
			{
				// We're looking for the start interval of the beat
				if (binSpan.fftBinSamples[sampleIndex].isHigh)
				{
					beats.push_back(binSpan.fftBinSamples[sampleIndex].timeIntoSong);
					intervalStartTimeIntoSong = binSpan.fftBinSamples[sampleIndex].timeIntoSong;
					onBeatInterval = true;
					isStartTheEndOfThePreviousInterval = false;
				}
			}
		}

		// We have all the intervals - find the median
		std::sort(periodIntervals.begin(), periodIntervals.end());
		int medianIndex = (int)(periodIntervals.size() / 2);

		float periodMedian = periodIntervals[medianIndex];

		binSpan.periodMedian = periodMedian;

		// Calculate the confidence 5%
		binSpan.totalPeriods = (int)beats.size();
		binSpan.periodMedianThreshold = periodMedianThreshold;
		int beatsAgreeingWithMedianWithinThreshold = 0;

		float maxErrorFromPeriodMedian = periodMedianThreshold * periodMedian;

		for (int periodIndex = 0; periodIndex < (int)periodIntervals.size(); ++periodIndex)
		{
			float diff = AbsoluteValue(periodIntervals[periodIndex] - periodMedian);

			if (diff <= maxErrorFromPeriodMedian)
			{
				beatsAgreeingWithMedianWithinThreshold++;
			}
		}

		binSpan.periodsWithinThreshold = beatsAgreeingWithMedianWithinThreshold;

		binSpan.periodConfidence = (float)binSpan.periodsWithinThreshold / (float)binSpan.totalPeriods;

		// Calculate the phase
		binSpan.phaseMedianThreshold = phaseMedianThreshold;
		std::vector<float> beatPhases;
		
		for (int beatPhaseIndex = 0; beatPhaseIndex < (int)beats.size(); ++beatPhaseIndex)
		{
			float currPhase = beats[beatPhaseIndex];

			while (currPhase - periodMedian > 0.f)
			{
				currPhase -= periodMedian;
			}

			beatPhases.push_back(currPhase);
		}

		binSpan.totalPhases = (int) beatPhases.size();

		// Take the median of the phases
		std::sort(beatPhases.begin(), beatPhases.end());
		int phaseMedianIndex = (int)(beatPhases.size() / 2);

		binSpan.phaseMedian = beatPhases[phaseMedianIndex];

		// Confidence in the phase
		int phasesAgreeingWithMedian = 0;
		float maxErrorFromPhaseMedian = binSpan.phaseMedian * phaseMedianThreshold;
		for (int beatPhaseIndex = 0; beatPhaseIndex < (int)beatPhases.size(); ++beatPhaseIndex)
		{
			float diff = AbsoluteValue(beatPhases[beatPhaseIndex] - binSpan.phaseMedian);

			if (diff <= maxErrorFromPhaseMedian)
			{
				phasesAgreeingWithMedian++;
			}
		}

		binSpan.phasesWithinThreshold = phasesAgreeingWithMedian;

		binSpan.phaseConfidence = (float) binSpan.phasesWithinThreshold / (float)binSpan.totalPhases;
	}

	// Done! Now write to file
	WriteFFTBeatAnalysisToFile();

	CleanUp();
}


//-----------------------------------------------------------------------------------------------
// Loads the FFT Data file by the given filename and returns it
//
File* FFTSystem::LoadFFTDataFile(const std::string& filename) const
{
	File* file = new File();
	bool opened = file->Open(filename.c_str(), "r");

	if (!opened)
	{
		ConsoleErrorf("Couldn't open file %s", filename.c_str());
		delete file;
		return nullptr;
	}

	return file;
}


//-----------------------------------------------------------------------------------------------
// Sets the UI parameters for rendering the graph
//
void FFTSystem::SetupFFTGraphUI()
{
	m_screenEdgePadding = Vector2(30.f, 100.f);

	Renderer* renderer = Renderer::GetInstance();
	AABB2 uiBounds = renderer->GetUIBounds();

	m_totalBounds = uiBounds;
	m_totalBounds.mins = m_screenEdgePadding;
	m_totalBounds.maxs = Vector2(uiBounds.maxs.x - m_screenEdgePadding.x, m_totalBounds.mins.y + m_graphHeight);

	// Max Value
	m_maxValueBounds.mins = m_totalBounds.mins;
	m_maxValueBounds.maxs = Vector2(m_maxValueBounds.mins.x + m_totalBounds.GetDimensions().x * 0.08f, m_totalBounds.maxs.y);

	// Axes
	m_yAxisBounds.mins = m_maxValueBounds.GetBottomRight();
	m_yAxisBounds.maxs = Vector2(m_yAxisBounds.mins.x + m_totalBounds.GetDimensions().x * 0.05f, m_totalBounds.maxs.y);

	m_xAxisBounds.mins = m_yAxisBounds.GetBottomRight();
	m_xAxisBounds.maxs = Vector2(m_totalBounds.maxs.x - m_totalBounds.GetDimensions().x * 0.05f, m_xAxisBounds.mins.y + 2.f * m_fontHeight);

	// Graph
	m_graphBounds = AABB2(m_xAxisBounds.GetTopLeft(), Vector2(m_xAxisBounds.maxs.x, m_totalBounds.maxs.y));
	m_maxValueBounds.mins.y = m_graphBounds.mins.y;

	// Heading
	m_headingBounds.mins = m_maxValueBounds.GetTopLeft();
	m_headingBounds.maxs = uiBounds.GetTopRight();
	m_headingBounds.maxs -= Vector2(m_screenEdgePadding.x); // Only pad on y by x padding, so it's not so much

	// Right Side Panel
	m_rightSidePanel.mins = m_xAxisBounds.GetBottomRight();
	m_rightSidePanel.maxs = m_totalBounds.GetTopRight();

	UpdateGridAndPanelMesh();
}


//-----------------------------------------------------------------------------------------------
// Starts playing a song and analying FFT data from it
//
void FFTSystem::CollectFFTDataFromSong(const char* songPath)
{
	if (IsPlaying())
	{
		ConsoleWarningf("Had to stop previous playback to start new FFT Bin analysis");
		CleanUp();
	}

	// Set the song path
	m_musicDataPath = songPath;
	m_systemState = STATE_COLLECTING_FFT_DATA;
}


//-----------------------------------------------------------------------------------------------
// Starts playback on a song with accompanying beat data for sound and visual testing
//
void FFTSystem::PlaySongWithBeatAnalysisData(const char* songName)
{
	if (IsPlaying())
	{
		ConsoleWarningf("Had to stop previous playback to start new FFT Bin analysis");
		CleanUp();
	}

	// Set the song path
	m_musicDataPath = Stringf("Data/Audio/Music/%s.mp3", songName);

	std::string beatAnalysisPath = Stringf("Data/FFTBeatAnalyses/%s.fftbeat", songName);
	LoadFFTBeatAnalysis(beatAnalysisPath);

	m_systemState = STATE_BEAT_DATA_PLAYBACK;
}


//-----------------------------------------------------------------------------------------------
// Sets all member data that can be immediately determined from the currently playing song,
// and sets up the initial state for FFT bin data analysis for updating as the song is playing
//
void FFTSystem::SetupForFFTPlayback()
{
	// Update sample rate
	m_musicChannel->getFrequency(&m_sampleRate);
	m_numBinsToSaveUpTo = Ceiling((float)m_fftWindowSize * (m_maxFrequencyToSaveUpTo / m_sampleRate));

	float freqSpanPerBin = m_sampleRate / m_fftWindowSize;

	m_FFTBinSets.reserve(m_numBinsToSaveUpTo);

	for (int binIndex = 0; binIndex < m_numBinsToSaveUpTo; ++binIndex)
	{
		FFTBinSet_t spanData;
		spanData.frequencyInterval = FloatRange(binIndex * freqSpanPerBin, (binIndex + 1) * freqSpanPerBin);

		m_FFTBinSets.push_back(spanData);
		m_FFTBinSets.back().fftBinSamples.reserve(10000);
	}
}


//-----------------------------------------------------------------------------------------------
// Update for collecting FFT data from an actively playing song
//
void FFTSystem::UpdateCollecting()
{
	if (!IsPlaying())
	{
		bool songJustLoaded = LoadSoundTrackAndPlay();
		SetupForFFTPlayback();

		// This ensures we don't start our timer until the song is already loaded, not paying for the delay
		if (songJustLoaded)
		{
			return;
		}
	}

	// Checks and updates the last sample from FMOD if it's new
	bool newSample = CheckForNewFFTSample();

	if (newSample)
	{
		AddCurrentFFTSampleToBinData();

		UpdateBarMesh();
		UpdateGridAndPanelMesh();
	}


	if (IsSoundFinished((SoundPlaybackID)m_musicChannel))
	{
		ConsolePrintf(Rgba::PURPLE, "Playback for song %s finished", m_musicDataPath.c_str());

		FinalizeBinCollection();
		WriteFFTBinDataToFile();
		CleanUp();
	}
}


//-----------------------------------------------------------------------------------------------
// Update for testing beat playback
//
void FFTSystem::UpdateBeatPlayback()
{
	if (!IsPlaying())
	{
		bool songJustLoaded = LoadSoundTrackAndPlay();

		// This ensures we don't start our timer until the song is already loaded, not paying for the delay
		if (songJustLoaded)
		{
			return;
		}
	}

	ASSERT_OR_DIE(IsPlaying(), "UpdateBeatPlayback() called when no song is playing");

	// Most of the "Updating" for this state occurs in Render(), in the form of checking the data
}


//-----------------------------------------------------------------------------------------------
// Renders the FFT data; Used when the system is collecting the FFT data during song playback
//
void FFTSystem::RenderFFTGraph() const
{
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(renderer->GetUICamera());
	AABB2 bounds = renderer->GetUIBounds();
	BitmapFont* font = AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png");

	renderer->DrawMeshWithMaterial(m_fftGridMesh, AssetDB::GetSharedMaterial("UI"));
	renderer->DrawMeshWithMaterial(m_fftBarMesh, AssetDB::GetSharedMaterial("Gradient"));

	std::string text = Stringf("Number of Channels: %i\n", m_pointerToFMODFFTSpectrum->numchannels);
	text += Stringf("Number of intervals displayed: %i (out of %i)\n", m_binsToDisplay, m_fftWindowSize);
	text += Stringf("Frequency resolution: %f hz\n", m_sampleRate / (float)m_fftWindowSize);
	text += Stringf("Sample Rate: %.0f hz\n", m_sampleRate);

	int windowType;
	m_fftDSP->getParameterInt(FMOD_DSP_FFT_WINDOWTYPE, &windowType, 0, 0);

	std::string windowTypeText = GetStringForWindowType((FMOD_DSP_FFT_WINDOW)windowType);

	text += Stringf("[Left, Right] Window Type: %s", windowTypeText.c_str());

	renderer->DrawTextInBox2D(text, m_headingBounds, Vector2::ZERO, m_fontHeight, TEXT_DRAW_SHRINK_TO_FIT, font, m_fontColor);

	// Draw x axis labels
	float maxFrequencyOnGraph = m_sampleRate * ((float)m_binsToDisplay / (float)m_fftWindowSize);

	float graphWidth = m_graphBounds.GetDimensions().x;
	float axisFontHeight = m_fontHeight * 0.5f;
	Vector2 xTextPos = Vector2(0.f, m_graphBounds.mins.y - axisFontHeight - 10.f);

	for (int i = 0; i <= m_gridSegmentCount.x; ++i)
	{
		float normalizedOffsetIntoXRange = ((float)i / (float)m_gridSegmentCount.x);
		float frequencyValue = normalizedOffsetIntoXRange * maxFrequencyOnGraph;
		std::string frequencyText = Stringf("%.0f", frequencyValue);

		float textWidth = font->GetStringWidth(frequencyText, axisFontHeight, 1.0f);

		xTextPos.x = m_graphBounds.mins.x + graphWidth * ((float)i / (float)m_gridSegmentCount.x) - (0.5f * textWidth);

		renderer->DrawText2D(Stringf("%.0f", frequencyValue), xTextPos, axisFontHeight, font, m_fontColor);
	}

	// Draw y axis labels
	float graphHeight = m_graphBounds.GetDimensions().y;

	for (int i = 0; i <= m_gridSegmentCount.y; ++i)
	{
		float value = (float)i / (float)m_gridSegmentCount.y;
		std::string labelText = Stringf("%.2f", value);
		float textWidth = font->GetStringWidth(labelText, axisFontHeight, 1.0f);

		Vector2 yTextPos;
		yTextPos.x = m_yAxisBounds.maxs.x - textWidth - 10.f;
		yTextPos.y = m_graphBounds.mins.y + (value * graphHeight) - (0.5f * axisFontHeight);
		renderer->DrawText2D(Stringf("%.2f", m_fftMaxYAxis * value), yTextPos, axisFontHeight, font, m_fontColor);
	}

	renderer->DrawTextInBox2D("Frequency (hz)", m_xAxisBounds, Vector2(0.5f, 1.f), m_fontHeight, TEXT_DRAW_OVERRUN, font, m_fontColor);

	float yPosition = RangeMapFloat(m_maxValueLastFrame, 0.f, m_fftMaxYAxis, 1.f, 0.0f);
	renderer->DrawTextInBox2D(Stringf("%.3f", m_maxValueLastFrame), m_maxValueBounds, Vector2(0.f, yPosition), m_fontHeight, TEXT_DRAW_SHRINK_TO_FIT, font, m_fontColor);
}


//-----------------------------------------------------------------------------------------------
// Returns the color used for the confidence when above certain thresholds
// Top 10% gets green, else top 25% gets yellow, otherwise gets red
//
Rgba GetFontColorForConfidence(const std::vector<FFTBinSet_t>& binSets, float confidence, bool isPeriod)
{
	int numBins = (int)binSets.size();
	int countGreaterThanGiven = 0;
	int maxThresholdForGood = numBins / 10;
	int maxThresholdForOkay = numBins / 4;

	for (int binIndex = 0; binIndex < numBins; ++binIndex)
	{
		float confidenceToCompareTo = (isPeriod ? binSets[binIndex].periodConfidence : binSets[binIndex].phaseConfidence);

		if (confidenceToCompareTo > confidence)
		{
			countGreaterThanGiven++;
		}
	}

	Rgba color = Rgba::RED;

	if (countGreaterThanGiven < maxThresholdForGood)
	{
		color = Rgba::GREEN;
	}
	else if (countGreaterThanGiven < maxThresholdForOkay)
	{
		color = Rgba::YELLOW;
	}

	return color;
}


//- C FUNCTION ----------------------------------------------------------------------------------
// Returns a string representing the mode given
//
std::string GetStringForBeatDisplayMode(eBeatDisplayMode mode)
{
	switch (mode)
	{
	case MODE_NORMAL:
		return "NORMAL";
		break;
	case MODE_MOST_CONFIDENT_PERIOD:
		return "MOST CONFIDENT PERIODS";
		break;
	case MODE_MOST_CONFIDENT_PHASE:
		return "MOST CONFIDENT PHASES";
		break;
	case MODE_MOST_EXPRESSIVE:
		return "MOST EXPRESSIVE";
		break;
	case MODE_LEAST_CONFIDENT_PERIOD:
		return "LEAST CONFIDENT PERIODS";
		break;
	case MODE_LEAST_CONFIDENT_PHASE:
		return "LEAST CONFIDENT PHASES";
		break;
	case MODE_LEAST_EXPRESSIVE:
		return "LEAST EXPRESSIVE";
		break;
	case BEST_QUALITY:
		return "BEST QUALITY";
		break;
	case WORST_QUALITY:
		return "WORST QUALITY";
		break;
	case NUM_BEAT_DISPLAY_MODES:
	default:
		return "YOU SHOULDN'T BE HERE";
		break;
	}
}


//-----------------------------------------------------------------------------------------------
// Renders the beat playback visuals for testing
//
void FFTSystem::RenderBeatPlayback() const
{
	static int lastBinSampleIndex = 0;

	unsigned int millisecondsIntoSong;
	m_musicChannel->getPosition(&millisecondsIntoSong, FMOD_TIMEUNIT_MS);
	float secondsIntoSong = (float)millisecondsIntoSong * 0.001f;
	
	// Render each bin to screen
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(renderer->GetUICamera());
	AABB2 uiBounds = renderer->GetUIBounds();
	Vector2 uiDimensions = uiBounds.GetDimensions();

	std::string modeText = Stringf("Display Mode: %s", GetStringForBeatDisplayMode(m_beatDisplayMode).c_str());

	renderer->DrawTextInBox2D(modeText, uiBounds, Vector2(0.5f, 0.f), 20.f, TEXT_DRAW_OVERRUN, AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png"), m_fontColor);
	AABB2 baseWindowBounds = AABB2(Vector2(0.f, 0.1f * uiDimensions.y), Vector2(0.25f * uiDimensions.x, 0.9f * uiDimensions.y));
	Vector2 baseWindowDimensions = baseWindowBounds.GetDimensions();

	int binIndicesToDisplay[4];
	GetBeatIndicesToDisplay(binIndicesToDisplay);

	for (int i = 0; i < 4; ++i)
	{
		int binSetIndex = binIndicesToDisplay[i];
		AABB2 interiorWindowBounds = baseWindowBounds;
		Vector2 interiorWindowDimensions = interiorWindowBounds.GetDimensions();

		interiorWindowBounds.AddPaddingToSides(-baseWindowDimensions.x * 0.05f, -baseWindowDimensions.y * 0.05f);

		const FFTBinSet_t& binSet = m_FFTBinSets[binSetIndex];

		renderer->Draw2DQuad(baseWindowBounds, AABB2::UNIT_SQUARE_OFFCENTER, m_lineAndPanelColor, AssetDB::GetSharedMaterial("UI"));
		renderer->Draw2DQuad(interiorWindowBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::BLACK, AssetDB::GetSharedMaterial("UI"));
		
		std::string headingText = Stringf("BIN: %i\nFreq: [%.2f - %.2f)", binSetIndex, binSet.frequencyInterval.min, binSet.frequencyInterval.max);
		renderer->DrawTextInBox2D(headingText, interiorWindowBounds, Vector2(0.5f, 0.f), 20.f, TEXT_DRAW_SHRINK_TO_FIT, AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png"), m_fontColor);

		std::string infoText;

		float fontHeight = 14.f;

		// Quality
				// Normalized Expressivity
		infoText = Stringf("Quality: %.3f", binSet.averageBinExpressivityNormalized * binSet.periodConfidence);
		renderer->DrawTextInBox2D(infoText, interiorWindowBounds, Vector2(0.f, 1.0f), fontHeight, TEXT_DRAW_OVERRUN, AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png"), m_fontColor);
		interiorWindowBounds.Translate(0.f, fontHeight);

		// Normalized Expressivity
		infoText = Stringf("Normalized Expressivity: %.3f", binSet.averageBinExpressivityNormalized);
		renderer->DrawTextInBox2D(infoText, interiorWindowBounds, Vector2(0.f, 1.0f), fontHeight, TEXT_DRAW_OVERRUN, AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png"), m_fontColor);
		interiorWindowBounds.Translate(0.f, fontHeight);

		// Expressivity
		infoText = Stringf("Expressivity: %.4f\n", binSet.averageBinExpressivity);
		renderer->DrawTextInBox2D(infoText, interiorWindowBounds, Vector2(0.f, 1.0f), fontHeight, TEXT_DRAW_OVERRUN, AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png"), m_fontColor);
		interiorWindowBounds.Translate(0.f, fontHeight);

		// Phase Confidence
		infoText = Stringf("Phase Confidence: %.2f%%", binSet.phaseConfidence * 100.f);
		Rgba phaseConfidenceColor = GetFontColorForConfidence(m_FFTBinSets, binSet.phaseConfidence, false);
		renderer->DrawTextInBox2D(infoText, interiorWindowBounds, Vector2(0.f, 1.0f), fontHeight, TEXT_DRAW_OVERRUN, AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png"), phaseConfidenceColor);
		interiorWindowBounds.Translate(0.f, fontHeight);

		// Phase
		infoText = Stringf("Phase: %.3f\n", binSet.phaseMedian);
		renderer->DrawTextInBox2D(infoText, interiorWindowBounds, Vector2(0.f, 1.0f), fontHeight, TEXT_DRAW_OVERRUN, AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png"), m_fontColor);
		interiorWindowBounds.Translate(0.f, fontHeight);

		// Period Confidence
		infoText = Stringf("Period Confidence: %.2f%%", binSet.periodConfidence * 100.f);
		Rgba periodConfidenceColor = GetFontColorForConfidence(m_FFTBinSets, binSet.periodConfidence, true);
		renderer->DrawTextInBox2D(infoText, interiorWindowBounds, Vector2(0.f, 1.0f), fontHeight, TEXT_DRAW_OVERRUN, AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png"), periodConfidenceColor);
		interiorWindowBounds.Translate(0.f, fontHeight);

		// Period
		infoText = Stringf("Period: %.3f", binSet.periodMedian);
		renderer->DrawTextInBox2D(infoText, interiorWindowBounds, Vector2(0.f, 1.0f), fontHeight, TEXT_DRAW_OVERRUN, AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png"), m_fontColor);
		interiorWindowBounds.Translate(0.f, fontHeight);
	
		
		Vector2 indicatorPosition = interiorWindowBounds.mins + Vector2(interiorWindowDimensions.x * 0.45f, interiorWindowDimensions.y * 0.5f);
		Vector2 indicatorDimensions = Vector2(interiorWindowDimensions.x * 0.3f);

		// Scale indicator dimensions based on expressivity
		indicatorDimensions *= binSet.averageBinExpressivityNormalized;

		AABB2 beatIndicatorBounds = AABB2(indicatorPosition - 0.5f * indicatorDimensions, indicatorPosition + 0.5f * indicatorDimensions);
		beatIndicatorBounds.Translate(0.f, 60.f);

		float timeWithPhaseRemoved = secondsIntoSong - binSet.phaseMedian;
		float timeIntoPeriod = ModFloat(timeWithPhaseRemoved, binSet.periodMedian);

		if (timeIntoPeriod < 0.05f)
		{
			renderer->Draw2DQuad(beatIndicatorBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::RED, AssetDB::GetSharedMaterial("UI"));
			renderer->DrawTextInBox2D("BEAT", beatIndicatorBounds, Vector2(0.5f), 50.f, TEXT_DRAW_SHRINK_TO_FIT, AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png"), Rgba::WHITE);
		}


		Vector2 qualityDimensions = Vector2(interiorWindowDimensions.x * 0.3f);
		AABB2 qualityBackgroundBounds = AABB2(indicatorPosition - 0.5f * qualityDimensions, indicatorPosition + 0.5f * qualityDimensions);
		qualityBackgroundBounds.Translate(0.f, -200.f);

		renderer->Draw2DQuad(qualityBackgroundBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::WHITE, AssetDB::GetSharedMaterial("UI"));
		
		// Scale based on period confidence and expressivity
		float fillWidth = qualityDimensions.x * binSet.periodConfidence;
		float fillHeight = qualityDimensions.y * binSet.averageBinExpressivityNormalized;

		AABB2 qualityFillBounds;
		qualityFillBounds.mins = qualityBackgroundBounds.mins;
		qualityFillBounds.maxs = qualityFillBounds.mins + Vector2(fillWidth, fillHeight);
		
		renderer->Draw2DQuad(qualityFillBounds, AABB2::UNIT_SQUARE_OFFCENTER, Rgba::RED, AssetDB::GetSharedMaterial("UI"));

		AABB2 qualityTextBounds = qualityBackgroundBounds;
		qualityTextBounds.Translate(0.f, 100.f);
		renderer->DrawTextInBox2D("Quality", qualityTextBounds, Vector2(0.5f), 50.f, TEXT_DRAW_SHRINK_TO_FIT, AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png"), m_fontColor);

		AABB2 qualityXLabelBounds = qualityBackgroundBounds;
		qualityXLabelBounds.Translate(0.f, -100.f);
		qualityXLabelBounds.AddPaddingToSides(20.f, 0.f);

		renderer->DrawTextInBox2D("Period Confidence", qualityXLabelBounds, Vector2(0.5f), 50.f, TEXT_DRAW_SHRINK_TO_FIT, AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png"), m_fontColor);

		AABB2 qualityYLabelBounds = qualityBackgroundBounds;
		qualityYLabelBounds.Translate(-145.f, 0.f);
		qualityYLabelBounds.AddPaddingToSides(-5.f, 0.f);

		renderer->DrawTextInBox2D("Normalized\nExpressivity", qualityYLabelBounds, Vector2(0.5f), 50.f, TEXT_DRAW_SHRINK_TO_FIT, AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png"), m_fontColor);

		baseWindowBounds.Translate(baseWindowDimensions.x, 0.f);
	}
}


//-----------------------------------------------------------------------------------------------
// Returns an array of 4 indices for which bins to display on screen
//
void FFTSystem::GetBeatIndicesToDisplay(int* out_beatIndices) const
{
	// check for normal mode here
	if (m_beatDisplayMode == MODE_NORMAL)
	{
		for (int i = 0; i < 4; ++i)
		{
			out_beatIndices[i] = m_beatDisplayBinOffset + i;
			ASSERT_OR_DIE(out_beatIndices[i] >= 0, "");
		}

		return;
	}

	std::vector<float> valuesSorted;
	std::vector<int> indicesSorted;
	for (int binIndex = 0; binIndex < (int)m_FFTBinSets.size(); ++binIndex)
	{
		float currValue = 0.f;
		bool checkIfCurrGreater = true;

		switch (m_beatDisplayMode)
		{
		case MODE_MOST_CONFIDENT_PERIOD:
			currValue = m_FFTBinSets[binIndex].periodConfidence;
			checkIfCurrGreater = true;
			break;
		case MODE_MOST_CONFIDENT_PHASE:
			currValue = m_FFTBinSets[binIndex].phaseConfidence;
			checkIfCurrGreater = true;
			break;
		case MODE_MOST_EXPRESSIVE:
			currValue = m_FFTBinSets[binIndex].averageBinExpressivity;
			checkIfCurrGreater = true;
			break;
		case MODE_LEAST_CONFIDENT_PERIOD:
			currValue = m_FFTBinSets[binIndex].periodConfidence;
			checkIfCurrGreater = false;
			break;
		case MODE_LEAST_CONFIDENT_PHASE:
			currValue = m_FFTBinSets[binIndex].phaseConfidence;
			checkIfCurrGreater = false;
			break;
		case MODE_LEAST_EXPRESSIVE:
			currValue = m_FFTBinSets[binIndex].averageBinExpressivity;
			checkIfCurrGreater = false;
			break;
		case BEST_QUALITY:
			currValue = m_FFTBinSets[binIndex].averageBinExpressivityNormalized * m_FFTBinSets[binIndex].periodConfidence;
			checkIfCurrGreater = true;
			break;
		case WORST_QUALITY:
			currValue = m_FFTBinSets[binIndex].averageBinExpressivityNormalized * m_FFTBinSets[binIndex].periodConfidence;
			checkIfCurrGreater = false;
			break;
		default:
			break;
		}

		bool inserted = false;
		for (int sortedIndex = 0; sortedIndex < (int)indicesSorted.size(); ++sortedIndex)
		{
			bool passedCheck = false;

			if (checkIfCurrGreater)
			{
				passedCheck = (valuesSorted[sortedIndex] < currValue);
			}
			else
			{
				passedCheck = (valuesSorted[sortedIndex] > currValue);
			}

			if (passedCheck)
			{
				valuesSorted.insert(valuesSorted.begin() + sortedIndex, currValue);
				indicesSorted.insert(indicesSorted.begin() + sortedIndex, binIndex);
				inserted = true;
				break;
			}
		}

		if (!inserted)
		{
			valuesSorted.push_back(currValue);
			indicesSorted.push_back(binIndex);
		}
	}

	// now take the 4 most confident
	for (int i = 0; i < 4; ++i)
	{
		out_beatIndices[i] = indicesSorted[i];
	}
}


//-----------------------------------------------------------------------------------------------
// Loads the sound from data and sets the given parameters on the FFTSystem
// Returns true if the song had to be loaded just now
//
bool FFTSystem::LoadSoundTrackAndPlay()
{
	// Load the track
	SoundID soundID = GetSound(m_musicDataPath);
	bool soundJustLoaded = false;

	if (soundID == MISSING_SOUND_ID)
	{
		soundID = CreateOrGetSound(m_musicDataPath);
		soundJustLoaded = true;
	}

	GUARANTEE_OR_DIE(soundID != MISSING_SOUND_ID, Stringf("Error: FFTSystem couldn't find song file %s", m_musicDataPath.c_str()).c_str());

	// Set the channel
	m_musicChannel = (FMOD::Channel*) AudioSystem::PlaySound(soundID, false, 1.0f);
	FMOD_RESULT result = m_musicChannel->addDSP(FMOD_CHANNELCONTROL_DSP_HEAD, m_fftDSP);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't ADD the DSP to the channel");	

	FMOD::Sound* sound = GetSoundForSoundID(soundID);

	unsigned int millisecondLength;
	sound->getLength(&millisecondLength, FMOD_TIMEUNIT_MS);
	m_songLength = ((float)millisecondLength / 1000.f);

	return soundJustLoaded;
}


//-----------------------------------------------------------------------------------------------
// Creates a Digital Signal Processor (DSP) and puts it on the system's main channel group
//
void FFTSystem::CreateAndAddFFTDSPToMasterChannel()
{
	FMOD::ChannelGroup* masterChannelGroup = nullptr;
	m_fmodSystem->getMasterChannelGroup(&masterChannelGroup);

	// Create and setup the FFT DSP, assigning it to the master channel group
	FMOD_RESULT result = m_fmodSystem->createDSPByType(FMOD_DSP_TYPE_FFT, &m_fftDSP);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't create the DSP");

	SetFFTWindowType(FMOD_DSP_FFT_WINDOW_BLACKMANHARRIS);

	result = m_fftDSP->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, m_fftWindowSize);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't assign window size parameter");
}


//---C FUNCTION----------------------------------------------------------------------------------
// Checks to see if the two spectrums match or not
//
bool DoSamplesMatch(float* oldSample, float* newSample, int length)
{
	ASSERT_OR_DIE(newSample != nullptr, "New Data in spectrum match was null");

	if (oldSample == nullptr)
	{
		return false;
	}

	bool match = true;
	for (int i = 0; i < length; ++i)
	{
		if (oldSample[i] != newSample[i])
		{
			match = false;
			break;
		}
	}

	return match;
}


//-----------------------------------------------------------------------------------------------
// Checks if the current sample in FMOD is a new sample to us, and updates members if so
// Returns true if so, false otherwise
//
bool FFTSystem::CheckForNewFFTSample()
{
	// If there's no song playing then there's no new sample
	if (!IsPlaying())
	{
		return false;
	}

	// Get the fft data
	void* spectrumData = nullptr;
	m_fftDSP->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void**)&spectrumData, 0, 0, 0);

	m_pointerToFMODFFTSpectrum = (FMOD_DSP_PARAMETER_FFT*)spectrumData;

	if (m_pointerToFMODFFTSpectrum == nullptr || m_pointerToFMODFFTSpectrum->length == 0)
	{
		return false;
	}

	// Check to see if there is a new sample this frame
	float* thisFrameSample = (float*)malloc(sizeof(float) * m_pointerToFMODFFTSpectrum->length);
	memset(thisFrameSample, 0, sizeof(float) * m_pointerToFMODFFTSpectrum->length);

	for (int binIndex = 0; binIndex < m_pointerToFMODFFTSpectrum->length; ++binIndex)
	{
		for (int channelIndex = 0; channelIndex < m_pointerToFMODFFTSpectrum->numchannels; ++channelIndex)
		{
			thisFrameSample[binIndex] += m_pointerToFMODFFTSpectrum->spectrum[channelIndex][binIndex];
		}

		// If we want average of the channels...
		thisFrameSample[binIndex] /= (float)m_pointerToFMODFFTSpectrum->numchannels;
	}

	bool receivedNewSampleThisFrame = !DoSamplesMatch(m_lastFFTSampleChannelAverages, thisFrameSample, m_pointerToFMODFFTSpectrum->length);

	if (receivedNewSampleThisFrame)
	{
		UpdateLastFFTSample(thisFrameSample);
	}
	else
	{
		free(thisFrameSample);
	}

	return receivedNewSampleThisFrame;
}


//-----------------------------------------------------------------------------------------------
// Updates the sum of the spectrum bins for the last frame, for detecting when a new sample is
// available
//
void FFTSystem::UpdateLastFFTSample(float* newData)
{
	if (m_lastFFTSampleChannelAverages != nullptr)
	{
		free(m_lastFFTSampleChannelAverages);
	}

	m_lastFFTSampleChannelAverages = newData;
}


//-----------------------------------------------------------------------------------------------
// Takes the current bin data sample from the FFT and adds it into the bin history
//
void FFTSystem::AddCurrentFFTSampleToBinData()
{
	for (int binIndex = 0; binIndex < m_numBinsToSaveUpTo; ++binIndex)
	{
		FFTBin_t binData;
		binData.binAverageOfAllChannels = m_lastFFTSampleChannelAverages[binIndex];
		unsigned int millisecondsIntoSong;
		m_musicChannel->getPosition(&millisecondsIntoSong, FMOD_TIMEUNIT_MS);
		binData.timeIntoSong = (float)millisecondsIntoSong * 0.001f;

		m_FFTBinSets[binIndex].fftBinSamples.push_back(binData);
	}
}


//-----------------------------------------------------------------------------------------------
// Finishes FFT data collection by calculating data on the entire set
//
void FFTSystem::FinalizeBinCollection()
{
	// Average expressivity for this bin throughout the track
	m_maxBinExpressivityAverage = -1.0f;
	m_minBinExpressivityAverage = 10.f;

	for (int binSetIndex = 0; binSetIndex < (int)m_FFTBinSets.size(); ++binSetIndex)
	{
		FFTBinSet_t& currSet = m_FFTBinSets[binSetIndex];

		int numSamplesInSet = (int)currSet.fftBinSamples.size();
		float averageExpressivity = 0.f;
		for (int sampleIndex = 0; sampleIndex < numSamplesInSet; ++sampleIndex)
		{
			averageExpressivity += currSet.fftBinSamples[sampleIndex].binAverageOfAllChannels;
		}

		averageExpressivity /= (float)numSamplesInSet;
		currSet.averageBinExpressivity = averageExpressivity;

		// Update global extremes
		if (averageExpressivity > m_maxBinExpressivityAverage)
		{
			m_maxBinExpressivityAverage = averageExpressivity;
			m_maxBinAverageBinIndex = binSetIndex;
		}

		if (averageExpressivity < m_minBinExpressivityAverage)
		{
			m_minBinExpressivityAverage = averageExpressivity;
			m_minBinAverageBinIndex = binSetIndex;
		}
	}

	// Normalized values from the max
	for (int binSetIndex = 0; binSetIndex < (int)m_FFTBinSets.size(); ++binSetIndex)
	{
		FFTBinSet_t& currSet = m_FFTBinSets[binSetIndex];
		currSet.averageBinExpressivityNormalized = currSet.averageBinExpressivity / m_maxBinExpressivityAverage;
	}
}


//-----------------------------------------------------------------------------------------------
// Writes the bin data history to a text file
//
void FFTSystem::WriteFFTBinDataToFile()
{
	CreateDirectoryA("Data/FFTLogs", NULL);

	std::vector<std::string> tokens = Tokenize(m_musicDataPath, '/');
	std::string songNameWithExt = tokens[tokens.size() - 1];

	std::string songName = std::string(songNameWithExt, 0, songNameWithExt.find_first_of('.'));
	std::string binDataFilePath = Stringf("Data/FFTLogs/%s.fftlog", songName.c_str());

	ConsolePrintf("Writing Bin Data to file %s...", binDataFilePath.c_str());
	File file;
	bool opened = file.Open(binDataFilePath.c_str(), "w+");

	ASSERT_OR_DIE(opened, "Error: Couldn't open FFT file for write");

	std::string buffer = Stringf("Song File: %s\n", m_musicDataPath.c_str()).c_str();

	buffer += Stringf("Duration: %.3f seconds\n", m_songLength).c_str();

	buffer += Stringf("Sample Rate: %.2fhz\n", m_sampleRate).c_str();

	buffer += Stringf("Max Frequency Analyzed: %.2f\n", m_maxFrequencyToSaveUpTo).c_str();

	buffer += Stringf("Number of Bins: %i\n", m_numBinsToSaveUpTo).c_str();

	buffer += Stringf("Number of Samples per bin: %i\n", m_FFTBinSets[0].fftBinSamples.size()).c_str();

	buffer += Stringf("Max Average Bin Expressivity: %.8f in Bin %i\n", m_maxBinExpressivityAverage, m_maxBinAverageBinIndex);
	
	buffer += Stringf("Min Average Bin Expressivity: %.8f in Bin %i\n", m_minBinExpressivityAverage, m_minBinAverageBinIndex);

	for (int spanIndex = 0; spanIndex < m_numBinsToSaveUpTo; ++spanIndex)
	{
		FFTBinSet_t& currentBinSpan = m_FFTBinSets[spanIndex];

		buffer += Stringf("BIN %i Frequency: %.2f %.2f (hertz)\n", spanIndex, currentBinSpan.frequencyInterval.min, currentBinSpan.frequencyInterval.max);
		buffer += Stringf("Average Expressivity: %.8f\nNormalized Expressivity: %.8f\n", currentBinSpan.averageBinExpressivity, currentBinSpan.averageBinExpressivityNormalized);

		int numFFTSamplesInSpan = (int)currentBinSpan.fftBinSamples.size();

		for (int sampleIndex = 0; sampleIndex < numFFTSamplesInSpan; ++sampleIndex)
		{
			buffer += Stringf("Time: %.4f - Value: %.8f\n", currentBinSpan.fftBinSamples[sampleIndex].timeIntoSong, currentBinSpan.fftBinSamples[sampleIndex].binAverageOfAllChannels);
		}
	}

	file.Write(buffer.data(), buffer.size());
	file.Close();

	ConsolePrintf(Rgba::GREEN, "Write Finished");
}


//-----------------------------------------------------------------------------------------------
// Deletes any FFT bin data leftover from the last data collection
//
void FFTSystem::CleanUp()
{
	m_FFTBinSets.clear();
	m_musicDataPath.clear();

	if (m_lastFFTSampleChannelAverages)
	{
		free(m_lastFFTSampleChannelAverages);
	}

	m_lastFFTSampleChannelAverages = nullptr;
	m_maxValueLastFrame = 0.f;
	m_sampleRate = -1.f;

	m_musicChannel = nullptr;
	m_songLength = 0.f;
	m_numBinsToSaveUpTo = -1;

	delete m_fftBarMesh;
	m_fftBarMesh = new Mesh();

	delete m_fftGridMesh;
	m_fftGridMesh = new Mesh();
	UpdateGridAndPanelMesh();

	m_systemState = STATE_IDLE;
	m_beatDisplayBinOffset = 0;
}


//-----------------------------------------------------------------------------------------------
// Initializes the FFT Bin data from the file given
//
void FFTSystem::SetupForFFTBeatAnalysis(File* file)
{
	std::string currLine;

	// Song name
	file->GetNextLine(currLine);
	m_musicDataPath = Tokenize(currLine, ' ')[2];

	// Duration
	file->GetNextLine(currLine);
	m_songLength = StringToFloat(Tokenize(currLine, ' ')[1]);

	// Sample Rate
	file->GetNextLine(currLine);
	m_sampleRate = StringToFloat(Tokenize(currLine, ' ')[2]);

	// Max Frequency Analyzed
	file->GetNextLine(currLine);

	// Bin Count
	file->GetNextLine(currLine);
	
	// Samples per bin
	file->GetNextLine(currLine);
	
	// Max Expressivity
	file->GetNextLine(currLine);
	std::vector<std::string> maxTokens = Tokenize(currLine, ' ');
	m_maxBinExpressivityAverage = StringToFloat(maxTokens[4]);
	m_maxBinAverageBinIndex = StringToInt(maxTokens[7]);

	// Min Expressivity
	file->GetNextLine(currLine);
	std::vector<std::string> minTokens = Tokenize(currLine, ' ');
	m_minBinExpressivityAverage = StringToFloat(minTokens[4]);
	m_minBinAverageBinIndex = StringToInt(minTokens[7]);

	// Bin information
	file->GetNextLine(currLine);
	while (!IsStringNullOrEmpty(currLine))
	{
		std::vector<std::string> lineTokens = Tokenize(currLine, ' ');
		
		// Start of a new bin
		if (lineTokens[0] == "BIN")
		{
			FFTBinSet_t span;
			span.frequencyInterval.min = StringToFloat(lineTokens[3]);
			span.frequencyInterval.max = StringToFloat(lineTokens[4]);

			// Get expressivities
			file->GetNextLine(currLine);
			std::vector<std::string> binAverageLine = Tokenize(currLine, ' ');
			span.averageBinExpressivity = StringToFloat(binAverageLine[2]);

			file->GetNextLine(currLine);
			std::vector<std::string> binAverageLineNormalized = Tokenize(currLine, ' ');
			span.averageBinExpressivityNormalized = StringToFloat(binAverageLineNormalized[2]);

			m_FFTBinSets.push_back(span);
		}
		else
		{
			// Is a bin sample line
			FFTBin_t data;
			data.timeIntoSong = StringToFloat(lineTokens[1]);
			data.binAverageOfAllChannels = StringToFloat(lineTokens[4]);

			m_FFTBinSets.back().fftBinSamples.push_back(data);
		}

		file->GetNextLine(currLine);
	}
}

// Example
// Song File: Data / Audio / Music / 120bpm.mp3
// Duration: 7.776 seconds
// Sample Rate: 48000.00 hz
// Number of Bins: 128
// Number of Samples per bin: 284
// Max Average Bin Expressivity: 0.00214435 in Bin 23
// Min Average Bin Expressivity: 0.00000469 in Bin 123
// BIN 0 | Frequency Range: [0.00 - 46.88 ) hertz
// Period Median: 0.5089 seconds
// Period Median Threshold: 10.00 %
// Periods Within Median Threshold: 10
// Total Periods: 12
// Period Confidence for this bin: 83.33 %
// Phase Median: 0.2863 seconds
// Phase Median Threshold: 10.00 %
// Phases Within Threshold: 8
// Total Phases: 12
// Phase Confidence for this Bin: 66.67 %
// Bin Average Expressivity: 0.00008917
// Bin Average Expressivity Normalized: 0.04158454
// HIGH SAMPLES
// Time: 1.3666 - Value: 0.00020867
// Time: 1.8565 - Value: 0.00027561
// Time: 2.3466 - Value: 0.00031129
// Time: 2.8555 - Value: 0.00023990
// Time: 3.3463 - Value: 0.00030161
// Time: 3.8562 - Value: 0.00021270
// Time: 4.3472 - Value: 0.00028484
// Time: 4.8664 - Value: 0.00019678
// Time: 5.3568 - Value: 0.00025126
// Time: 5.8459 - Value: 0.00031366
// Time: 6.3765 - Value: 0.00016566
// Time: 6.8271 - Value: 0.00100535
// Time: 7.3366 - Value: 0.00031105

//-----------------------------------------------------------------------------------------------
// Writes the beat analysis data to a beat data file
//
void FFTSystem::WriteFFTBeatAnalysisToFile()
{
	CreateDirectoryA("Data/FFTBeatAnalyses", NULL);

	std::vector<std::string> tokens = Tokenize(m_musicDataPath, '/');
	std::string songNameWithExt = tokens[tokens.size() - 1];

	std::string songName = std::string(songNameWithExt, 0, songNameWithExt.find_first_of('.'));
	std::string filePath = Stringf("Data/FFTBeatAnalyses/%s.fftbeat", songName.c_str());
	File file;
	bool success = file.Open(filePath.c_str(), "w+");

	if (!success)
	{
		ConsoleErrorf("Couldn't open beat data file %s for write", filePath.c_str());
		return;
	}

	// Song title
	std::string buffer = Stringf("Song File: %s\n", m_musicDataPath.c_str()).c_str();

	// Duration
	buffer += Stringf("Duration: %.3f seconds\n", m_songLength).c_str();

	// Sample Rate
	buffer += Stringf("Sample Rate: %.2f hz\n", m_sampleRate).c_str();

	// Bin Count
	int binCount = (int) m_FFTBinSets.size();
	buffer += Stringf("Number of Bins: %i\n", binCount).c_str();

	// Sample Count
	int numSamples = (int) m_FFTBinSets[0].fftBinSamples.size();
	buffer += Stringf("Number of Samples per bin: %i\n", numSamples).c_str();

	// Max Expressivity
	buffer += Stringf("Max Average Bin Expressivity: %.8f in Bin %i\n", m_maxBinExpressivityAverage, m_maxBinAverageBinIndex);

	// Min Expressivity
	buffer += Stringf("Min Average Bin Expressivity: %.8f in Bin %i\n", m_minBinExpressivityAverage, m_minBinAverageBinIndex);

	// For each bin
	for (int spanIndex = 0; spanIndex < binCount; ++spanIndex)
	{
		FFTBinSet_t& currentBinSpan = m_FFTBinSets[spanIndex];
		numSamples = (int)currentBinSpan.fftBinSamples.size();

		// Bin Header
		buffer += Stringf("BIN %i | Frequency Range: [ %.2f - %.2f ) hertz\n", spanIndex, currentBinSpan.frequencyInterval.min, currentBinSpan.frequencyInterval.max);

		// Period Median
		buffer += Stringf("Period Median: %.4f seconds\n", currentBinSpan.periodMedian);

		// Period Median Threshold
		buffer += Stringf("Period Median Threshold: %.2f %%\n", currentBinSpan.periodMedianThreshold * 100.f);

		// Periods Within Threshold
		buffer += Stringf("Periods Within Median Threshold: %i\n", currentBinSpan.periodsWithinThreshold);

		// Total Periods
		buffer += Stringf("Total Periods: %i\n", currentBinSpan.totalPeriods);

		// % Confidence in Period
		buffer += Stringf("Period Confidence for this bin: %.2f %%\n", currentBinSpan.periodConfidence * 100.f);

		// Phase Median
		buffer += Stringf("Phase Median: %.4f seconds\n", currentBinSpan.phaseMedian);

		// Phase Median Threshold
		buffer += Stringf("Phase Median Threshold: %.2f %%\n", currentBinSpan.phaseMedianThreshold * 100.f);

		// Phases Within Threshold
		buffer += Stringf("Phases Within Threshold: %i\n", currentBinSpan.phasesWithinThreshold);

		// Total Phases
		buffer += Stringf("Total Phases: %i\n", currentBinSpan.totalPhases);

		// Phase confidence
		buffer += Stringf("Phase Confidence for this Bin: %.2f %%\n", currentBinSpan.phaseConfidence * 100.f);
		
		// Expressivities
		buffer += Stringf("Bin Average Expressivity: %.8f\n", currentBinSpan.averageBinExpressivity);
		buffer += Stringf("Bin Average Expressivity Normalized: %.8f\n", currentBinSpan.averageBinExpressivityNormalized);

		// All the interval data
		buffer += Stringf("HIGH SAMPLES\n");

		for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
		{
			if (currentBinSpan.fftBinSamples[sampleIndex].isHigh)
			{
				buffer += Stringf("Time: %.4f - Value: %.8f\n", currentBinSpan.fftBinSamples[sampleIndex].timeIntoSong, currentBinSpan.fftBinSamples[sampleIndex].binAverageOfAllChannels);
			}
		}
	}

	file.Write(buffer.c_str(), buffer.size());
	file.Close();
}


// Example
// Song File: Data / Audio / Music / 120bpm.mp3
// Duration: 7.776 seconds
// Sample Rate: 48000.00 hz
// Number of Bins: 128
// Number of Samples per bin: 284
// Max Average Bin Expressivity: 0.00214435 in Bin 23
// Min Average Bin Expressivity: 0.00000469 in Bin 123
// BIN 0 | Frequency Range: [0.00 - 46.88 ) hertz
// Period Median: 0.5089 seconds
// Period Median Threshold: 10.00 %
// Periods Within Median Threshold: 10
// Total Periods: 12
// Period Confidence for this bin: 83.33 %
// Phase Median: 0.2863 seconds
// Phase Median Threshold: 10.00 %
// Phases Within Threshold: 8
// Total Phases: 12
// Phase Confidence for this Bin: 66.67 %
// Bin Average Expressivity: 0.00008917
// Bin Average Expressivity Normalized: 0.04158454
// HIGH SAMPLES
// Time: 1.3666 - Value: 0.00020867
// Time: 1.8565 - Value: 0.00027561
// Time: 2.3466 - Value: 0.00031129
// Time: 2.8555 - Value: 0.00023990
// Time: 3.3463 - Value: 0.00030161
// Time: 3.8562 - Value: 0.00021270
// Time: 4.3472 - Value: 0.00028484
// Time: 4.8664 - Value: 0.00019678
// Time: 5.3568 - Value: 0.00025126
// Time: 5.8459 - Value: 0.00031366
// Time: 6.3765 - Value: 0.00016566
// Time: 6.8271 - Value: 0.00100535
// Time: 7.3366 - Value: 0.00031105

//-----------------------------------------------------------------------------------------------
// Loads the FFT
//
void FFTSystem::LoadFFTBeatAnalysis(const std::string& filePath)
{
	File file;
	bool fileOpened = file.Open(filePath.c_str(), "r");

	if (!fileOpened)
	{
		ConsoleErrorf("Couldn't open file %s", filePath.c_str());
		return;
	}

	file.LoadFileToMemory();

	std::string currLine;
	file.GetNextLine(currLine);
	m_musicDataPath = Tokenize(currLine, ' ')[2];

	file.GetNextLine(currLine);
	m_songLength = StringToFloat(Tokenize(currLine, ' ')[1]);

	file.GetNextLine(currLine);
	m_sampleRate = StringToFloat(Tokenize(currLine, ' ')[2]);

	// Bin count
	file.GetNextLine(currLine);
	int binCount = StringToInt(Tokenize(currLine, ' ')[3]);
	m_FFTBinSets.resize(binCount);

	// Samples per bin
	file.GetNextLine(currLine);
	int sampleCount = StringToInt(Tokenize(currLine, ' ')[5]);
	for (int binIndex = 0; binIndex < binCount; ++binIndex)
	{
		m_FFTBinSets[binIndex].fftBinSamples.reserve(sampleCount);
	}

	file.GetNextLine(currLine);
	std::vector<std::string> maxTokens = Tokenize(currLine, ' ');
	m_maxBinExpressivityAverage = StringToFloat(maxTokens[4]);
	m_maxBinAverageBinIndex = StringToInt(maxTokens[7]);

	file.GetNextLine(currLine);
	std::vector<std::string> minTokens = Tokenize(currLine, ' ');
	m_minBinExpressivityAverage = StringToFloat(minTokens[4]);
	m_minBinAverageBinIndex = StringToInt(minTokens[7]);

	// Bin data
	file.GetNextLine(currLine);
	int runningBinIndex = 0;
	while (!IsStringNullOrEmpty(currLine))
	{
		std::vector<std::string> tokens = Tokenize(currLine, ' ');

		if (tokens[0] == "BIN")
		{
			runningBinIndex = StringToInt(tokens[1]);
			FFTBinSet_t& binSet = m_FFTBinSets[runningBinIndex];

			float minFreq = StringToFloat(tokens[6]);
			float maxFreq = StringToFloat(tokens[8]);

			binSet.frequencyInterval = FloatRange(minFreq, maxFreq);

			file.GetNextLine(currLine);
			binSet.periodMedian = StringToFloat(Tokenize(currLine, ' ')[2]);

			file.GetNextLine(currLine);
			binSet.periodMedianThreshold = StringToFloat(Tokenize(currLine, ' ')[3]) * 0.01f;

			file.GetNextLine(currLine);
			binSet.periodsWithinThreshold = StringToInt(Tokenize(currLine, ' ')[4]);

			file.GetNextLine(currLine);
			binSet.totalPeriods = StringToInt(Tokenize(currLine, ' ')[2]);

			file.GetNextLine(currLine);
			binSet.periodConfidence = StringToFloat(Tokenize(currLine, ' ')[5]) * 0.01f;

			file.GetNextLine(currLine);
			binSet.phaseMedian = StringToFloat(Tokenize(currLine, ' ')[2]);

			file.GetNextLine(currLine);
			binSet.phaseMedianThreshold = StringToFloat(Tokenize(currLine, ' ')[3]) * 0.01f;

			file.GetNextLine(currLine);
			binSet.phasesWithinThreshold = StringToInt(Tokenize(currLine, ' ')[3]);

			file.GetNextLine(currLine);
			binSet.totalPhases = StringToInt(Tokenize(currLine, ' ')[2]);

			file.GetNextLine(currLine);
			binSet.phaseConfidence = StringToFloat(Tokenize(currLine, ' ')[5]) * 0.01f;

			file.GetNextLine(currLine);
			binSet.averageBinExpressivity = StringToFloat(Tokenize(currLine, ' ')[3]);
			
			file.GetNextLine(currLine);
			binSet.averageBinExpressivityNormalized = StringToFloat(Tokenize(currLine, ' ')[4]);

			file.GetNextLine(currLine);
		}
		else if (tokens[0] == "HIGH")
		{
			std::string highLine;
			file.GetNextLine(highLine);
			std::vector<std::string> highTokens = Tokenize(highLine, ' ');

			while (highTokens[0] == "Time:")
			{
				FFTBin_t bin;
				bin.timeIntoSong = StringToFloat(highTokens[1]);
				bin.binAverageOfAllChannels = StringToFloat(highTokens[4]);
				bin.isHigh = true;

				FFTBinSet_t& binSet = m_FFTBinSets[runningBinIndex];
				binSet.fftBinSamples.push_back(bin);

				file.GetNextLine(highLine);

				if (!IsStringNullOrEmpty(highLine))
				{
					highTokens = Tokenize(highLine, ' ');
				}
				else
				{
					break;
				}
			}

			currLine = highLine;
		}
		else
		{
			ERROR_AND_DIE("Bad token");
		}
	}

	file.Close();
}


//-----------------------------------------------------------------------------------------------
// Updates the mesh to represent the bar graph
//
void FFTSystem::UpdateBarMesh()
{
	if (m_pointerToFMODFFTSpectrum != nullptr)
	{
		float boxWidth = m_graphBounds.GetDimensions().x / (float)m_binsToDisplay;
		AABB2 baseBoxBounds = AABB2(m_graphBounds.mins, m_graphBounds.mins + Vector2(boxWidth, m_graphBounds.GetDimensions().y));

		MeshBuilder mb;
		mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);

		float oneOverMaxYValue = 1.0f / m_fftMaxYAxis;
		m_maxValueLastFrame = 0.f;

		for (unsigned int i = 0; i < m_binsToDisplay; ++i)
		{
			// Get the sum of all channels
			float value = m_lastFFTSampleChannelAverages[i];
			
			m_maxValueLastFrame = MaxFloat(value, m_maxValueLastFrame);

			AABB2 currBoxBounds = baseBoxBounds;
			currBoxBounds.maxs.y = oneOverMaxYValue * value * baseBoxBounds.GetDimensions().y + baseBoxBounds.mins.y;

			// Texture coords
			AABB2 texCoords;
			texCoords.mins = Vector2::ZERO;
			texCoords.maxs = Vector2(1.0f, value);

			mb.Push2DQuad(currBoxBounds, texCoords);

			baseBoxBounds.Translate(Vector2(baseBoxBounds.GetDimensions().x, 0.f));
		}

		mb.FinishBuilding();
		mb.UpdateMesh(*m_fftBarMesh);
	}
}


//-----------------------------------------------------------------------------------------------
// Rebuilds the mesh used for rendering the grid and panels
//
void FFTSystem::UpdateGridAndPanelMesh()
{
	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);

	// Push in background first to avoid overdrawing
	mb.Push2DQuad(m_totalBounds, AABB2::UNIT_SQUARE_OFFCENTER, m_backgroundColor);

	// Push a grid
	int baseThickness = 1;

	// Vertical lines for X axis
	float width = m_graphBounds.GetDimensions().x / (float)m_gridSegmentCount.x;
	for (int i = 0; i <= m_gridSegmentCount.x; ++i)
	{
		int lineThickness = baseThickness;
		if (i % 2 == 0)
		{
			lineThickness *= 2;
		}

		float x = m_graphBounds.mins.x + i * width;

		Vector2 min = Vector2(x - lineThickness, m_graphBounds.mins.y);
		Vector2 max = Vector2(x + lineThickness, m_graphBounds.maxs.y);

		AABB2 line = AABB2(min, max);

		mb.Push2DQuad(line, AABB2::UNIT_SQUARE_OFFCENTER, m_lineAndPanelColor);
	}

	// Horizontal lines for Y axis
	float height = m_graphBounds.GetDimensions().y / (float)(m_gridSegmentCount.y);
	for (int i = 0; i <= m_gridSegmentCount.y; ++i)
	{
		float thickness = 1;
		if (i % 2 == 0)
		{
			thickness *= 2;
		}

		float y = m_graphBounds.mins.y + i * height;

		Vector2 min = Vector2(m_graphBounds.mins.x, y - thickness);
		Vector2 max = Vector2(m_graphBounds.maxs.x, y + thickness);

		AABB2 line = AABB2(min, max);

		mb.Push2DQuad(line, AABB2::UNIT_SQUARE_OFFCENTER, m_lineAndPanelColor);
	}

	// Line for max value
	float oneOverMaxYValue = 1.0f / m_fftMaxYAxis;

	AABB2 maxValueLine;
	maxValueLine.mins = Vector2(m_graphBounds.mins.x, m_graphBounds.GetDimensions().y * m_maxValueLastFrame * oneOverMaxYValue + m_graphBounds.mins.y);
	maxValueLine.maxs = Vector2(m_graphBounds.maxs.x, m_graphBounds.GetDimensions().y * m_maxValueLastFrame * oneOverMaxYValue + m_graphBounds.mins.y);
	maxValueLine.AddPaddingToSides(0.f, (float)baseThickness);

	mb.Push2DQuad(maxValueLine, AABB2::UNIT_SQUARE_OFFCENTER, Rgba(255, 255, 0, 100));

	// Push the background panels
	mb.Push2DQuad(m_headingBounds, AABB2::UNIT_SQUARE_OFFCENTER, m_lineAndPanelColor);
	mb.Push2DQuad(m_yAxisBounds, AABB2::UNIT_SQUARE_OFFCENTER, m_lineAndPanelColor);
	mb.Push2DQuad(m_xAxisBounds, AABB2::UNIT_SQUARE_OFFCENTER, m_lineAndPanelColor);
	mb.Push2DQuad(m_rightSidePanel, AABB2::UNIT_SQUARE_OFFCENTER, m_lineAndPanelColor);

	mb.FinishBuilding();
	mb.UpdateMesh(*m_fftGridMesh);
}
