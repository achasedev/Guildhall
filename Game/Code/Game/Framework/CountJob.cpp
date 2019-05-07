/************************************************************************/
/* File: CountJob.cpp
/* Author: Andrew Chase
/* Date: May 7th 2019
/* Description: Implementation of the CountJob class
/************************************************************************/
#include "Game/Framework/Game.hpp"
#include "Game/Framework/CountJob.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"

int CountJob::s_nextJobNumber = 0;


//-----------------------------------------------------------------------------------------------
// Constructor
//
CountJob::CountJob()
{
	m_jobType = 5;
	m_jobFlags = 0;
	m_jobNumber = s_nextJobNumber++;

	Game::GetInstance()->m_totalCreatedJobs++;
}


//-----------------------------------------------------------------------------------------------
// Descructor
//
CountJob::~CountJob()
{

}


//-----------------------------------------------------------------------------------------------
// Counts up to 1,000,000
//
void CountJob::Execute()
{
	for (int i = 0; i < 1000000; ++i)
	{
		m_count++;
	}
}


//-----------------------------------------------------------------------------------------------
// Prints the result to dev console
//
void CountJob::Finalize()
{
	Game::GetInstance()->m_numJobsFinished++;
	ConsolePrintf(Rgba::GetRandomColor(), "Job %i finished counting to 1,000,000", m_jobNumber);
}
