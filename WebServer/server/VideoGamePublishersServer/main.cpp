#include <drogon/drogon.h>

int main()
{
    //drogon::app().loadConfigFile("/Users/davidcemeljic/Library/CloudStorage/OneDrive-Personal/Documents/FER/7-sem/OR/Labosi/lab/VideoGamePublishers/WebServer/server/VideoGamePublishersServer/config.json");
    drogon::app().loadConfigFile("../server/VideoGamePublishersServer/config.json");

    drogon::app().run();
    return 0;
}
