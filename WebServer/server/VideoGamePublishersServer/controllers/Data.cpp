#include "Data.h"
#include <pqxx/pqxx>
#include <boost/dll/runtime_symbol_info.hpp>


void Data::getPartialData(const drogon::HttpRequestPtr& req,
                          std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
    int pageSize = 5;
    const std::string pageSizeParam = req->getParameter("pageSize");
    if (!pageSizeParam.empty())
    {
        pageSize = std::stoi(pageSizeParam);
    }

    int pageIndex = 0;
    const std::string pageIndexParam = req->getParameter("pageIndex");
    if (!pageIndexParam.empty())
    {
        pageIndex = std::stoi(pageIndexParam);
    }

    const std::string searchQuery = req->getParameter("search");

    SearchType searchType = SearchType::All;
    const std::string searchTypeParam = req->getParameter("searchType");
    if (!searchTypeParam.empty())
    {
        searchType = static_cast<SearchType>(std::stoi(searchTypeParam));
    }

    std::string searchQuerySql;
    switch (searchType)
    {
    case SearchType::All:
        {
            searchQuerySql = std::format(
                R"(where cast (publishers.id as varchar) like '%{}%' or publishers.name like '%{}%' or TO_CHAR(publishers."dateFounded", 'YYYY-MM-DD') like '%{}%' or publishers.country like '%{}%' or publishers.description like '%{}%' 
											 or publishers.website like '%{}%' or dirPublisher.name like '%{}%' or dirPublisher.surname like '%{}%' or studios.name like '%{}%' or studios.country like '%{}%' 
											 or TO_CHAR(studios."dateFounded", 'YYYY-MM-DD') like '%{}%' or cast (studios."numDevelopers" as varchar) like '%{}%' or dirStudio.name like '%{}%' or dirStudio.surname like '%{}%' 
											 or games.name like '%{}%' or TO_CHAR(games."releaseDate", 'YYYY-MM-DD') like '%{}%' or genre like '%{}%')",
                searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery,
                searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery,
                searchQuery);
            break;
        }
    case SearchType::PublisherName:
        {
            searchQuerySql = std::format(R"(where publishers.name like '%{}%')", searchQuery);
            break;
        }
    case SearchType::StudioName:
        {
            searchQuerySql = std::format(R"(where studios.name like '%{}%')", searchQuery);
            break;
        }
    case SearchType::GameTitle:
        {
            searchQuerySql = std::format(R"(where games.name like '%{}%')", searchQuery);
            break;
        }
    default:
        break;
    }

    const std::string query = std::format(
        R"(select json_agg(row_to_json(dataRows)) from (select publishers.id as "publisherID", publishers.name as "publisherName", publishers."dateFounded" as "publisherDateFounded", publishers.country as "publisherCountry", 
publishers.description as "publisherDesc", publishers.website as "publisherWebsite", dirPublisher.name as "publisherDirectorName", dirPublisher.surname as "publisherDirectorSurname", 
studios.name as "studioName", studios.country as "studioCountry", studios."dateFounded" as "studioDateFounded", studios."numDevelopers", dirStudio.name as "studioDirectorName", dirStudio.surname as "studioDirectorSurname",
games.name as "gameTitle", games."releaseDate" as "gameReleaseDate", genre
from publishers 
left join directors dirPublisher on "idDirector" = dirPublisher.id 
right join studios on publishers.id = "idPublisher" 
join directors dirStudio on studios."idDirector" = dirStudio.id
join games on games."idStudio" = studios.id {} order by publishers.id limit {} offset {}) dataRows;)",
        searchQuerySql, pageSize, pageIndex * pageSize);

    const std::string queryCount =
        std::format(
            R"(select count(*) from (select publishers.id as "publisherID", publishers.name as "publisherName", publishers."dateFounded" as "publisherDateFounded", publishers.country as "publisherCountry", 
publishers.description as "publisherDesc", publishers.website as "publisherWebsite", dirPublisher.name as "publisherDirectorName", dirPublisher.surname as "publisherDirectorSurname", 
studios.name as "studioName", studios.country as "studioCountry", studios."dateFounded" as "studioDateFounded", studios."numDevelopers", dirStudio.name as "studioDirectorName", dirStudio.surname as "studioDirectorSurname",
games.name as "gameTitle", games."releaseDate" as "gameReleaseDate", genre
from publishers 
left join directors dirPublisher on "idDirector" = dirPublisher.id 
right join studios on publishers.id = "idPublisher" 
join directors dirStudio on studios."idDirector" = dirStudio.id
join games on games."idStudio" = studios.id {}) dataRows)", searchQuerySql);
    
    pqxx::connection dbConnection(
        "dbname=VideoGamePublishers user=postgres password=David2702 hostaddr=127.0.0.1 port=5433");

    if (!dbConnection.is_open())
    {
        std::cout << "Can't open database" << std::endl;
        callback(drogon::HttpResponse::newNotFoundResponse());
    }
    pqxx::work dbTransaction(dbConnection);

    pqxx::result dbResult = dbTransaction.exec(query);
    pqxx::result dbTotalRowCount = dbTransaction.exec(queryCount);

    dbTransaction.commit();
    dbConnection.close();

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

    SearchType searchType = SearchType::All;
    const std::string searchTypeParam = req->getParameter("searchType");
    if (!searchTypeParam.empty())
    {
        searchType = static_cast<SearchType>(std::stoi(searchTypeParam));
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
    case SearchType::All:
        {
            searchQuerySql = std::format(
                R"(where cast (publishers.id as varchar) like '%{}%' or publishers.name like '%{}%' or TO_CHAR(publishers."dateFounded", 'YYYY-MM-DD') like '%{}%' or publishers.country like '%{}%' or publishers.description like '%{}%' 
											 or publishers.website like '%{}%' or dirPublisher.name like '%{}%' or dirPublisher.surname like '%{}%' or studios.name like '%{}%' or studios.country like '%{}%' 
											 or TO_CHAR(studios."dateFounded", 'YYYY-MM-DD') like '%{}%' or cast (studios."numDevelopers" as varchar) like '%{}%' or dirStudio.name like '%{}%' or dirStudio.surname like '%{}%' 
											 or games.name like '%{}%' or TO_CHAR(games."releaseDate", 'YYYY-MM-DD') like '%{}%' or genre like '%{}%')",
                searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery,
                searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery,
                searchQuery);
            break;
        }
    case SearchType::PublisherName:
        {
            searchQuerySql = std::format(R"(where publishers.name like '%{}%')", searchQuery);
            break;
        }
    case SearchType::StudioName:
        {
            searchQuerySql = std::format(R"(where studios.name like '%{}%')", searchQuery);
            break;
        }
    case SearchType::GameTitle:
        {
            searchQuerySql = std::format(R"(where games.name like '%{}%')", searchQuery);
            break;
        }
    default:
        break;
    }

    const std::string query = std::format(
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

    pqxx::connection dbConnection(
        "dbname=VideoGamePublishers user=postgres password=David2702 hostaddr=127.0.0.1 port=5433");

    if (!dbConnection.is_open())
    {
        std::cout << "Can't open database" << std::endl;
        callback(drogon::HttpResponse::newNotFoundResponse());
    }
    pqxx::work dbTransaction(dbConnection);

    pqxx::result dbResult = dbTransaction.exec(query);

    dbTransaction.commit();
    dbConnection.close();

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

    SearchType searchType = SearchType::All;
    const std::string searchTypeParam = req->getParameter("searchType");
    if (!searchTypeParam.empty())
    {
        searchType = static_cast<SearchType>(std::stoi(searchTypeParam));
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
    case SearchType::All:
        {
            searchQuerySql = std::format(
                R"(where cast (publishers.id as varchar) like '%{}%' or publishers.name like '%{}%' or TO_CHAR(publishers."dateFounded", 'YYYY-MM-DD') like '%{}%' or publishers.country like '%{}%' or publishers.description like '%{}%' 
											 or publishers.website like '%{}%' or dirPublisher.name like '%{}%' or dirPublisher.surname like '%{}%' or studios.name like '%{}%' or studios.country like '%{}%' 
											 or TO_CHAR(studios."dateFounded", 'YYYY-MM-DD') like '%{}%' or cast (studios."numDevelopers" as varchar) like '%{}%' or dirStudio.name like '%{}%' or dirStudio.surname like '%{}%' 
											 or games.name like '%{}%' or TO_CHAR(games."releaseDate", 'YYYY-MM-DD') like '%{}%' or genre like '%{}%')",
                searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery,
                searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery, searchQuery,
                searchQuery);
            break;
        }
    case SearchType::PublisherName:
        {
            searchQuerySql = std::format(R"(where publishers.name like '%{}%')", searchQuery);
            break;
        }
    case SearchType::StudioName:
        {
            searchQuerySql = std::format(R"(where studios.name like '%{}%')", searchQuery);
            break;
        }
    case SearchType::GameTitle:
        {
            searchQuerySql = std::format(R"(where games.name like '%{}%')", searchQuery);
            break;
        }
    default:
        break;
    }

    const std::string query = std::format(
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

    pqxx::connection dbConnection(
        "dbname=VideoGamePublishers user=postgres password=David2702 hostaddr=127.0.0.1 port=5433");

    if (!dbConnection.is_open())
    {
        std::cout << "Can't open database" << std::endl;
        callback(drogon::HttpResponse::newNotFoundResponse());
    }
    pqxx::work dbTransaction(dbConnection);

    pqxx::result dbResult = dbTransaction.exec(query);

    dbTransaction.commit();
    dbConnection.close();

    std::string csv;
    for (const auto& row : dbResult)
    {
        csv += std::format("{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{},{}\n",
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
