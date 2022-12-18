#include "Data.h"
#include <pqxx/pqxx>
#include <boost/dll/runtime_symbol_info.hpp>
#include <fmt/format.h>

#include "../DbStatics.h"

void Data::getData(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
    int pageSize = 0;
    const std::string pageSizeParam = req->getParameter("pageSize");
    if (!pageSizeParam.empty())
    {
        pageSize = std::stoi(pageSizeParam);
    }

    int pageIndex = -1;
    const std::string pageIndexParam = req->getParameter("pageIndex");
    if (!pageIndexParam.empty())
    {
        pageIndex = std::stoi(pageIndexParam);
    }

    const std::string searchQuery = req->getParameter("search");

    FilterType searchType = FilterType::All;
    const std::string searchTypeParam = req->getParameter("searchType");
    if (!searchTypeParam.empty())
    {
        searchType = static_cast<FilterType>(std::stoi(searchTypeParam));
    }

    std::string searchQuerySql = getFilterQueryByType(searchQuery, searchType);

    const std::string query = makeQueryForTable(searchQuerySql, pageSize, pageIndex);
    const std::string queryCount = makeCountQueryForTable(searchQuerySql);

    pqxx::connection* dbConnection = DbStatics::getDatabaseConnection();
    if (dbConnection == nullptr || !dbConnection->is_open())
    {
        std::cerr << "Can't open database" << std::endl;
        callback(drogon::HttpResponse::newNotFoundResponse());
    }

    pqxx::work dbTransaction(*dbConnection);

    pqxx::result dbResult = dbTransaction.exec(query);
    pqxx::result dbTotalRowCount = dbTransaction.exec(queryCount);

    dbTransaction.commit();
    dbConnection->close();
    delete dbConnection;

    Json::Value responseJson;
    Json::Value publishersJson;
    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    std::string errors;

    if (!dbResult.empty() && dbResult[0].size() > 0)
    {
        if (dbResult[0][0].is_null())
        {
            publishersJson = Json::Value("");
        }
        else
        {
            const std::string row0 = dbResult[0][0].c_str();

            bool parsingSuccessful = reader->parse(row0.c_str(), row0.c_str() + row0.size(), &publishersJson, &errors);
            if (!parsingSuccessful)
            {
                std::cout << "Failed to parse JSON" << std::endl;
                std::cout << errors << std::endl;
                publishersJson = Json::Value("");
            }
        }
    }

    responseJson["publishers"] = publishersJson;
    responseJson["rowCount"] = dbTotalRowCount[0][0].as<std::string>();

    const auto response = drogon::HttpResponse::newHttpJsonResponse(responseJson);
    callback(response);
}

void Data::getPublisher(const drogon::HttpRequestPtr& req,
                        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                        int publisherId) const
{
    pqxx::connection* dbConnection = DbStatics::getDatabaseConnection();

    if (dbConnection == nullptr || !dbConnection->is_open())
    {
        std::cerr << "Can't open database" << std::endl;
        callback(drogon::HttpResponse::newNotFoundResponse());
    }
    pqxx::work dbTransaction(*dbConnection);

    const std::string filter = fmt::format("where publishers.id = {}", publisherId);
    pqxx::result dbResult = dbTransaction.exec(DbStatics::makePublisherQuery(filter));

    dbTransaction.commit();
    dbConnection->close();
    delete dbConnection;

    int status = 200;
    std::string message = "Fetched publisher data";
    Json::Value publishersJson;

    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    std::string errors;

    if (!dbResult.empty() && dbResult[0].size() > 0)
    {
        if (dbResult[0][0].is_null())
        {
            status = 500;
            message = fmt::format("Database error", publisherId);
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

                status = 500;
                message = fmt::format("Database error", publisherId);
                publishersJson = Json::Value();
            }
        }
    }
    else
    {
        status = 404;
        message = fmt::format("Publisher with ID {} does not exist", publisherId);
    }


    const auto response = drogon::HttpResponse::newHttpJsonResponse(
        ResponseWrapper(status, message, publishersJson));
    response->setStatusCode(static_cast<drogon::HttpStatusCode>(status));
    callback(response);
}

