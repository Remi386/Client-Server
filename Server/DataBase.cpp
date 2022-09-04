#include "DataBase.h"

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