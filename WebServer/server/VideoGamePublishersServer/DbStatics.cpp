#include "DbStatics.h"

#include <fstream>
#include <iostream>
#include <pqxx/pqxx>
#include <fmt/format.h>
#include <json/reader.h>
#include <json/value.h>

pqxx::connection* DbStatics::getDatabaseConnection()
{
#ifdef WIN32
    return new pqxx::connection(
        "dbname=VideoGamePublishers user=postgres password=David2702 hostaddr=127.0.0.1 port=5433");
#else
    return new pqxx::connection("dbname=VideoGamePublishers user=postgres password=bazepodataka hostaddr=127.0.0.1 port=5432");
#endif
}

DbResult DbStatics::createNewPublisher(const Json::Value& publisher)
{
    if (publisher.empty())
    {
        const std::string message = "Error while trying to create new publisher - JSON empty!";
        std::cerr << message << std::endl;
        return {-1, DbResultStatus::Error, message};
    }

    std::unique_ptr<pqxx::connection> dbConnection = std::unique_ptr<pqxx::connection>(getDatabaseConnection());
    pqxx::work dbTransaction = pqxx::work(*dbConnection);

    const DbResult directorResult = createNewDirector(dbTransaction, publisher["publisherDirector"]);
    if (directorResult.status == DbResultStatus::Error)
    {
        dbTransaction.abort();
        std::cerr << "Could not create new publisher because because new director could not be added: " << publisher.
            toStyledString() << std::endl;
        return {
            -1, DbResultStatus::Error, "Could not create new publisher because because new director could not be added!"
        };
    }

    try
    {
        const pqxx::result existsResult = dbTransaction.exec(
            fmt::format(
                R"(select id from publishers where name = '{}' and "dateFounded" = '{}' and country = '{}' and description = '{}' and website = '{}' and "idDirector" = '{}';)",
                publisher["publisherName"].asString(),
                publisher["publisherDateFounded"].asString(),
                publisher["publisherCountry"].asString(),
                publisher["publisherDesc"].asString(),
                publisher["publisherWebsite"].asString(),
                directorResult.id));

        if (!existsResult.empty())
        {
            return {existsResult[0][0].as<int>(), DbResultStatus::AlreadyExists, "Publisher already exists!"};
        }

        const pqxx::result result =
            dbTransaction.exec(fmt::format(
                R"(INSERT INTO publishers (name, "dateFounded", country, description, website, "idDirector") VALUES ('{}', '{}', '{}', '{}', '{}', '{}') ON CONFLICT DO NOTHING RETURNING id;)",
                publisher["publisherName"].asString(),
                publisher["publisherDateFounded"].asString(),
                publisher["publisherCountry"].asString(),
                publisher["publisherDesc"].asString(),
                publisher["publisherWebsite"].asString(),
                directorResult.id
            ));

        const int publisherId = result[0][0].as<int>();

        for (const Json::Value& studio : publisher["studios"])
        {
            const DbResult studioResult = createNewStudio(dbTransaction, studio, publisherId);
            if (studioResult.status == DbResultStatus::Error)
            {
                dbTransaction.abort();
                return {
                    -1, DbResultStatus::Error,
                    "Could not create new publisher because because new studio could not be added!"
                };
            }
        }

        dbTransaction.commit();
        const std::string message = fmt::format("Publisher {} added successfully with id {}!",
                                                publisher["publisherName"].asString(),
                                                publisherId);
        std::cout << message << std::endl;
        return {publisherId, DbResultStatus::Created, message};
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error while creating new publisher: " << publisher.toStyledString() << std::endl;
        std::cerr << e.what() << std::endl;
    }

    dbTransaction.abort();
    return {-1, DbResultStatus::Error, "Error while trying to create new publisher!"};
}

DbResult DbStatics::createNewDirector(pqxx::work& dbTransaction, const Json::Value& director)
{
    if (director.empty())
    {
        const std::string message = "Error while trying to create new director - JSON empty!";
        std::cerr << message << std::endl;
        return {-1, DbResultStatus::Error, message};
    }

    try
    {
        const pqxx::result existsResult = dbTransaction.exec(
            fmt::format("select id from directors where name = '{}' and surname = '{}';",
                        director["name"].asString(), director["surname"].asString()));

        if (!existsResult.empty())
        {
            return {existsResult[0][0].as<int>(), DbResultStatus::AlreadyExists, "Director already exists!"};
        }

        const pqxx::result dbResult = dbTransaction.exec(fmt::format(
            "INSERT INTO directors (name, surname) VALUES ('{}', '{}') ON CONFLICT DO NOTHING RETURNING id;",
            director["name"].asString(), director["surname"].asString()
        ));

        const int directorId = dbResult[0][0].as<int>();

        const std::string message = fmt::format("Director {} added successfully with id {}",
                                                director["name"].asString(),
                                                directorId);
        std::cout << message << std::endl;
        return {directorId, DbResultStatus::Created, message};
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error while trying to create new director: " << director.toStyledString() << std::endl;
        std::cerr << e.what() << std::endl;
        return {-1, DbResultStatus::Error, "Error while trying to create new director!"};
    }
}