void Data::getStudio(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                     int studioId) const
{
    pqxx::connection* dbConnection = DbStatics::getDatabaseConnection();

    if (dbConnection == nullptr || !dbConnection->is_open())
    {
        std::cerr << "Can't open database" << std::endl;
        callback(drogon::HttpResponse::newNotFoundResponse());
    }
    pqxx::work dbTransaction(*dbConnection);

    const std::string filter = fmt::format("where studios.id = {}", studioId);
    pqxx::result dbResult = dbTransaction.exec(DbStatics::makeStudioQuery(filter));

    dbTransaction.commit();
    dbConnection->close();
    delete dbConnection;

    int status = 200;
    std::string message = "Fetched studio data";
    Json::Value studioJson;

    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    std::string errors;


    if (!dbResult.empty() && dbResult[0].size() > 0)
    {
        if (dbResult[0][0].is_null())
        {
            status = 500;
            message = fmt::format("Database error", studioId);
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

                status = 500;
                message = "Database error";
                studioJson = Json::Value();
            }
        }
    }
    else
    {
        status = 404;
        message = fmt::format("Studio with ID {} does not exist", studioId);
    }

    const auto response = drogon::HttpResponse::newHttpJsonResponse(
        ResponseWrapper(status, message, studioJson));
    response->setStatusCode(static_cast<drogon::HttpStatusCode>(status));
    callback(response);
}

void Data::getTitle(const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    int titleId) const
{
    pqxx::connection* dbConnection = DbStatics::getDatabaseConnection();

    if (dbConnection == nullptr || !dbConnection->is_open())
    {
        std::cerr << "Can't open database" << std::endl;
        callback(drogon::HttpResponse::newNotFoundResponse());
    }
    pqxx::work dbTransaction(*dbConnection);

    const std::string filter = fmt::format("where games.id = {}", titleId);
    pqxx::result dbResult = dbTransaction.exec(DbStatics::makeTitleQuery(filter));

    dbTransaction.commit();
    dbConnection->close();
    delete dbConnection;

    int status = 200;
    std::string message = "Fetched title data";
    Json::Value studioJson;

    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    std::string errors;


    if (!dbResult.empty() && dbResult[0].size() > 0)
    {
        if (dbResult[0][0].is_null())
        {
            status = 500;
            message = fmt::format("Database error", titleId);
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

                status = 500;
                message = "Database error";
                studioJson = Json::Value();
            }
        }
    }
    else
    {
        status = 404;
        message = fmt::format("Title with ID {} does not exist", titleId);
    }

    const auto response = drogon::HttpResponse::newHttpJsonResponse(
        ResponseWrapper(status, message, studioJson));
    response->setStatusCode(static_cast<drogon::HttpStatusCode>(status));
    callback(response);
}

void Data::getDirector(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback, int directorId) const
{
    pqxx::connection* dbConnection = DbStatics::getDatabaseConnection();

    if (dbConnection == nullptr || !dbConnection->is_open())
    {
        std::cerr << "Can't open database" << std::endl;
        callback(drogon::HttpResponse::newNotFoundResponse());
    }
    pqxx::work dbTransaction(*dbConnection);

    const std::string filter = fmt::format("where directors.id = {}", directorId);
    const pqxx::result dbResult = dbTransaction.exec(DbStatics::makeDirectorQuery(filter));

    dbTransaction.commit();
    dbConnection->close();
    delete dbConnection;

    int status = 200;
    std::string message = "Fetched director data";
    Json::Value studioJson;

    Json::CharReaderBuilder builder;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    std::string errors;


    if (!dbResult.empty() && dbResult[0].size() > 0)
    {
        if (dbResult[0][0].is_null())
        {
            status = 500;
            message = fmt::format("Database error", directorId);
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

                status = 500;
                message = "Database error";
                studioJson = Json::Value();
            }
        }
    }
    else
    {
        status = 404;
        message = fmt::format("Director with ID {} does not exist", directorId);
    }

    const auto response = drogon::HttpResponse::newHttpJsonResponse(
        ResponseWrapper(status, message, studioJson));
    response->setStatusCode(static_cast<drogon::HttpStatusCode>(status));
    callback(response);
}

