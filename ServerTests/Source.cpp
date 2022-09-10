#include <iostream>
#include "TestEnvironment.h"
#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
	//Add global test enviroment to create database only once
	TestEnvironment* env = new TestEnvironment();
	testing::AddGlobalTestEnvironment(env);

	std::cout << "Running server tests: " << std::endl;
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}