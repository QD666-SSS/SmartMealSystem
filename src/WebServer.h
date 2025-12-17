#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <string>
#include <atomic>

class Database;
class RecommendationEngine;

// 跨平台的socket处理
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

class SimpleWebServer {
private:
    int port;
    socket_t server_socket;
    std::atomic<bool> running;
    Database* db;
    RecommendationEngine* engine;

public:
    SimpleWebServer(Database* db, RecommendationEngine* engine, int port = 8080);
    ~SimpleWebServer();
    
    void start();
    void stop();

private:
    bool initialize();
    void run();
    void handleClient(socket_t client_socket);
    std::string handleHttpRequest(const std::string& request);
    std::string serveIndexPage();
    std::string handleLogin(const std::string& request);
    std::string handleRegister(const std::string& request);
    std::string handleGetProfile(const std::string& request);
    std::string handleGetFoods(const std::string& request);
    std::string handleGetRecommendations(const std::string& request);
    std::string handleGetMeals(const std::string& request);
    std::string handleGetStatistics(const std::string& request);
    void cleanup();
};

// 导出SimpleWebServer类供外部使用
SimpleWebServer* CreateWebServer(Database* db, RecommendationEngine* engine, int port);
void DeleteWebServer(SimpleWebServer* server);

#endif // WEBSERVER_H