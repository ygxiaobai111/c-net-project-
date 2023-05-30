#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <arpa/inet.h>
// 经历了10天终于做出来了
//运行于linux系统
struct ClientData {
    int clientSocket;
    sockaddr_in clientAddr;
};

std::map<int, std::vector<int> > roomClients;

void writeLog(const std::string& message) {
    std::ofstream logFile("log.txt", std::ios::app);
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    }
}

void broadcastMessage(const std::string& message, const std::vector<int>& clients) {
    for (std::vector<int>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
        int clientSocket = *it;
        send(clientSocket, message.c_str(), message.length(), 0);
    }
}

void* handleClient(void* arg) {
    ClientData* clientData = static_cast<ClientData*>(arg);
    int clientSocket = clientData->clientSocket;
    sockaddr_in clientAddr = clientData->clientAddr;

    char buffer[1024] = { 0 };
    int room = -1;

    // 接收客户端发送的房间号
    if (recv(clientSocket, (char*)&room, sizeof(room), 0) <= 0) {
        // 接收失败或客户端断开连接
        close(clientSocket);
        delete clientData;
        return NULL;
    }

    // 将客户端添加到房间
    roomClients[room].push_back(clientSocket);

    while (true) {
        // 接收客户端发送的消息
        int recvResult = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (recvResult <= 0) {
            // 接收失败或客户端断开连接
            break;
        }

        // 将消息发送给同一房间的其他客户端
        const std::vector<int>& clients = roomClients[room];
        broadcastMessage(buffer, clients);

        // 添加到日志
        std::ostringstream oss;
        oss << "Room " << room << " - From " << inet_ntoa(clientAddr.sin_addr) << ": " << buffer;
        writeLog(oss.str());
    }

    // 从房间中移除客户端
    std::vector<int>& clients = roomClients[room];
    clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());

    // 关闭客户端连接
    close(clientSocket);

    // 释放内存
    delete clientData;

    return NULL;
}

int main() {
    // 创建 socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "socket 创建失败" << std::endl;
        return 1;
    }

    // 绑定地址和端口
    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12345); // 使用12345端口

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "绑定地址和端口失败" << std::endl;
        close(serverSocket);
        return 1;
    }

    // 监听连接
    if (listen(serverSocket, 5) < 0) {
        std::cerr << "监听失败" << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "服务器已启动，等待连接..." << std::endl;

    while (true) {
        // 接受客户端连接
        sockaddr_in clientAddr = { 0 };
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            std::cerr << "接受连接失败" << std::endl;
            continue;
        }

        // 创建客户端线程
        ClientData* clientData = new ClientData;
        clientData->clientSocket = clientSocket;
        clientData->clientAddr = clientAddr;

        pthread_t clientThread;
        if (pthread_create(&clientThread, NULL, handleClient, clientData) != 0) {
            std::cerr << "创建线程失败" << std::endl;
            delete clientData;
            close(clientSocket);
        }

        std::cout << "客户端连接成功" << std::endl;
    }

    // 关闭服务器 socket
    close(serverSocket);

    return 0;
}
