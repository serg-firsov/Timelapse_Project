/*
* Time-lapse project
* Author: Sergey Firsov
*/

#ifndef TIMELAPSE2_WEBPAGECONTENT_H
#define TIMELAPSE2_WEBPAGECONTENT_H

#include <ESP8266WebServer.h>

class WebPageContent {
public:
    static void send(ESP8266WebServer*);
    static void pageNotFound(ESP8266WebServer*);
};

#endif //TIMELAPSE2_WEBPAGECONTENT_H
