#include "DataBase.h"

/*
pqxx::connection conn;// { "user=postgres password=postgre host=localhost port=5432 dbname=template1 connect_timeout=10" };
		pqxx::work w(conn);
		pqxx::nontransaction quary(conn);

		quary.exec("CREATE DATABASE Test");

		quary.exec("CREATE TABLE Users("
			"UserId SERIAL PRIMARY KEY,"
			"Login varchar(80) NOT NULL,"
			"Password varchar(40) NOT NULL"
			"Dollars int NOT NULL"
			"Rubles int NOT NULL"
			")");

		quary.commit();

		quary.exec("CREATE TABLE History("
			"HistoryId SERIAL PRIMARY KEY,"
			"UserID references Users(UserId)"
			"Dollars int NOT NULL"
			"Rubles int NOT NULL"
			"OtherUserId references Users(UserId)"
			"RegTime varchar(20) NOT NULL"
			"CompTime varchar(20) NOT NULL"
			")");
*/

int64_t DataBase::registerNewUser(const std::string& login, const std::string& password)
{
	static int64_t counter = 0;

	if (login2ID.find(login) == login2ID.end())
	{
		clientsInfo.emplace(counter, ClientInfo());
		login2ID.emplace(login, counter);
		ID2Password.emplace(counter, password);

		return counter++;
	}

	return -1;
}

int64_t DataBase::getUserID(const std::string& login, const std::string& password) const
{
	auto iter = login2ID.find(login);

	if (iter != login2ID.end()) {
		if (ID2Password.at(iter->second) == password) {
			return iter->second;
		}

		return -2;
	}

	return -1;
}