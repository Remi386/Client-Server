#pragma once
#include "TestEnvironment.h"
#include <gtest/gtest.h>

class DataBaseTest : public testing::Test
{
protected:

	virtual void SetUp() override
	{
		//Clear all of the information between tests
		database = TestEnvironment::getDataBase();
	}

	virtual void TearDown() override
	{
		database->clearTables();
	}

	DataBase* database;
};
