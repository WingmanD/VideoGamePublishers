#include <drogon/drogon.h>

int main()
{
#ifdef WIN32
    drogon::app().loadConfigFile("../server/VideoGamePublishersServer/config.json");
#else
    drogon::app().loadConfigFile("../../server/VideoGamePublishersServer/configMac.json");
#endif

    drogon::HttpAppFramework::instance()
        .registerHandler("/",
                         [=](const drogon::HttpRequestPtr& req,
                             std::function<void (const drogon::HttpResponsePtr&)>&& callback)
                         {
                             auto params = req->getParameters();
                             auto cookies = req->getCookies();

                             drogon::HttpViewData data;
                             data.insert("params", params);

                             if (cookies.contains("user"))
                             {
                                 const std::string userData = drogon::utils::base64Decode(cookies.at("user"));

                                 Json::Value user;
                                 Json::CharReaderBuilder builder;
                                 std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
                                 std::string errors;

                                 if (!userData.empty() && cookies.contains("session"))
                                 {
                                     const bool parsingSuccessful = reader->parse(
                                         userData.c_str(), userData.c_str() + userData.size(), &user,
                                         &errors);
                                     if (parsingSuccessful)
                                     {
                                         data.insert("user", user);
                                     }
                                     else
                                     {
                                         std::cout << "Failed to parse JSON" << std::endl;
                                         std::cout << errors << std::endl;
                                     }
                                 }
                             }

                             const auto resp = drogon::HttpResponse::newHttpViewResponse("home.csp", data);
                             callback(resp);
                         });

    drogon::HttpAppFramework::instance()
        .registerHandler("/table",
                         [=](const drogon::HttpRequestPtr& req,
                             std::function<void (const drogon::HttpResponsePtr&)>&& callback)
                         {
                             auto params = req->getParameters();
                             auto cookies = req->getCookies();

                             drogon::HttpViewData data;
                             data.insert("params", params);

                             if (cookies.contains("user"))
                             {
                                 const std::string userData = drogon::utils::base64Decode(cookies.at("user"));

                                 Json::Value user;
                                 Json::CharReaderBuilder builder;
                                 std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
                                 std::string errors;

                                 if (!userData.empty() && cookies.contains("session"))
                                 {
                                     const bool parsingSuccessful = reader->parse(
                                         userData.c_str(), userData.c_str() + userData.size(), &user,
                                         &errors);
                                     if (parsingSuccessful)
                                     {
                                         data.insert("user", user);
                                     }
                                     else
                                     {
                                         std::cout << "Failed to parse JSON" << std::endl;
                                         std::cout << errors << std::endl;
                                     }
                                 }
                             }

                             const auto resp = drogon::HttpResponse::newHttpViewResponse("table.csp", data);
                             callback(resp);
                         });

    drogon::app().run();

    return 0;
}