void Data::createPublisher(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
    const auto requestJson = req->getJsonObject();
    if (requestJson == nullptr || requestJson->empty())
    {
        const auto response = drogon::HttpResponse::newHttpJsonResponse(
            ResponseWrapper(400, "Invalid request body", Json::Value()));
        response->setStatusCode(static_cast<drogon::HttpStatusCode>(400));
        callback(response);
        return;
    }

    const DbResult newPublisherResult = DbStatics::createNewPublisher(*requestJson.get());
    if (newPublisherResult.status == DbResultStatus::Error)
    {
        const auto response = drogon::HttpResponse::newHttpJsonResponse(
            ResponseWrapper(400, newPublisherResult.messageText, Json::Value()));
        response->setStatusCode(static_cast<drogon::HttpStatusCode>(400));
        callback(response);
        return;
    }

    int status = 400;
    const Json::Value publisherJson = DbStatics::getPublisherData(newPublisherResult.id);

    if (newPublisherResult.status == DbResultStatus::Created)
    {
        status = 201;
    }

    if (newPublisherResult.status == DbResultStatus::AlreadyExists)
    {
        status = 200;
    }

    const auto response = drogon::HttpResponse::newHttpJsonResponse(
        ResponseWrapper(status, newPublisherResult.messageText, publisherJson));
    response->setStatusCode(static_cast<drogon::HttpStatusCode>(status));
    callback(response);
}

void Data::updatePublisher(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback, int publisherId) const
{
    const auto requestJson = req->getJsonObject();
    
    if (requestJson == nullptr || requestJson->empty())
    {
        const auto response = drogon::HttpResponse::newHttpJsonResponse(
            ResponseWrapper(400, "Invalid request body", Json::Value()));
        response->setStatusCode(static_cast<drogon::HttpStatusCode>(400));
        callback(response);
        return;
    }

    const DbResult updatePublisherResult = DbStatics::updatePublisher(publisherId, *requestJson.get());
    if (updatePublisherResult.status == DbResultStatus::Error)
    {
        const auto response = drogon::HttpResponse::newHttpJsonResponse(
            ResponseWrapper(400, updatePublisherResult.messageText, Json::Value()));
        response->setStatusCode(static_cast<drogon::HttpStatusCode>(400));
        callback(response);
        return;
    }

    int status = 400;
    const Json::Value publisherJson = DbStatics::getPublisherData(publisherId);

    if (updatePublisherResult.status == DbResultStatus::Updated)
    {
        status = 200;
    }

    if (updatePublisherResult.status == DbResultStatus::NotFound)
    {
        status = 404;
    }

    const auto response = drogon::HttpResponse::newHttpJsonResponse(
        ResponseWrapper(status, updatePublisherResult.messageText, publisherJson));
    response->setStatusCode(static_cast<drogon::HttpStatusCode>(status));
    callback(response);
}

void Data::notImplemented(const drogon::HttpRequestPtr& req,
    std::function<void(const drogon::HttpResponsePtr&)>&& callback, int publisherId) const
{
    
    const auto response = drogon::HttpResponse::newHttpJsonResponse(
        ResponseWrapper(501, "Not implemented", Json::Value()));
    response->setStatusCode(static_cast<drogon::HttpStatusCode>(501));
    callback(response);}

