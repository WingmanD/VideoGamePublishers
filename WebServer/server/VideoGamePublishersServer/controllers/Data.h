#pragma once

#include <drogon/HttpController.h>

enum class SearchType : int { All = 0, PublisherName = 1, StudioName = 2, GameTitle = 3 };

class Data : public drogon::HttpController<Data>
{
public:
    METHOD_LIST_BEGIN
        METHOD_ADD(Data::getPartialData, "/", drogon::Get);
        METHOD_ADD(Data::getExportJson, "/exportJson", drogon::Get);
        METHOD_ADD(Data::getExportCsv, "/exportCsv", drogon::Get);
    METHOD_LIST_END

public:
    /*
    * /data/pageIndex=1&pageSize=10&searchType=1&search=...
    */
    void getPartialData(const drogon::HttpRequestPtr& req,
                        std::function<void (const drogon::HttpResponsePtr&)>&& callback) const;

    /*
     * /data/exportJson?pageIndex=1&pageSize=10&searchType=1&search=...
     */
    void getExportJson(const drogon::HttpRequestPtr& req,
                       std::function<void (const drogon::HttpResponsePtr&)>&& callback) const;
    /*
     * /data/exportCsv?pageIndex=1&pageSize=10&searchType=1&search=...
     */
    void getExportCsv(const drogon::HttpRequestPtr& req,
                      std::function<void (const drogon::HttpResponsePtr&)>&& callback) const;
};
