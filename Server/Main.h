#pragma once
#include "Marketplace.h"
#include "DataBase.h"

class Main 
{
public:
	Main()
		: database(DataBase()), 
		market(database)
	{}

private:
	DataBase database;
	Marketplace market;
};