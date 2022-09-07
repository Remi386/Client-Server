#pragma once
#include "../Server/DataBase.h"
#include <gtest/gtest.h>

class DataBaseTest : public testing::Test
{
protected:

	virtual void SetUp() override
	{
		//Clear all of the information between tests
		database = new DataBase();
	}

	virtual void TearDown() override
	{
		delete database;
	}

	DataBase* database;
};