void Data::deletePublisher(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback, int publisherId) const
{
    const DbResult deletePublisherResult = DbStatics::deletePublisher(publisherId);
    if (deletePublisherResult.status == DbResultStatus::Error)
    {
        const auto response = drogon::HttpResponse::newHttpJsonResponse(
            ResponseWrapper(400, deletePublisherResult.messageText, Json::Value()));
        response->setStatusCode(static_cast<drogon::HttpStatusCode>(400));
        callback(response);
        return;
    }

    int status = 400;

    if (deletePublisherResult.status == DbResultStatus::Deleted)
    {
        status = 200;
    }

    if (deletePublisherResult.status == DbResultStatus::NotFound)
    {
        status = 404;
    }

    const auto response = drogon::HttpResponse::newHttpJsonResponse(
        ResponseWrapper(status, deletePublisherResult.messageText, Json::Value()));
    response->setStatusCode(static_cast<drogon::HttpStatusCode>(status));
    callback(response);
}

void Data::getExportJson(const drogon::HttpRequestPtr& req,
                         std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
    bool exportAll = true;

    int pageSize = 5;
    const std::string pageSizeParam = req->getParameter("pageSize");
    if (!pageSizeParam.empty())
    {
        pageSize = std::stoi(pageSizeParam);
        exportAll = false;
    }

    int pageIndex = 0;
    const std::string pageIndexParam = req->getParameter("pageIndex");
    if (!pageIndexParam.empty())
    {
        pageIndex = std::stoi(pageIndexParam);
        exportAll = false;
    }

    const std::string searchQuery = req->getParameter("search");

    FilterType searchType = FilterType::All;
    const std::string searchTypeParam = req->getParameter("searchType");
    if (!searchTypeParam.empty())
    {
        searchType = static_cast<FilterType>(std::stoi(searchTypeParam));
        exportAll = false;
    }

    if (exportAll)
    {
        const std::string fullPath = boost::dll::program_location().parent_path().parent_path().parent_path().string() +
            "/server/VideoGamePublishersServer/files/data.json";
        const auto response = drogon::HttpResponse::newFileResponse(fullPath, "data.json");
        callback(response);
        return;
    }


    std::string searchQuerySql;
    switch (searchType)
    {
    case FilterType::All:
        {
            searchQuerySql = fmt::format(
                R"(where cast (publishers.id as varchar) like '%{}%' or publishers.name like '%{}%' or TO_CHAR(publishers."dateFounded", 'YYYY-MM-DD') like '%{}%' or publishers.country like '%{}%' or publishers.description like '%{}%' 
											 or publishers.website like '%{}%' or dirPublisher.name like '%{}%' or dirPublisher.surname like '%{}%' or studios.name like '%{}%' or studios.country like '%{}%' 
											 or TO_CHAR(studios."dateFounded", 'YYYY-MM-DD') like '%{}%' or cast (studios."numDevelopers" as varchar) like '%{}%' or dirStudio.name like '%{}%' or dirStudio.surname like '%{}%' 
											 or games.name like '%{}%' or TO_CHAR(games."releaseDate", 'YYYY-MM-DD') like '%{}%' or genre like '%{}%')",
                searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery,
                searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery,
                searchQuery);
            break;
        }
    case FilterType::PublisherName:
        {
            searchQuerySql = fmt::format(R"(where publishers.name like '%{}%')", searchQuery);
            break;
        }
    case FilterType::StudioName:
        {
            searchQuerySql = fmt::format(R"(where studios.name like '%{}%')", searchQuery);
            break;
        }
    case FilterType::GameTitle:
        {
            searchQuerySql = fmt::format(R"(where games.name like '%{}%')", searchQuery);
            break;
        }
    default:
        break;
    }

    const std::string query = fmt::format(
        R"(with data_rows as (select publishers.id as "publisherID", publishers.name as "publisherName", publishers."dateFounded" as "publisherDateFounded", publishers.country as "publisherCountry", 
publishers.description as "publisherDesc", publishers.website as "publisherWebsite", dirPublisher.name as "publisherDirectorName", dirPublisher.surname as "publisherDirectorSurname", 
studios.id as "studioID", studios."idPublisher" as "studioPublisherID", studios.name as "studioName", studios.country as "studioCountry", studios."dateFounded" as "studioDateFounded", studios."numDevelopers" as "numDevelopers", 
dirStudio.name as "studioDirectorName", dirStudio.surname as "studioDirectorSurname",
games.id as gameID, games."idStudio" as "gameStudioID", games.name as "gameTitle", games."releaseDate" as "gameReleaseDate", genre
from publishers 
left join directors dirPublisher on "idDirector" = dirPublisher.id 
right join studios on publishers.id = "idPublisher" 
join directors dirStudio on studios."idDirector" = dirStudio.id
join games on games."idStudio" = studios.id {} order by publishers.id limit {} offset {})

