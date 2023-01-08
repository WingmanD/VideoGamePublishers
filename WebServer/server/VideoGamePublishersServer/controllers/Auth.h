#pragma once
#include <drogon/HttpController.h>

class Auth : public drogon::HttpController<Auth>
{
public:
    METHOD_LIST_BEGIN
        METHOD_ADD(Auth::getLogin, "/login", drogon::Get);
        METHOD_ADD(Auth::getLogout, "/logout", drogon::Get);
    METHOD_LIST_END

public:
    // GET /auth/login
    void getLogin(const drogon::HttpRequestPtr& req,
                  std::function<void (const drogon::HttpResponsePtr&)>&& callback) const;

    // GET /auth/logout
    void getLogout(const drogon::HttpRequestPtr& req,
                   std::function<void (const drogon::HttpResponsePtr&)>&& callback) const;
};