DbResult DbStatics::createNewStudio(pqxx::work& dbTransaction, const Json::Value& studio, int publisherId)
{
    if (studio.empty())
    {
        const std::string message = "Error while trying to create new studio - JSON empty!";
        std::cerr << message << std::endl;
        return {-1, DbResultStatus::Error, message};
    }

    try
    {
        const DbResult directorResult = createNewDirector(dbTransaction, studio["director"]);
        if (directorResult.status == DbResultStatus::Error)
        {
            std::cerr << "Could not create new studio because because new director could not be added: " << studio.
                toStyledString() << std::endl;
            return {
                -1, DbResultStatus::Error,
                "Could not create new studio because because new director could not be added!"
            };
        }

        const pqxx::result existsResult = dbTransaction.exec(
            fmt::format(
                R"(select id from studios where name = '{}' and country = '{}' and "dateFounded" = '{}' and "numDevelopers" = '{}' and "idPublisher" = '{}' and "idDirector" = '{}';)",
                studio["name"].asString(),
                studio["country"].asString(),
                studio["dateFounded"].asString(),
                studio["numDevelopers"].asInt(),
                publisherId,
                directorResult.id
            ));

        if (!existsResult.empty())
        {
            return {existsResult[0][0].as<int>(), DbResultStatus::AlreadyExists, "Studio already exists!"};
        }

        const pqxx::result dbResult = dbTransaction.exec(fmt::format(
            R"(insert into studios (name, country, "dateFounded", "numDevelopers", "idPublisher", "idDirector") values ('{}', '{}', '{}', '{}', '{}', '{}') on conflict do nothing returning id;)",
            studio["name"].asString(),
            studio["country"].asString(),
            studio["dateFounded"].asString(),
            studio["numDevelopers"].asInt(),
            publisherId,
            directorResult.id
        ));

        const int studioId = dbResult[0][0].as<int>();

        for (const Json::Value& title : studio["titles"])
        {
            const DbResult titleId = createNewTitle(dbTransaction, title, studioId);
            if (titleId.status == DbResultStatus::Error)
            {
                return {
                    -1, DbResultStatus::Error,
                    "Could not create new studio because because new title could not be added!"
                };
            }
        }

        const std::string message = fmt::format("Studio {} added successfully with id {}", studio["name"].asString(),
                                                studioId);
        std::cout << message << std::endl;
        return {studioId, DbResultStatus::Created, message};
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error while trying to create new studio: " << studio.toStyledString() << std::endl;
        std::cerr << e.what() << std::endl;
        return {-1, DbResultStatus::Error, "Error while trying to create new studio!"};
    }
}

DbResult DbStatics::createNewTitle(pqxx::work& dbTransaction, const Json::Value& title, int studioId)
{
    if (title.empty())
    {
        const std::string message = "Error while trying to create new title - JSON empty!";
        std::cerr << message << std::endl;
        return {-1, DbResultStatus::Error, message};
    }

    try
    {
        const pqxx::result existsResult = dbTransaction.exec(
            fmt::format(
                R"(select id from games where name = '{}' and "releaseDate" = '{}' and genre = '{}' and "idStudio" = '{}';)",
                title["gameTitle"].asString(),
                title["gameReleaseDate"].asString(),
                title["genre"].asString(),
                studioId
            ));

        if (!existsResult.empty())
        {
            return {existsResult[0][0].as<int>(), DbResultStatus::AlreadyExists, "Title already exists!"};
        }

        const pqxx::result dbResult = dbTransaction.exec(fmt::format(
            R"(insert into games (name, "releaseDate", genre, "idStudio") values ('{}', '{}', '{}', '{}') on conflict do nothing returning id;)",
            title["gameTitle"].asString(),
            title["gameReleaseDate"].asString(),
            title["genre"].asString(),
            studioId
        ));

        const int gameId = dbResult[0][0].as<int>();


        const std::string message = fmt::format("Title {} added successfully with id {}!",
                                                title["gameTitle"].asString(),
                                                gameId);
        std::cout << message << std::endl;
        return {gameId, DbResultStatus::Created, message};
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error while trying to create new title: " << title.toStyledString() << std::endl;
        std::cerr << e.what() << std::endl;
        return {-1, DbResultStatus::Error, "Error while trying to create new title!"};
    }
}

