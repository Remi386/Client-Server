#pragma once
#include <gtest/gtest.h>
#include "../Server/CompletedTradeRequest.h"

class CompletedTradeRequestTest : public testing::Test {
protected:

	virtual void SetUp() override
	{
		timeOfCreation = boost::posix_time::second_clock::universal_time();
	}

	virtual void TearDown() override
	{

	}

	boost::posix_time::ptime timeOfCreation;
};