select json_agg(row_to_json(export)) as "publisherData" from (select distinct "publisherID", "publisherName", "publisherDateFounded", "publisherCountry" , 
"publisherDesc", "publisherWebsite",
jsonb_build_object('name', "publisherDirectorName", 'surname', "publisherDirectorSurname") as "publisherDirector", 
(select jsonb_agg(row_to_json(publisherStudios)) from 
 (select "studioName" as "name", "studioCountry" as "country", "studioDateFounded" as "dateFounded", "numDevelopers",
  json_build_object('name', "studioDirectorName", 'surname',"studioDirectorSurname") as "director",
  (select json_agg(row_to_json(titles)) from (select "gameTitle", "gameReleaseDate", genre from data_rows where "gameStudioID" = data_rows_studios."studioID") as titles) as "titles"
  from data_rows as data_rows_studios
  where data_rows_studios."studioPublisherID" = data_rows."publisherID") 
 as publisherStudios) as "studios"
from data_rows) export
)", searchQuerySql, pageSize,
        pageSize * pageIndex);

    pqxx::connection* dbConnection = DbStatics::getDatabaseConnection();

    if (dbConnection == nullptr || !dbConnection->is_open())
    {
        std::cout << "Can't open database" << std::endl;
        callback(drogon::HttpResponse::newNotFoundResponse());
    }
    pqxx::work dbTransaction(*dbConnection);

    pqxx::result dbResult = dbTransaction.exec(query);

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
            publishersJson = Json::Value("");
        }
        else
        {
            const std::string row0 = dbResult[0][0].c_str();

            bool parsingSuccessful = reader->parse(row0.c_str(), row0.c_str() + row0.size(), &publishersJson, &errors);
            if (!parsingSuccessful)
            {
                std::cout << "Failed to parse JSON" << std::endl;
                std::cout << errors << std::endl;
                publishersJson = Json::Value("");
            }
        }
    }

    const auto response = drogon::HttpResponse::newHttpJsonResponse(publishersJson);
    callback(response);
}