DbResult DbStatics::updatePublisher(int publisherId, const Json::Value& updatedPublisher)
{
    const Json::Value oldPublisher = getPublisherData(publisherId);
    if (oldPublisher.empty())
    {
        const std::string message = "Publisher not found!";
        std::cerr << message << std::endl;
        return {-1, DbResultStatus::NotFound, message};
    }

    const std::string directorIdQuery = fmt::format(
        R"(select "idDirector" from publishers where id = '{}';)",
        publisherId
    );

    const std::unique_ptr<pqxx::connection> dbConnection = std::unique_ptr<pqxx::connection>(getDatabaseConnection());
    pqxx::work dbTransaction = pqxx::work(*dbConnection);

    int oldDirectorId = -1;
    try
    {
        pqxx::result directorIdResult = dbTransaction.exec(directorIdQuery);
        if (!directorIdResult.empty())
        {
            oldDirectorId = directorIdResult[0][0].as<int>();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error while trying to get old director id for publisher with id " << publisherId << std::endl;
        std::cerr << e.what() << std::endl;
        return {-1, DbResultStatus::Error, "Error while trying to get old director id for publisher!"};
    }

    int newDirectorId = oldDirectorId;

    if (oldPublisher["director"] != updatedPublisher["director"])
    {
        deleteDirector(dbTransaction, oldDirectorId);
        const DbResult directorResult = createNewDirector(dbTransaction, updatedPublisher["director"]);
        newDirectorId = directorResult.id;
    }

    try
    {
        const pqxx::result dbResult = dbTransaction.exec(fmt::format(
            R"(update publishers set name = '{}', "dateFounded" = '{}', country = '{}', description = '{}', website = '{}', "idDirector" = '{}' where id = '{}';)",
            updatedPublisher["publisherName"].asString(),
            updatedPublisher["publisherDateFounded"].asString(),
            updatedPublisher["publisherCountry"].asString(),
            updatedPublisher["publisherDesc"].asString(),
            updatedPublisher["publisherWebsite"].asString(),
            newDirectorId,
            publisherId
        ));
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error while trying to update publisher with id " << publisherId << std::endl;
        std::cerr << e.what() << std::endl;
        return {-1, DbResultStatus::Error, "Error while trying to update publisher!"};
    }

    for (const Json::Value& studio : updatedPublisher["studios"])
    {
        try
        {
            const pqxx::result studioExistsResult = dbTransaction.exec(
                fmt::format(
                    R"(select id from studios where name = '{}' and "dateFounded" = '{}' and country = '{}' and "numDevelopers" = '{}' and "idPublisher" = '{}';)",
                    studio["name"].asString(),
                    studio["dateFounded"].asString(),
                    studio["country"].asString(),
                    studio["numDevelopers"].asInt(),
                    publisherId
                )
            );

            if (studioExistsResult.empty())
            {
                const DbResult studioResult = createNewStudio(dbTransaction, studio, publisherId);
                if (studioResult.status == DbResultStatus::Error)
                {
                    return {
                        -1, DbResultStatus::Error,
                        "Could not update publisher because because new studio could not be added!"
                    };
                }
            }
            else
            {
                const int studioId = studioExistsResult[0][0].as<int>();
                const DbResult studioResult = updateStudio(dbTransaction, studioId, studio);
                if (studioResult.status == DbResultStatus::Error)
                {
                    return {
                        -1, DbResultStatus::Error,
                        "Could not update publisher because because studio could not be updated!"
                    };
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error while trying to create studio!" << std::endl;
            std::cerr << e.what() << std::endl;
            return {-1, DbResultStatus::Error, "Error while trying to create studio!"};
        }
    }

    for (const Json::Value& oldStudio : oldPublisher["studios"])
    {
        bool shouldDeleteStudio = true;
        for (const Json::Value& newStudio : updatedPublisher["studios"])
        {
            if (oldStudio["name"].asString() == newStudio["name"].asString() &&
                oldStudio["dateFounded"].asString() == newStudio["dateFounded"].asString() &&
                oldStudio["country"].asString() == newStudio["country"].asString() &&
                oldStudio["numDevelopers"].asInt() == newStudio["numDevelopers"].asInt())
            {
                shouldDeleteStudio = false;
                break;
            }
        }

        if (shouldDeleteStudio)
        {
            try
            {
                const pqxx::result oldStudioIdResult = dbTransaction.exec(
                    fmt::format(
                        R"(select id from studios where name = '{}' and "dateFounded" = '{}' and country = '{}' and "numDevelopers" = '{}' and "idPublisher" = '{}';)",
                        oldStudio["name"].asString(),
                        oldStudio["dateFounded"].asString(),
                        oldStudio["country"].asString(),
                        oldStudio["numDevelopers"].asInt(),
                        publisherId
                    ));

                if (!oldStudioIdResult.empty())
                {
                    deleteStudio(dbTransaction, oldStudioIdResult[0][0].as<int>());
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error while trying to delete old studio!" << std::endl;
                std::cerr << e.what() << std::endl;
                return {-1, DbResultStatus::Error, "Error while trying to delete old studio!"};
            }
        }
    }

    dbTransaction.commit();

    const std::string message = fmt::format("Publisher {} updated successfully!",
                                            updatedPublisher["publisherName"].asString());
    std::cout << message << std::endl;
    return {publisherId, DbResultStatus::Updated, message};
}

DbResult DbStatics::updateStudio(pqxx::work& dbTransaction, int studioId, const Json::Value& updatedStudio)
{
    const Json::Value oldStudio = getStudioData(studioId);
    if (oldStudio.empty())
    {
        const std::string message = "Studio not found!";
        std::cerr << message << std::endl;
        return {-1, DbResultStatus::NotFound, message};
    }
    try
    {
        const pqxx::result dbResult = dbTransaction.exec(fmt::format(
            R"(update studios set name = '{}', "dateFounded" = '{}', country = '{}', "numDevelopers" = '{}' where id = '{}';)",
            updatedStudio["name"].asString(),
            updatedStudio["dateFounded"].asString(),
            updatedStudio["country"].asString(),
            updatedStudio["numDevelopers"].asInt(),
            studioId
        ));
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error while trying to update studio with id " << studioId << std::endl;
        std::cerr << e.what() << std::endl;
        return {-1, DbResultStatus::Error, "Error while trying to update studio!"};
    }

    for (const Json::Value& game : updatedStudio["titles"])
    {
        try
        {
            const pqxx::result gameExistsResult = dbTransaction.exec(
                fmt::format(
                    R"(select id from games where name = '{}' and "releaseDate" = '{}' and "idStudio" = '{}';)",
                    game["gameTitle"].asString(),
                    game["gameReleaseDate"].asString(),
                    studioId
                )
            );

            if (gameExistsResult.empty())
            {
                const DbResult gameResult = createNewTitle(dbTransaction, game, studioId);
                if (gameResult.status == DbResultStatus::Error)
                {
                    return {
                        -1, DbResultStatus::Error,
                        "Could not update studio because because new game could not be added!"
                    };
                }
            }
            else
            {
                const int gameId = gameExistsResult[0][0].as<int>();
                const DbResult gameResult = updateTitle(dbTransaction, gameId, game);
                if (gameResult.status == DbResultStatus::Error)
                {
                    return {
                        -1, DbResultStatus::Error,
                        "Could not update studio because because game could not be updated!"
                    };
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error while trying to create game!" << std::endl;
            std::cerr << e.what() << std::endl;
            return {-1, DbResultStatus::Error, "Error while trying to create game!"};
        }
    }

    for (const Json::Value& oldGame : oldStudio["titles"])
    {
        bool shouldDeleteGame = true;
        for (const Json::Value& newGame : updatedStudio["titles"])
        {
            if (oldGame["gameTitle"].asString() == newGame["gameTitle"].asString() &&
                oldGame["gameReleaseDate"].asString() == newGame["gameReleaseDate"].asString())
            {
                shouldDeleteGame = false;
                break;
            }
        }

        if (shouldDeleteGame)
        {
            try
            {
                std::cout << "Deleting game " << oldGame.toStyledString() << std::endl;

                const pqxx::result oldGameIdResult = dbTransaction.exec(
                    fmt::format(
                        R"(select id from games where name = '{}' and "releaseDate" = '{}' and genre = '{}' and "idStudio" = '{}';)",
                        oldGame["gameTitle"].asString(),
                        oldGame["gameReleaseDate"].asString(),
                        oldGame["genre"].asString(),
                        studioId
                    ));

                if (!oldGameIdResult.empty())
                {
                    deleteTitle(dbTransaction, oldGameIdResult[0][0].as<int>());
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error while trying to delete old game!" << std::endl;
                std::cerr << e.what() << std::endl;
                return {-1, DbResultStatus::Error, "Error while trying to delete old game!"};
            }
        }
    }

    return {studioId, DbResultStatus::Updated, "Studio updated successfully!"};
}

DbResult DbStatics::updateTitle(pqxx::work& dbTransaction, int gameId, const Json::Value& newGame)
{
    const Json::Value oldGame = getTitleData(gameId);
    if (oldGame.empty())
    {
        const std::string message = "Game not found!";
        std::cerr << message << std::endl;
        return {-1, DbResultStatus::NotFound, message};
    }

    try
    {
        const pqxx::result dbResult = dbTransaction.exec(fmt::format(
            R"(update games set name = '{}', "releaseDate" = '{}', genre = '{}' where id = '{}';)",
            newGame["gameTitle"].asString(),
            newGame["gameReleaseDate"].asString(),
            newGame["genre"].asString(),
            gameId
        ));

        return {gameId, DbResultStatus::Updated, "Game updated successfully!"};
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error while trying to update game with id " << gameId << std::endl;
        std::cerr << e.what() << std::endl;
        return {-1, DbResultStatus::Error, "Error while trying to update game!"};
    }
}

DbResult DbStatics::deletePublisher(int publisherId)
{
    if (publisherId == -1)
    {
        return {-1, DbResultStatus::Error, "Error while trying to delete publisher - id is -1!"};
    }

    try
    {
        const std::unique_ptr<pqxx::connection> dbConnection = std::unique_ptr<pqxx::connection>(
            getDatabaseConnection());
        pqxx::work dbTransaction(*dbConnection);

        const std::string directorIdQuery = fmt::format(
            R"(select "idDirector" from publishers where id = '{}';)",
            publisherId
        );
        const pqxx::result directorIdResult = dbTransaction.exec(directorIdQuery);

        const std::string studioIdQuery = fmt::format(
            R"(select id from studios where "idPublisher" = '{}';)",
            publisherId
        );
        const pqxx::result studioIdResult = dbTransaction.exec(studioIdQuery);
        for (const pqxx::row& row : studioIdResult)
        {
            deleteStudio(dbTransaction, row[0].as<int>());
        }

        const pqxx::result dbResult = dbTransaction.exec(fmt::format(
            R"(delete from publishers where id = '{}' returning id;)",
            publisherId
        ));

        if (dbResult.empty())
        {
            return {-1, DbResultStatus::NotFound, "Publisher not found!"};
        }

        if (!directorIdResult.empty())
        {
            deleteDirector(dbTransaction, directorIdResult[0][0].as<int>());
        }

        dbTransaction.commit();
        const std::string message = fmt::format("Publisher with id {} deleted successfully!", publisherId);
        std::cout << message << std::endl;
        return {publisherId, DbResultStatus::Deleted, message};
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error while trying to delete publisher with id " << publisherId << std::endl;
        std::cerr << e.what() << std::endl;
        return {-1, DbResultStatus::Error, "Error while trying to delete publisher!"};
    }
}

DbResult DbStatics::deleteDirector(pqxx::work& dbTransaction, int directorId)
{
    if (directorId == -1)
    {
        return {-1, DbResultStatus::Error, "Error while trying to delete director - id is -1!"};
    }

    const std::string query = fmt::format(R"(
        delete from directors where id = {};
    )", directorId);

    try
    {
        dbTransaction.exec(query);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error while trying to delete director with id " << directorId << std::endl;
        std::cerr << e.what() << std::endl;
        return {-1, DbResultStatus::Error, "Error while trying to delete director!"};
    }

    std::cout << "Director with id " << directorId << " deleted successfully!" << std::endl;
    return {directorId, DbResultStatus::Deleted, "Director deleted successfully!"};
}

DbResult DbStatics::deleteStudio(pqxx::work& dbTransaction, int studioId)
{
    if (studioId == -1)
    {
        return {-1, DbResultStatus::Error, "Error while trying to delete studio - id is -1!"};
    }

    const std::string directorIdQuery = fmt::format(R"(select "idDirector" from studios where id = {};)", studioId);
    const pqxx::result directorIdResult = dbTransaction.exec(directorIdQuery);

    const std::string titlesQuery = fmt::format(R"(select id from games where "idStudio" = {};)", studioId);
    try
    {
        const pqxx::result titlesResult = dbTransaction.exec(titlesQuery);
        for (const pqxx::row& title : titlesResult)
        {
            deleteTitle(dbTransaction, title[0].as<int>());
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error while trying to delete titles for studio with id " << studioId << std::endl;
        std::cerr << e.what() << std::endl;
    }

    const std::string deleteQuery = fmt::format(R"(
        delete from games where "idStudio" = {};
        delete from studios where id = {};
    )", studioId, studioId);

    try
    {
        dbTransaction.exec(deleteQuery);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error while trying to delete studio with id " << studioId << std::endl;
        std::cerr << e.what() << std::endl;
        return {-1, DbResultStatus::Error, "Error while trying to delete studio!"};
    }

    if (!directorIdResult.empty())
    {
        deleteDirector(dbTransaction, directorIdResult[0][0].as<int>());
    }

    return {studioId, DbResultStatus::Deleted, "Studio deleted successfully!"};
}

DbResult DbStatics::deleteTitle(pqxx::work& dbTransaction, int titleId)
{
    if (titleId == -1)
    {
        return {-1, DbResultStatus::Error, "Error while trying to delete title - id is -1!"};
    }

    const std::string query = fmt::format(R"(
        delete from games where id = {};
    )", titleId);
    try
    {
        dbTransaction.exec(query);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error while trying to delete title with id " << titleId << std::endl;
        std::cerr << e.what() << std::endl;
        return {-1, DbResultStatus::Error, "Error while trying to delete title!"};
    }

    return {titleId, DbResultStatus::Deleted, "Title deleted successfully!"};
}

Json::Value DbStatics::getPublisherData(int id)
{
    pqxx::connection* dbConnection = getDatabaseConnection();

    if (dbConnection == nullptr || !dbConnection->is_open())
    {
        std::cerr << "Can't open database" << std::endl;
        return {};
    }
    pqxx::work dbTransaction(*dbConnection);

    const std::string filter = fmt::format("where publishers.id = {}", id);
    pqxx::result dbResult = dbTransaction.exec(makePublisherQuery(filter));

    dbTransaction.commit();
    dbConnection->close();
    delete dbConnection;

    Json::Value publishersJson;

    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    std::string errors;

    if (!dbResult.empty() && dbResult[0].size() > 0)
    {
        if (dbResult[0][0].is_null())
        {
            publishersJson = Json::Value();
        }
        else
        {
            const std::string row0 = dbResult[0][0].c_str();

            bool parsingSuccessful = reader->parse(row0.c_str(), row0.c_str() + row0.size(), &publishersJson, &errors);
            if (!parsingSuccessful)
            {
                std::cout << "Failed to parse JSON" << std::endl;
                std::cout << errors << std::endl;

                publishersJson = Json::Value();
            }
        }
    }

    return publishersJson;
}

Json::Value DbStatics::getStudioData(int id)
{
    pqxx::connection* dbConnection = getDatabaseConnection();

    if (dbConnection == nullptr || !dbConnection->is_open())
    {
        std::cerr << "Can't open database" << std::endl;
        return {};
    }
    pqxx::work dbTransaction(*dbConnection);

    const std::string filter = fmt::format("where studios.id = {}", id);
    pqxx::result dbResult = dbTransaction.exec(makeStudioQuery(filter));

    dbTransaction.commit();
    dbConnection->close();
    delete dbConnection;

    Json::Value studioJson;

    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    std::string errors;

    if (!dbResult.empty() && dbResult[0].size() > 0)
    {
        if (dbResult[0][0].is_null())
        {
            studioJson = Json::Value();
        }
        else
        {
            const std::string row0 = dbResult[0][0].c_str();

            bool parsingSuccessful = reader->parse(row0.c_str(), row0.c_str() + row0.size(), &studioJson, &errors);
            if (!parsingSuccessful)
            {
                std::cout << "Failed to parse JSON" << std::endl;
                std::cout << errors << std::endl;

                studioJson = Json::Value();
            }
        }
    }

    return studioJson;
}

Json::Value DbStatics::getTitleData(int gameId)
{
    pqxx::connection* dbConnection = getDatabaseConnection();

    if (dbConnection == nullptr || !dbConnection->is_open())
    {
        std::cerr << "Can't open database" << std::endl;
        return {};
    }
    pqxx::work dbTransaction(*dbConnection);

    const std::string filter = fmt::format("where games.id = {}", gameId);
    pqxx::result dbResult = dbTransaction.exec(makeTitleQuery(filter));

    dbTransaction.commit();
    dbConnection->close();
    delete dbConnection;

    Json::Value studioJson;

    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    std::string errors;

    if (!dbResult.empty() && dbResult[0].size() > 0)
    {
        if (dbResult[0][0].is_null())
        {
            studioJson = Json::Value();
        }
        else
        {
            const std::string row0 = dbResult[0][0].c_str();

            bool parsingSuccessful = reader->parse(row0.c_str(), row0.c_str() + row0.size(), &studioJson, &errors);
            if (!parsingSuccessful)
            {
                std::cout << "Failed to parse JSON" << std::endl;
                std::cout << errors << std::endl;

                studioJson = Json::Value();
            }
        }
    }

    return studioJson;
}

bool DbStatics::updateJsonExportFile()
{
    const std::string query =
        R"(select json_agg(row_to_json(publisherList)) as "publishers"
    from (select publishers.id as "publisherID", publishers.name as "publisherName",
    publishers."dateFounded" as "publisherDateFounded",
    publishers.country as "publisherCountry", publishers.description as "publisherDesc",
    publishers.website as "publisherWebsite", json_build_object('name', directors.name, 'surname', directors.surname)
    as "publisherDirector", (select json_agg(row_to_json(publisherStudios)) from (select studios.name as "name",
    studios.country as "country", studios."dateFounded" as "dateFounded", studios."numDevelopers",
    json_build_object('name', directors.name, 'surname', directors.surname) as "director",
    (select json_agg(row_to_json(titles)) from (select games.name as "title", games."releaseDate" as "releaseDate",
    genre from games where games."idStudio" = studios.id) as titles) as "titles" from studios
    left join directors on studios."idDirector" = directors.id where studios."idPublisher" = publishers.id)
    as publisherStudios) as "studios" from publishers left join directors on publishers."idDirector" = directors.id)
    as publisherList)";

    const std::unique_ptr<pqxx::connection> dbConnection = std::unique_ptr<pqxx::connection>(getDatabaseConnection());
    if (dbConnection == nullptr || !dbConnection->is_open())
    {
        std::cerr << "DbStatics::updateJsonExportFile: Can't open database" << std::endl;
        return false;
    }

    pqxx::work dbTransaction = pqxx::work(*dbConnection);
    pqxx::result dbResult = dbTransaction.exec(query);

    if (!dbResult.empty() && dbResult[0].size() > 0)
    {
        if (dbResult[0][0].is_null())
        {
            std::cerr << "DbStatics::updateJsonExportFile: Can't open database" << std::endl;
            return false;
        }

        const std::string row0 = dbResult[0][0].c_str();
        try
        {
            std::ofstream file("../server/VideoGamePublishersServer/files/data.json");
            if (!file.is_open())
            {
                std::cerr << "DbStatics::updateJsonExportFile: Can't open file" << std::endl;
                return false;
            }

            file << row0;
            file.close();
            return true;
        }
        catch (std::exception& e)
        {
            std::cerr << "DbStatics::updateJsonExportFile: " << e.what() << std::endl;
            return false;
        }
    }

    return false;
}

bool DbStatics::updateCsvExportFile()
{
    const std::string query =
        R"(select publishers.id as "publisherID", publishers.name as "publisherName", publishers."dateFounded" as "publisherDateFounded", 
publishers.country as "publisherCountry", publishers.description as "publisherDesc", publishers.website as "publisherWebsite", 
dirPublisher.name as "publisherDirectorName", dirPublisher.surname as "publisherDirectorSurname", studios.name as "studioName", 
studios.country as "studioCountry", studios."dateFounded" as "studioDateFounded", studios."numDevelopers", dirStudio.name as 
"studioDirectorName", dirStudio.surname as "studioDirectorSurname", games.name as "gameTitle", games."releaseDate" as "gameReleaseDate", genre 
from publishers 
left join directors dirPublisher on "idDirector" = dirPublisher.id 
right join studios on publishers.id = "idPublisher" 
join directors dirStudio on studios."idDirector" = dirStudio.id 
join games on games."idStudio" = studios.id;)";

    const std::unique_ptr<pqxx::connection> dbConnection = std::unique_ptr<pqxx::connection>(getDatabaseConnection());
    if (dbConnection == nullptr || !dbConnection->is_open())
    {
        std::cerr << "DbStatics::updateJsonExportFile: Can't open database" << std::endl;
        return false;
    }

    pqxx::work dbTransaction = pqxx::work(*dbConnection);
    pqxx::result dbResult = dbTransaction.exec(query);

    if (!dbResult.empty() && dbResult[0].size() > 0)
    {
        if (dbResult[0][0].is_null())
        {
            std::cerr << "DbStatics::updateCsvExportFile: Can't open database" << std::endl;
            return false;
        }

        try
        {
            std::ofstream file("../server/VideoGamePublishersServer/files/data.csv");
            if (!file.is_open())
            {
                std::cerr << "DbStatics::updateCsvExportFile: Can't open file" << std::endl;
                return false;
            }

            for (const auto& row : dbResult)
            {
                std::string rowString;
                for (const auto& column : row)
                {
                    rowString += column.c_str();
                    rowString += ",";
                }
                file << rowString << std::endl;
            }

            file.close();
            return true;
        }
        catch (std::exception& e)
        {
            std::cerr << "DbStatics::updateCsvExportFile: " << e.what() << std::endl;
            return false;
        }
    }

    return false;
}

std::string DbStatics::makePublisherQuery(const std::string& filterQuery)
{
    return fmt::format(
        R"(with data_rows as (select publishers.id as "publisherID", publishers.name as "publisherName", publishers."dateFounded" as "publisherDateFounded", publishers.country as "publisherCountry", 
publishers.description as "publisherDesc", publishers.website as "publisherWebsite", dirPublisher.name as "publisherDirectorName", dirPublisher.surname as "publisherDirectorSurname", 
studios.id as "studioID", studios."idPublisher" as "studioPublisherID", studios.name as "studioName", studios.country as "studioCountry", studios."dateFounded" as "studioDateFounded", studios."numDevelopers" as "numDevelopers", 
dirStudio.name as "studioDirectorName", dirStudio.surname as "studioDirectorSurname",
games.id as gameID, games."idStudio" as "gameStudioID", games.name as "gameTitle", games."releaseDate" as "gameReleaseDate", genre
from publishers 
left join directors dirPublisher on "idDirector" = dirPublisher.id 
right join studios on publishers.id = "idPublisher" 
join directors dirStudio on studios."idDirector" = dirStudio.id
join games on games."idStudio" = studios.id {})

select row_to_json(export) as "publisherData" from (select distinct "publisherID", "publisherName", "publisherDateFounded", "publisherCountry" , 
"publisherDesc", "publisherWebsite",
jsonb_build_object('name', "publisherDirectorName", 'surname', "publisherDirectorSurname") as "publisherDirector", 
(select jsonb_agg(row_to_json(publisherStudios)) from 
 (select "studioName" as "name", "studioCountry" as "country", "studioDateFounded" as "dateFounded", "numDevelopers",
  json_build_object('name', "studioDirectorName", 'surname',"studioDirectorSurname") as "director",
  (select json_agg(row_to_json(titles)) from (select "gameTitle", "gameReleaseDate", genre from data_rows where "gameStudioID" = data_rows_studios."studioID") as titles) as "titles"
  from data_rows as data_rows_studios
  where data_rows_studios."studioPublisherID" = data_rows."publisherID") 
 as publisherStudios) as "studios"
from data_rows) export)", filterQuery);
}

std::string DbStatics::makeStudioQuery(const std::string& filterQuery)
{
    return fmt::format(
        R"(with data_rows as (select studios.id as "id", studios."idPublisher" as "publisherID", studios.name as "name", studios.country as "country", studios."dateFounded" as "dateFounded", studios."numDevelopers" as "numDevelopers", 
dirStudio.name as "studioDirectorName", dirStudio.surname as "studioDirectorSurname",
games.id as gameID, games."idStudio" as "studioID", games.name as "gameTitle", games."releaseDate" as "gameReleaseDate", genre
from studios
join directors dirStudio on studios."idDirector" = dirStudio.id
join games on games."idStudio" = studios.id {})

select row_to_json(export) as "studioData"
from
(select distinct "id", "publisherID", "name", "country", "dateFounded", "numDevelopers",
 jsonb_build_object('name', "studioDirectorName", 'surname', "studioDirectorSurname") as "director", 
 (select jsonb_agg(row_to_json(titles)) as "titles" from (select "gameTitle", "gameReleaseDate", genre from data_rows where "studioID" = "id") as titles) from data_rows) as "export";)",
        filterQuery);
}

std::string DbStatics::makeTitleQuery(const std::string& filterQuery)
{
    return fmt::format(
        R"(select row_to_json(game) from (select games.id as gameID, games."idStudio" as "studioID", games.name as "gameTitle", games."releaseDate" as "gameReleaseDate", genre
from games {}) as game;)",
        filterQuery);
}

std::string DbStatics::makeDirectorQuery(const std::string& filterQuery)
{
    return fmt::format(
        R"(select row_to_json(director) from (select directors.id as directorID, directors.name as "name", directors.surname as "surname"
from directors
{}) as director;)",
        filterQuery);
}
