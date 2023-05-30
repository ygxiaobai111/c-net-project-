#include <iostream>
#include <string>
#include <winsock2.h>
#include <limits>
#undef max
#pragma comment(lib, "ws2_32.lib")
const int BUFFER_SIZE = 1024;
//客户端程序
int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize winsock" << std::endl;
        return 1;
    }

    // 创建 socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        WSACleanup();
        return 1;
    }

    // 输入服务器 IP 和端口
    std::string serverIP;
    std::cout << "请输入服务器 IP 地址: ";
    std::cin >> serverIP;

    int serverPort;
    std::cout << "请输入服务器端口号: ";
    std::cin >> serverPort;

    // 连接服务器
    sockaddr_in serverAddr = { 0 };
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());
    serverAddr.sin_port = htons(serverPort);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to the server" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // 输入房间号
    int room;
    std::cout << "请输入房间号: ";
    std::cin >> room;

    // 清除输入缓冲区中的换行符
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // 发送房间号到服务器
    send(clientSocket, (char*)&room, sizeof(room), 0);

    // 等待服务器的确认消息
    char buffer[BUFFER_SIZE] = { 0 };




    // 开始聊天
    std::cout << "您已进入房间，可以开始聊天（输入 q 退出）" << std::endl;

    while (true) {
        std::string message;
        std::cout << "请输入消息: ";
        std::getline(std::cin, message);

        if (message == "q") {
            break;
        }

        // 发送消息到服务器
        send(clientSocket, message.c_str(), message.length(), 0);
        std::cout << "发送成功"<<std::endl;
        // 接收服务器返回的消息
        memset(buffer, 0, sizeof(buffer));
        recv(clientSocket, buffer, sizeof(buffer), 0);

        std::cout << "服务器回复: " << buffer << std::endl;
    }

    // 关闭客户端 socket
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