void Data::getExportCsv(const drogon::HttpRequestPtr& req,
                        std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
    bool exportAll = true;

    int pageSize = 5;
    const std::string pageSizeParam = req->getParameter("pageSize");
    if (!pageSizeParam.empty())
    {
        pageSize = std::stoi(pageSizeParam);
        exportAll = false;
    }

    int pageIndex = 0;
    const std::string pageIndexParam = req->getParameter("pageIndex");
    if (!pageIndexParam.empty())
    {
        pageIndex = std::stoi(pageIndexParam);
        exportAll = false;
    }

    const std::string searchQuery = req->getParameter("search");

    FilterType searchType = FilterType::All;
    const std::string searchTypeParam = req->getParameter("searchType");
    if (!searchTypeParam.empty())
    {
        searchType = static_cast<FilterType>(std::stoi(searchTypeParam));
        exportAll = false;
    }

    if (exportAll)
    {
        const std::string fullPath = boost::dll::program_location().parent_path().parent_path().parent_path().string() +
            "/server/VideoGamePublishersServer/files/data.csv";
        const auto response = drogon::HttpResponse::newFileResponse(fullPath, "data.csv");
        callback(response);
        return;
    }


    std::string searchQuerySql;
    switch (searchType)
    {
    case FilterType::All:
        {
            searchQuerySql = fmt::format(
                R"(where cast (publishers.id as varchar) like '%{}%' or publishers.name like '%{}%' or TO_CHAR(publishers."dateFounded", 'YYYY-MM-DD') like '%{}%' or publishers.country like '%{}%' or publishers.description like '%{}%' 
											 or publishers.website like '%{}%' or dirPublisher.name like '%{}%' or dirPublisher.surname like '%{}%' or studios.name like '%{}%' or studios.country like '%{}%' 
											 or TO_CHAR(studios."dateFounded", 'YYYY-MM-DD') like '%{}%' or cast (studios."numDevelopers" as varchar) like '%{}%' or dirStudio.name like '%{}%' or dirStudio.surname like '%{}%' 
											 or games.name like '%{}%' or TO_CHAR(games."releaseDate", 'YYYY-MM-DD') like '%{}%' or genre like '%{}%')",
                searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery,
                searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery,
                searchQuery);
            break;
        }
    case FilterType::PublisherName:
        {
            searchQuerySql = fmt::format(R"(where publishers.name like '%{}%')", searchQuery);
            break;
        }
    case FilterType::StudioName:
        {
            searchQuerySql = fmt::format(R"(where studios.name like '%{}%')", searchQuery);
            break;
        }
    case FilterType::GameTitle:
        {
            searchQuerySql = fmt::format(R"(where games.name like '%{}%')", searchQuery);
            break;
        }
    default:
        break;
    }

    const std::string query = fmt::format(
        R"((select publishers.id as "publisherID", publishers.name as "publisherName", publishers."dateFounded" as "publisherDateFounded", publishers.country as "publisherCountry", 
publishers.description as "publisherDesc", publishers.website as "publisherWebsite", dirPublisher.name as "publisherDirectorName", dirPublisher.surname as "publisherDirectorSurname", 
studios.name as "studioName", studios.country as "studioCountry", studios."dateFounded" as "studioDateFounded", studios."numDevelopers", dirStudio.name as "studioDirectorName", dirStudio.surname as "studioDirectorSurname",
games.name as "gameTitle", games."releaseDate" as "gameReleaseDate", genre
from publishers 
left join directors dirPublisher on "idDirector" = dirPublisher.id 
right join studios on publishers.id = "idPublisher" 
join directors dirStudio on studios."idDirector" = dirStudio.id
join games on games."idStudio" = studios.id {} order by publishers.id limit {} offset {});)", searchQuerySql, pageSize,
        pageSize * pageIndex);

    pqxx::connection* dbConnection = DbStatics::getDatabaseConnection();

    if (dbConnection == nullptr || !dbConnection->is_open())
    {
        std::cout << "Can't open database" << std::endl;
        callback(drogon::HttpResponse::newNotFoundResponse());
    }
    pqxx::work dbTransaction(*dbConnection);

    pqxx::result dbResult = dbTransaction.exec(query);

    dbTransaction.commit();
    dbConnection->close();
    delete dbConnection;

    std::string csv;
    for (const auto& row : dbResult)
    {
        csv += fmt::format("{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{}\n",
                           row[0].as<std::string>(), row[1].as<std::string>(), row[2].as<std::string>(),
                           row[3].as<std::string>(), row[4].as<std::string>(), row[5].as<std::string>(),
                           row[6].as<std::string>(), row[7].as<std::string>(), row[8].as<std::string>(),
                           row[9].as<std::string>(), row[10].as<std::string>(), row[11].as<std::string>(),
                           row[12].as<std::string>(), row[13].as<std::string>(), row[14].as<std::string>(),
                           row[15].as<std::string>(), row[16].as<std::string>());
    }

    Json::Value value;
    value["csv"] = csv;
    const auto response = drogon::HttpResponse::newHttpJsonResponse(csv);
    callback(response);
}

