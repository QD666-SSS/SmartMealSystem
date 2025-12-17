#include "../include/WebServer.h"
#include <iostream>

int main() {
    WebServer server(8000, "www");
    server.start();
    return 0;
}
