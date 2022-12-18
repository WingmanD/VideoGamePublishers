#pragma once
#include <pqxx/transaction>

namespace Json
{
    class Value;
}

namespace pqxx
{
    class connection;
}

enum class DbResultStatus { Created, AlreadyExists, Updated, Deleted, NotFound, Error };

struct DbResult
{
    DbResult(int id, DbResultStatus status, std::string messageText) : id(id),
                                                                       status(status),
                                                                       messageText(std::move(messageText)) {}

    int id;
    DbResultStatus status;
    std::string messageText;
};

class DbStatics
{
public:
    static pqxx::connection* getDatabaseConnection();
    static DbResult createNewPublisher(const Json::Value& publisher);
    static DbResult createNewDirector(pqxx::work& dbTransaction, const Json::Value& director);
    static DbResult createNewStudio(pqxx::work& dbTransaction, const Json::Value& studio, int publisherId);
    static DbResult createNewTitle(pqxx::work& dbTransaction, const Json::Value& title, int studioId);

    static DbResult updatePublisher(int publisherId, const Json::Value& updatedPublisher);
    static DbResult updateStudio(pqxx::work& dbTransaction, int studioId, const Json::Value& updatedStudio);
    static DbResult updateTitle(pqxx::work& dbTransaction, int gameId, const Json::Value& newGame);

    static DbResult deletePublisher(int publisherId);
    static DbResult deleteDirector(pqxx::work& dbTransaction, int directorId);
    static DbResult deleteStudio(pqxx::work& dbTransaction, int studioId);
    static DbResult deleteTitle(pqxx::work& dbTransaction, int titleId);

    static Json::Value getPublisherData(int id);
    static Json::Value getStudioData(int id);
    static Json::Value getTitleData(int gameId);

    static std::string makePublisherQuery(const std::string& filterQuery);
    static std::string makeStudioQuery(const std::string& filterQuery);
    static std::string makeTitleQuery(const std::string& filterQuery);
    static std::string makeDirectorQuery(const std::string& filterQuery);
};