std::string Data::getFilterQueryByType(const std::string& searchQuery, FilterType filterType) const
{
    switch (filterType)
    {
    case FilterType::All:
        {
            return fmt::format(
                R"(where cast (publishers.id as varchar) like '%{}%' or publishers.name like '%{}%' or TO_CHAR(publishers."dateFounded", 'YYYY-MM-DD') like '%{}%' or publishers.country like '%{}%' or publishers.description like '%{}%' 
											 or publishers.website like '%{}%' or dirPublisher.name like '%{}%' or dirPublisher.surname like '%{}%' or studios.name like '%{}%' or studios.country like '%{}%' 
											 or TO_CHAR(studios."dateFounded", 'YYYY-MM-DD') like '%{}%' or cast (studios."numDevelopers" as varchar) like '%{}%' or dirStudio.name like '%{}%' or dirStudio.surname like '%{}%' 
											 or games.name like '%{}%' or TO_CHAR(games."releaseDate", 'YYYY-MM-DD') like '%{}%' or genre like '%{}%')",
                searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery,
                searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery,
                searchQuery);
        }
    case FilterType::PublisherName:
        {
            return fmt::format(R"(where publishers.name like '%{}%')", searchQuery);
        }
    case FilterType::StudioName:
        {
            return fmt::format(R"(where studios.name like '%{}%')", searchQuery);
        }
    case FilterType::GameTitle:
        {
            return fmt::format(R"(where games.name like '%{}%')", searchQuery);
        }
    }

    return "";
}

std::string Data::makeQueryForTable(const std::string& filterQuery, int pageSize /*= 0*/, int pageIndex /*= -1*/) const
{
    std::string limit;
    if (pageIndex > -1 && pageSize > 0)
    {
        limit = fmt::format("limit {} offset {}", pageSize, pageIndex * pageSize);
    }

    return fmt::format(
        R"(select json_agg(row_to_json(dataRows)) from (select publishers.id as "publisherID", publishers.name as "publisherName", publishers."dateFounded" as "publisherDateFounded", publishers.country as "publisherCountry", 
publishers.description as "publisherDesc", publishers.website as "publisherWebsite", dirPublisher.name as "publisherDirectorName", dirPublisher.surname as "publisherDirectorSurname", 
studios.name as "studioName", studios.country as "studioCountry", studios."dateFounded" as "studioDateFounded", studios."numDevelopers", dirStudio.name as "studioDirectorName", dirStudio.surname as "studioDirectorSurname",
games.name as "gameTitle", games."releaseDate" as "gameReleaseDate", genre
from publishers 
left join directors dirPublisher on "idDirector" = dirPublisher.id 
right join studios on publishers.id = "idPublisher" 
join directors dirStudio on studios."idDirector" = dirStudio.id
join games on games."idStudio" = studios.id {} order by publishers.id {}) dataRows;)",
        filterQuery, limit);
}

std::string Data::makeCountQueryForTable(const std::string& filterQuery) const
{
    return fmt::format(
        R"(select count(*) from (select publishers.id as "publisherID", publishers.name as "publisherName", publishers."dateFounded" as "publisherDateFounded", publishers.country as "publisherCountry", 
publishers.description as "publisherDesc", publishers.website as "publisherWebsite", dirPublisher.name as "publisherDirectorName", dirPublisher.surname as "publisherDirectorSurname", 
studios.name as "studioName", studios.country as "studioCountry", studios."dateFounded" as "studioDateFounded", studios."numDevelopers", dirStudio.name as "studioDirectorName", dirStudio.surname as "studioDirectorSurname",
games.name as "gameTitle", games."releaseDate" as "gameReleaseDate", genre
from publishers 
left join directors dirPublisher on "idDirector" = dirPublisher.id 
right join studios on publishers.id = "idPublisher" 
join directors dirStudio on studios."idDirector" = dirStudio.id
join games on games."idStudio" = studios.id {}) dataRows)", filterQuery);
}
