/************************************************************************/
/* File: CountJob.hpp
/* Author: Andrew Chase
/* Date: May 7th 2019
/* Description: Class for counting to 1,000,000 using the job system
/************************************************************************/
#pragma once
#include "Engine/Core/JobSystem/Job.hpp"

class CountJob : public Job
{
public:
	//-----Public Methods-----

	CountJob();
	virtual ~CountJob();

	virtual void Execute() override;
	virtual void Finalize() override;


private:
	//-----Private Data-----

	int m_jobNumber;
	int m_count = 0;

	static int s_nextJobNumber;
};
