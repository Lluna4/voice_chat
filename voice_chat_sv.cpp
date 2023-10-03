#undef _WINSOCKAPI_
#define _WINSOCKAPI_
#include <iostream>
#include <Windows.h>
#include <fstream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <vector>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")

int PORT = 5050;
const std::string SERVER_IP = "0.0.0.0";
const int BUFFER_SIZE = 8192 * 2;

std::vector<int> clients;

void manage_server(int socket, int number)
{
    char *buffer = (char *)calloc(BUFFER_SIZE, sizeof(char));
    int bytes_read = 0;
    while (1)
    {
        bytes_read = recv(socket, buffer, BUFFER_SIZE, 0);
        std::cout << bytes_read << std::endl;
        if (bytes_read == -1)
        {
            break;
        }
        if (buffer)
        {
            int bytes_send = send(socket, buffer, BUFFER_SIZE, 0);
            std::cout << "(" << bytes_send << ")" << std::endl;
        }
        memset(buffer, 0, BUFFER_SIZE);
    }
}

int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == 0)
    {
        std::cout << "Se fallo al crear socket" << std::endl;
        return -1;
    }
    address.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP.c_str(), &address.sin_addr);
    address.sin_port = htons(PORT);
    if (bind(sock, (struct sockaddr*)&address, sizeof(address)) < 0)
    {
        std::cout << "Bind ha fallado" << std::endl;
    }
    std::cout << "Escuchando conexiones" << "en "<< PORT << std::endl;
    while (true)
    {
        std::cout << "AAAA" << std::endl;
        listen(sock, 5);
        int new_socket = accept(sock, (struct sockaddr*)&address, &addrlen);
        std::cout << new_socket << std::endl;
        std::thread man_sv(manage_server, new_socket, clients.size());
        clients.push_back(new_socket);
        man_sv.detach();
    }
}