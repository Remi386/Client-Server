# TestTaskClientServer
To Build this project you will need following libraries:

1. Boost (system, regex, date_time libs)

2. OpenSSL

3. PostgreSQL (libpqxx)

4. Json for modern C++ (nlohmann::json)

5. Google Test (automaticly configured)

And, of course, CMake
## Configure libraries ob Debian-like distributives

First of all, update package list:

```
    sudo apt-get update
```

### Configure Boost
Run this command on terminal to download  and build packages:
```
    sudo apt-get install libboost-all-dev
```

### Configure OpenSSL
This command will be enough
```
    sudo apt-get install libssl-dev
```

### Configure PostgreSQL
For this programm you will need *[PostgreSQL Server](https://www.postgresql.org/download/linux/)

Short version:

Creating the file repository configuration:
```
    $ sudo sh -c 'echo "deb http://apt.postgresql.org/pub/repos/apt $(lsb_release -cs)-pgdg main" > /etc/apt/sources.list.d/pgdg.list'

```

Importing the repository signing key:

```
    $ wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | sudo apt-key add -
```

Install latest version:

```
    sudo apt-get install postgresql
```

The creation of necessary databases and tables are hardcoded, the only thing you will need to
do is walk through a little dialog if something went wrong (e.g. your role doesnt exist or authentication failed). 

However, you can create database manually:

In psql (if role doesnt exist change user to postgres with 'su -u postgres') type:

```
    CREATE DATABASE testtaskdatabase
```

```
    \c testtaskdatabase
```

```
    CREATE TABLE Users(
	    UserID SERIAL PRIMARY KEY,
	    Login varchar(80) NOT NULL,
	    Password varchar(64) NOT NULL,
	    Dollars INTEGER NOT NULL DEFAULT 0,
	    Rubles INTEGER NOT NULL DEFAULT 0
	    )

	CREATE TABLE History(
	    HistoryId SERIAL PRIMARY KEY,
	    OwnerID INTEGER references Users(UserID),
	    PartnerID INTEGER references Users(UserID),
	    Dollars INTEGER NOT NULL,
	    Rubles INTEGER NOT NULL,
	    RegTime varchar(20) NOT NULL,
	    CompTime varchar(20) NOT NULL,
	    Type varchar(4) NOT NULL
        )
```

Repeat actions with database databasefortests;

If you dont like names, you can change it in code (files Server/main.cpp, ServerTests/TestEnvironment.h) MUST BE LOWERCASE;

In addition, you may also want to add following lines on top of the other records in file 
/etc/postgresql/<version>/main/pg_hba.conf

```
    local testtaskdatabase all      trust

    local databasefortests all      trust
```
This allow you not to enter password every time

After changes in */pg_hba.conf file, you must restart postgreSQL:

```
    sudo service postgresql restart
```

### Configure nlohmann::json
Run
```
    sudo apt-get install nlohmann-json-dev 
```
Or, if it fails:
```
    sudo apt-get install nlohmann-json3-dev
```

### Autoconfigured libs

GoogleTest and libpqxx are downloaded and builded with main build

Reasons for that:

1. Google highly recommends build library with project to avoid undefined behaviour

2. libpqxx versions at apt sources are far behind from newest releases

## Build project

To build programm copy and paste to terminal:

```
    mkdir Debug && cd Debug && cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build .
```

## Run tests

To run tests, type 

```
    ctest
```

On build directory or 

```
    ServerTest/runServerTests 
```

for more verbose if something went wrong