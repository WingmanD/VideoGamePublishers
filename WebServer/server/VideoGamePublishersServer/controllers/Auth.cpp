#include "Auth.h"

void Auth::getLogin(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
    const auto resp = drogon::HttpResponse::newRedirectionResponse("http://localhost:5000/login");
    callback(resp);
}

void Auth::getLogout(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback) const
{
    const auto resp = drogon::HttpResponse::newRedirectionResponse("http://localhost:5000/logout");
    callback(resp);
}
