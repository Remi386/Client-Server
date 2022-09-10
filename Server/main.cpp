#include "../NetCommon/NetCommon.h"
#include <iostream>
#include "Server.h"
#include "DataBase.h"
#include "Marketplace.h"

using boost::asio::ip::tcp;

int main()
{
    try
    {
        DataBase database("testtaskdatabase");
        Marketplace market(database);

        boost::asio::io_context context;

        Server s(context, market, database);

        context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}