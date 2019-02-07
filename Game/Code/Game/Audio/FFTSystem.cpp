/************************************************************************/
/* File: FFTSystem.cpp
/* Author: Andrew Chase
/* Date: February 6th 2019
/* Description: Implementation of the FFTSystem class
/************************************************************************/
#include "Game/Audio/FFTSystem.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/Resources/BitmapFont.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor
//
FFTSystem::FFTSystem()
{
	CreateAndAddFFTDSPToMasterChannel();
	SetupFFTGraphUI();
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
FFTSystem::~FFTSystem()
{
	// Shouldn't have to delete these, as they're just convenience pointers
	m_fftDSP = nullptr;
	m_spectrumData = nullptr;
}


//-----------------------------------------------------------------------------------------------
// "Update"
//
void FFTSystem::BeginFrame()
{
	AudioSystem::BeginFrame();

	// Analyze the FFT data for this frame
	UpdateFFTData();

	// Update bar mesh if rendering
	if (m_renderFFTGraph)
	{
		UpdateBarMesh();
		UpdateGridAndPanelMesh();
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
		windowType--;
	}

	if (input->WasKeyJustPressed(InputSystem::KEYBOARD_RIGHT_ARROW))
	{
		windowType++;
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
	Renderer* renderer = Renderer::GetInstance();
	renderer->SetCurrentCamera(renderer->GetUICamera());
	AABB2 bounds = renderer->GetUIBounds();
	BitmapFont* font = AssetDB::GetBitmapFont("Data/Images/Fonts/ConsoleFont.png");

	renderer->DrawMeshWithMaterial(&m_gridMesh, AssetDB::GetSharedMaterial("UI"));
	renderer->DrawMeshWithMaterial(&m_barMesh, AssetDB::GetSharedMaterial("Gradient"));

	std::string text = Stringf("Number of Channels: %i\n", m_spectrumData->numchannels);
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
// Plays the music track on the system for FFT analysis
//
void FFTSystem::PlayMusicTrackForFFT(SoundID soundID, float volume /*= 1.f*/)
{
	if (m_musicChannel != nullptr)
	{
		AudioSystem::StopSound((SoundPlaybackID)m_musicChannel);
	}

	m_musicChannel = (FMOD::Channel*) AudioSystem::PlaySound(soundID, true, volume);

	// Update sample rate
	m_musicChannel->getFrequency(&m_sampleRate);
}


//-----------------------------------------------------------------------------------------------
// Sets the max frequency value that is shown on the graph when rendererd (X-axis)
//
void FFTSystem::SetFFTGraphMaxXValue(float maxXValue)
{
	float frequencyPerSegment = m_nyquistFreq / (float)(m_fftWindowSize / 2);
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
// Sets the FFT window size
//
void FFTSystem::SetFFTWindowSize(int windowSize)
{
	m_fftWindowSize = windowSize;

	FMOD_RESULT result = m_fftDSP->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, m_fftWindowSize);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't assign window size parameter");
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
// Sets whether the FFT Graph should render to the screen
//
void FFTSystem::SetShouldRenderFFTGraph(bool shouldRender)
{
	m_renderFFTGraph = shouldRender;
}


//-----------------------------------------------------------------------------------------------
// Returns whether the system is set to render the graph
//
bool FFTSystem::IsSetToRenderGraph()
{
	return m_renderFFTGraph;
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
	SetFFTWindowSize(m_fftWindowSize);

	result = masterChannelGroup->addDSP(FMOD_CHANNELCONTROL_DSP_HEAD, m_fftDSP);
	ASSERT_OR_DIE(result == FMOD_OK, "Couldn't ADD the DSP to the master channel group");
}


//-----------------------------------------------------------------------------------------------
// Grabs the last frame's FFT data and parses it
//
void FFTSystem::UpdateFFTData()
{
	FMOD::ChannelGroup* masterChannelGroup = nullptr;
	m_fmodSystem->getMasterChannelGroup(&masterChannelGroup);

	// Get the fft data
	void* spectrumData = nullptr;
	m_fftDSP->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void**)&spectrumData, 0, 0, 0);
	m_spectrumData = (FMOD_DSP_PARAMETER_FFT*)spectrumData;
}


//-----------------------------------------------------------------------------------------------
// Updates the mesh to represent the bar graph
//
void FFTSystem::UpdateBarMesh()
{
	if (m_spectrumData != nullptr)
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
			float value = 0.f;
			for (int j = 0; j < m_spectrumData->numchannels; ++j)
			{
				value += m_spectrumData->spectrum[j][i];
			}

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
		mb.UpdateMesh(m_barMesh);
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
	mb.UpdateMesh(m_gridMesh);
}
