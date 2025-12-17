#include "WebServer.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

// 跨平台的socket处理
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <cstring>
    #include <arpa/inet.h>
    #define closesocket close
#endif

SimpleWebServer::SimpleWebServer(int port) : port(port), server_socket(INVALID_SOCKET), running(false) {}

SimpleWebServer::~SimpleWebServer() {
    cleanup();
}

void SimpleWebServer::start() {
    if (initialize()) {
        running = true;
        std::cout << "Web服务器启动在端口 " << port << std::endl;
        run();
    }
}

void SimpleWebServer::stop() {
    running = false;
    closesocket(server_socket);
    cleanup();
}

bool SimpleWebServer::initialize() {
#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cout << "WSAStartup失败: " << result << std::endl;
        return false;
    }
#endif

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cout << "创建socket失败" << std::endl;
        cleanup();
        return false;
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cout << "绑定失败" << std::endl;
        closesocket(server_socket);
        cleanup();
        return false;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "监听失败" << std::endl;
        closesocket(server_socket);
        cleanup();
        return false;
    }

    return true;
}

void SimpleWebServer::run() {
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        socket_t client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket != INVALID_SOCKET) {
            std::thread(&SimpleWebServer::handleClient, this, client_socket).detach();
        }
    }
}

void SimpleWebServer::handleClient(socket_t client_socket) {
    char buffer[4096] = {0};
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    
    if (bytes_received > 0) {
        std::string request(buffer, bytes_received);
        std::string response = handleHttpRequest(request);
        send(client_socket, response.c_str(), response.length(), 0);
    }
    
    closesocket(client_socket);
}

std::string SimpleWebServer::handleHttpRequest(const std::string& request) {
    std::istringstream stream(request);
    std::string method, path, version;
    stream >> method >> path >> version;
    
    std::string response;
    
    if (path == "/" || path == "/index.html") {
        response = serveIndexPage();
    } else if (path == "/api/login") {
        response = handleLogin(request);
    } else if (path == "/api/register") {
        response = handleRegister(request);
    } else {
        response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 Not Found";
    }
    
    return response;
}

std::string SimpleWebServer::serveIndexPage() {
    std::string html;
    // 尝试多个可能的路径
    std::vector<std::string> possiblePaths = {
        "index.html",
        "src/index.html",
        "./index.html"
    };
    
    for (const auto& path : possiblePaths) {
        std::ifstream file(path);
        if (file.is_open()) {
            std::ostringstream ss;
            ss << file.rdbuf();
            html = ss.str();
            file.close();
            break;
        }
    }
    
    if (html.empty()) {
        html = "<html><body><h1>Error: Could not load index.html</h1></body></html>";
    }
    
    return "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: " + 
           std::to_string(html.length()) + "\r\n\r\n" + html;
}

std::string SimpleWebServer::handleLogin(const std::string& request) {
    // 简单的登录验证逻辑
    // 这里应该与C++的User类集成
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 50\r\n\r\n{\"success\":true,\"message\":\"Login successful\"}";
}

std::string SimpleWebServer::handleRegister(const std::string& request) {
    // 简单的注册逻辑
    return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 50\r\n\r\n{\"success\":true,\"message\":\"Registration successful\"}";
}

void SimpleWebServer::cleanup() {
    if (server_socket != INVALID_SOCKET) {
        closesocket(server_socket);
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

// 导出SimpleWebServer类供外部使用
SimpleWebServer* CreateWebServer(int port) {
    return new SimpleWebServer(port);
}

void DeleteWebServer(SimpleWebServer* server) {
    delete server;
}