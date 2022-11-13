#include <drogon/drogon.h>

int main()
{
    drogon::app().addListener("0.0.0.0", 80);
    drogon::app().loadConfigFile("../server/VideoGamePublishersServer/config.json");

    drogon::app().run();
    return 0;
}
