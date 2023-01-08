#pragma once

#include <drogon/HttpController.h>

enum class FilterType : int { All = 0, PublisherName = 1, StudioName = 2, GameTitle = 3 };

struct ResponseWrapper
{
    ResponseWrapper(int status,
                    std::string message,
                    Json::Value response) : status(status), message(std::move(message)),
                                            response(std::move(response)) { }

public:
    int status;
    std::string message;
    Json::Value response;

    operator Json::Value() const
    {
        Json::Value json;
        json["status"] = statusToString(status);
        json["message"] = message;
        json["response"] = response;
        json["response"] = response;
        return json;
    }

private:
    std::string statusToString(int httpStatus) const
    {
        switch (httpStatus)
        {
        case 200:
            {
                return "OK";
            }
        case 201:
            {
                return "Created";
            }
        case 400:
            {
                return "Bad Request";
            }
        case 401:
            {
                return "Unauthorized";
            }
        case 403:
            {
                return "Forbidden";
            }
        case 404:
            {
                return "Not Found";
            }
        case 500:
            {
                return "Internal Server Error";
            }
        case 501:
            {
                return "Not Implemented";
            }
        default:
            {
                return "Unknown";
            }
        }
    }
};

class Data : public drogon::HttpController<Data>
{
public:
    METHOD_LIST_BEGIN
        METHOD_ADD(Data::getData, "/", drogon::Get);
        METHOD_ADD(Data::getPublisher, "/{publisherId}", drogon::Get);
        METHOD_ADD(Data::createPublisher, "/", drogon::Post);
        METHOD_ADD(Data::updatePublisher, "/{publisherId}", drogon::Put);
        METHOD_ADD(Data::notImplemented, "/", drogon::Patch);
        METHOD_ADD(Data::deletePublisher, "/{publisherId}", drogon::Delete);

        METHOD_ADD(Data::notImplemented, "/studio", drogon::Get);
        METHOD_ADD(Data::getStudio, "/studio/{studioId}", drogon::Get);
        METHOD_ADD(Data::notImplemented, "/studio", drogon::Post);
        METHOD_ADD(Data::notImplemented, "/studio/{studioId}", drogon::Patch);
        METHOD_ADD(Data::notImplemented, "/studio/{studioId}", drogon::Delete);

        METHOD_ADD(Data::notImplemented, "/title", drogon::Get);
        METHOD_ADD(Data::getTitle, "/title/{titleId}", drogon::Get);
        METHOD_ADD(Data::notImplemented, "/title", drogon::Post);
        METHOD_ADD(Data::notImplemented, "/title/{titleId}", drogon::Patch);
        METHOD_ADD(Data::notImplemented, "/title/{titleId}", drogon::Delete);

        METHOD_ADD(Data::notImplemented, "/director", drogon::Get);
        METHOD_ADD(Data::getDirector, "/director/{directorId}", drogon::Get);
        METHOD_ADD(Data::notImplemented, "/director", drogon::Post);
        METHOD_ADD(Data::notImplemented, "/director/{directorId}", drogon::Patch);
        METHOD_ADD(Data::notImplemented, "/director/{directorId}", drogon::Delete);

        METHOD_ADD(Data::getExportJson, "/exportJson", drogon::Get);
        METHOD_ADD(Data::notImplemented, "/exportJson", drogon::Post);
        METHOD_ADD(Data::notImplemented, "/exportJson", drogon::Patch);
        METHOD_ADD(Data::notImplemented, "/exportJson", drogon::Delete);

        METHOD_ADD(Data::getExportCsv, "/exportCsv", drogon::Get);
        METHOD_ADD(Data::notImplemented, "/exportCsv", drogon::Post);
        METHOD_ADD(Data::notImplemented, "/exportCsv", drogon::Patch);
        METHOD_ADD(Data::notImplemented, "/exportCsv", drogon::Delete);

        METHOD_ADD(Data::getRegenerateJson, "/regenerateJson", drogon::Get);
        METHOD_ADD(Data::getRegenerateCsv, "/regenerateCsv", drogon::Get);
    METHOD_LIST_END

public:
    /*
    * GET /data/?pageIndex=1&pageSize=10&searchType=1&search=...
    * queryString is optional
    */
    void getData(const drogon::HttpRequestPtr& req,
                 std::function<void (const drogon::HttpResponsePtr&)>&& callback) const;

    /*
    * GET /data/{publisherId}
    */
    void getPublisher(const drogon::HttpRequestPtr& req,
                      std::function<void (const drogon::HttpResponsePtr&)>&& callback, int publisherId) const;
    /*
    * GET /data/studio/{studioId}
    */
    void getStudio(const drogon::HttpRequestPtr& req,
                   std::function<void (const drogon::HttpResponsePtr&)>&& callback, int studioId) const;
    /*
    * GET /data/title/{titleId}
    */
    void getTitle(const drogon::HttpRequestPtr& req,
                  std::function<void (const drogon::HttpResponsePtr&)>&& callback, int titleId) const;

    /*
    * GET /data/director/{directorId}
    */
    void getDirector(const drogon::HttpRequestPtr& req,
                     std::function<void (const drogon::HttpResponsePtr&)>&& callback, int directorId) const;

    /*
     * POST /data/
     */
    void createPublisher(const drogon::HttpRequestPtr& req,
                         std::function<void (const drogon::HttpResponsePtr&)>&& callback) const;
    /*
     * Put /data/{publisherId}
     */
    void updatePublisher(const drogon::HttpRequestPtr& req,
                         std::function<void (const drogon::HttpResponsePtr&)>&& callback, int publisherId) const;

    /*
     * Other paths
     */
    void notImplemented(const drogon::HttpRequestPtr& req,
                        std::function<void (const drogon::HttpResponsePtr&)>&& callback, int publisherId) const;

    /*
     * DELETE /data/{publisherId}
     */
    void deletePublisher(const drogon::HttpRequestPtr& req,
                         std::function<void (const drogon::HttpResponsePtr&)>&& callback, int publisherId) const;

    /*
     * GET /data/exportJson?pageIndex=1&pageSize=10&searchType=1&search=...
     * queryString is optional
     */
    void getExportJson(const drogon::HttpRequestPtr& req,
                       std::function<void (const drogon::HttpResponsePtr&)>&& callback) const;
    /*
     * GET /data/exportCsv?pageIndex=1&pageSize=10&searchType=1&search=...
     * queryString is optional
     */
    void getExportCsv(const drogon::HttpRequestPtr& req,
                      std::function<void (const drogon::HttpResponsePtr&)>&& callback) const;

    /*
     * GET /data/regenerateJson
     */
    void getRegenerateJson(const drogon::HttpRequestPtr& req,
                           std::function<void (const drogon::HttpResponsePtr&)>&& callback) const;

    /*
     * GET /data/regenerateCsv
     */
    void getRegenerateCsv(const drogon::HttpRequestPtr& req,
                           std::function<void (const drogon::HttpResponsePtr&)>&& callback) const;

private:
    [[nodiscard]] std::string getFilterQueryByType(const std::string& searchQuery, FilterType filterType) const;
    [[nodiscard]] std::string makeQueryForTable(const std::string& filterQuery, int pageSize = 0,
                                                int pageIndex = -1) const;
    [[nodiscard]] std::string makeCountQueryForTable(const std::string& filterQuery) const;
};
