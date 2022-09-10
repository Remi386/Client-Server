#pragma once
#include <gtest/gtest.h>
#include "../Server/DataBase.h"

class TestEnvironment : public testing::Environment {

public:

	virtual ~TestEnvironment() = default;


	virtual void SetUp() override
	{
	}

	virtual void TearDown() override
	{
	}

	static DataBase* getDataBase() 
	{ 
		static DataBase database = DataBase("databasefortests");

		return &database; 
	}

};