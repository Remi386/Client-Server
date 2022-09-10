#include "DataBase.h"
#include <pqxx/pqxx>

//"user=postgres password=postgre host=localhost port=5432 dbname=template1 "
DataBase::DataBase(const std::string& dbname)
{	
	bool connected = false;
	bool createDB = false;

	std::string databasename = dbname;
	std::string user = "";
	std::string password = "";

	while (!connected) {

		std::string options = "host=localhost "
							  "port=5432 "
							  "dbname=" + databasename +
							  " connect_timeout=10 " +
							  user + " "
							  + password;

		try {

			connection = std::make_unique<pqxx::connection>(options);

			connected = true;
			std::cout << "Connected!" << std::endl;

			if (createDB) {
				createDatabase(dbname, user, password);
			}

		}
		catch (const pqxx::broken_connection& connErr) {

			std::string errorMessage = connErr.what();

			if (errorMessage.find("no password supplied") != std::string::npos) {
				std::cout << "No password to PostgreSQL provided!\n";
				std::cout << "Please, enter password for PostgreSQL: ";
				std::cin >> password;
				password = "password=" + password;
			}
			else if (errorMessage.find("authentication failed for user") != std::string::npos) {
				std::cout << "Authentication failed!\n";

				std::string input;
				do {
					std::cout << "Do you want to enter password "
						"again or switch user to postgres?\n"
						"(type 'again' or 'switch'): ";

					std::cin >> input;
				} while (input != "again" && input != "switch");

				if (input == "again") {
					std::cout << "Please, enter password for your user in PostgreSQL: ";
					std::cin >> password;
					password = "password=" + password;

				}
				else {
					std::cout << "Please, enter password for user postgres (or leave it empty): ";
					std::cin >> password;
					
					if(!password.empty())
						password = "password=" + password;

					user = "user=postgres";
				}
			}
			else if (errorMessage.find("database \"" + dbname + "\" does not exist") != std::string::npos)
			{
				std::cout << "Database " << dbname << " not found, switching to template1" << std::endl;
				databasename = "template1";
				createDB = true;
			}
		}
		catch (const std::exception& e) {
			std::cout << "Exception: " << e.what() << std::endl;
		}
	} //while(!connected)
}

void DataBase::createDatabase(const std::string& dbname,
	const std::string& user,
	const std::string& password)
{
	try {

		{ //Create New DataBase
			pqxx::nontransaction query(*connection);
			auto reselt = query.exec("CREATE DATABASE " + dbname);
			query.exec("GRANT ALL ON DATABASE " + dbname + " TO PUBLIC");
		}

		connection->close();

		std::string options = "host=localhost "
			"port=5432 "
			"dbname=" + dbname +
			" connect_timeout=10 " +
			user + " "
			+ password;

		//Connect to it
		connection = std::make_unique<pqxx::connection>(options);

		//Create tables
		pqxx::nontransaction query(*connection);

		query.exec("CREATE TABLE Users("
			"UserID SERIAL PRIMARY KEY,"
			"Login varchar(80) NOT NULL,"
			"Password varchar(64) NOT NULL,"
			"Dollars INTEGER NOT NULL DEFAULT 0,"
			"Rubles INTEGER NOT NULL DEFAULT 0"
			")");

		query.exec("CREATE TABLE History("
			"HistoryId SERIAL PRIMARY KEY,"
			"OwnerID INTEGER references Users(UserID),"
			"PartnerID INTEGER references Users(UserID),"
			"Dollars INTEGER NOT NULL,"
			"Rubles INTEGER NOT NULL,"
			"RegTime varchar(20) NOT NULL,"
			"CompTime varchar(20) NOT NULL,"
			"Type varchar(4) NOT NULL)"
		);
	}
	catch (pqxx::sql_error& e) {
		std::string sqlErrCode = e.sqlstate();
		if (sqlErrCode == "42P04") { // catch duplicate_database
			std::cout << "Database: " << dbname << " exists, proceeding further\n";
			return;
		}
		throw;
	}
	catch (const std::exception& e) {
		std::cout << "Exepction: " << e.what() << std::endl;
		throw;
	}
}

int64_t DataBase::registerNewUser(const std::string& login, const std::string& password)
{

	pqxx::work w(*connection);

	pqxx::result selectRes = w.exec("SELECT userid "
									"FROM users "
									"WHERE login = " + w.quote(login));

	if (selectRes.empty()) {

		pqxx::result insertRes = w.exec("INSERT INTO users (login, password) VALUES (" 
										+ w.quote(login) + ", " + w.quote(password) + ")"
										" RETURNING userid");

		w.commit();

		return insertRes[0][0].as<int64_t>();
	}

	return -1;
}

int64_t DataBase::getUserID(const std::string& login, const std::string& password) const
{
	pqxx::work w(*connection);

	pqxx::result selectRes = w.exec("SELECT userid, password "
									"FROM users "
									"WHERE login = " + w.quote(login));

	w.commit(); //not really needed for select queries

	if (selectRes.empty())
		return -1;

	auto [id, pass] = selectRes[0].as<int64_t, std::string>();

	if (pass != password)
		return -2;

	return id;
}

void DataBase::addCompletedTradeRequest(int64_t clientID, 
										const CompletedTradeRequest & request)
{
	pqxx::work w(*connection);

	w.exec("INSERT INTO history (ownerid, partnerid, dollars, "
								"rubles, regtime, comptime, type) "
		   "VALUES ("
		   + std::to_string(clientID) + ", "
		   + std::to_string(request.getOtherUserID()) + ", "
		   + std::to_string(request.getVolume()) + ", "
		   + std::to_string(request.getPrice()) + ", "
		   + w.quote(request.getRegistrationTimeString()) + ", "
		   + w.quote(request.getComletionTimeString()) + ", "
		   + w.quote(request.getTypeString()) + ")"
	);

	w.commit();
}

std::list<CompletedTradeRequest> DataBase::getClientTradeHistory(int64_t clientID)
{
	std::list<CompletedTradeRequest> hist;

	pqxx::work w(*connection);

	pqxx::result res = w.exec("SELECT partnerid, dollars, rubles, "
							  "regtime, comptime, type "
							  "FROM history "
							  "WHERE ownerid = " + std::to_string(clientID));

	w.commit();

	for (const auto& row : res) {
		auto [PartnerID, Dollars, Rubles, RegTime, CompTime, Type]
			= row.as<int64_t, int64_t, int64_t, std::string, std::string, std::string>();

		TradeRequestType tradeType = Type == "Buy" ? TradeRequestType::Buy 
												   : TradeRequestType::Sell;

		boost::posix_time::ptime registrationTime 
			= boost::posix_time::from_iso_extended_string(RegTime);

		boost::posix_time::ptime completionTime
			= boost::posix_time::from_iso_extended_string(CompTime);

		CompletedTradeRequest req(Dollars, Rubles, PartnerID, 
								  registrationTime, completionTime, tradeType);

		hist.push_back(std::move(req));
	}
	
	return hist;
}

std::optional<ClientInfo> DataBase::getClientInfo(int64_t clientID)
{
	pqxx::work w(*connection);

	pqxx::result selectRes = w.exec("SELECT dollars, rubles "
									"FROM users "
									"WHERE userid = " + std::to_string(clientID));

	w.commit();

	if (!selectRes.empty()) {

		auto [dollars, rubles] = selectRes[0].as<int64_t, int64_t>();

		return ClientInfo(dollars, rubles);
	}

	return std::nullopt;
}

void DataBase::updateClientInfo(int64_t clientID, const ClientInfo& newInfo)
{
	pqxx::work w(*connection);

	w.exec("UPDATE users "
		   "SET dollars = " + std::to_string(newInfo.getDollars()) +
		   ", rubles = " + std::to_string(newInfo.getRubles()) +
		   " WHERE userid = " + std::to_string(clientID));

	w.commit();
}

void DataBase::clearTables()
{
	pqxx::nontransaction query(*connection);

	query.exec("TRUNCATE users, history");
}