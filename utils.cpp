#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "message.pb.h"
#include <atomic>

#include <inttypes.h>

#define BUFF_MAX 8 // 1024 ; 2048 ; 4096 ; 8192

// using namespace std;

extern "C"
{
    // Initialize needed variables and structures
    int server_sockfd, new_sockfd, client_sockfd;
    struct sockaddr_in address;
    struct sockaddr_in serv_addr;

    int listening_socket(int port)
    {
        // Create Socket and returns -1 if it fails
        if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            return -1;
        // printf("Socket created!\n");

        // make address reusable, if used shortly before
        int opt = 1;
        if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
        {
            printf("setsockopt failed!\n");
            return -1;
        }

        // Setting server byte order; declare (local)host IP address;
        // set and convert port number (into network byte order)
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        // Connects socket to server; returns -1 if fails
        if (connect(server_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            return -1;
        // printf("Connected!\n");

        // passing file descriptor, address structure and the lenght
        // of the address structure to bind current IP on the port
        // returns -1 if fails
        if (bind(server_sockfd, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            // printf("Binding failed!\n");
            return -1;
        }

        // lets the socket listen for upto 5 connections
        // return -1 if an error occours
        if (listen(server_sockfd, 5) < 0)
        {
            // printf("Listening failed!\n");
            return -1;
        }

        return server_sockfd;
    }

    int connect_socket(const char *hostname, const int port)
    {
        // Creates socket and returns -1 if fails
        if ((client_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            return -1;
        // printf("Socket created!\n");

        /* seems to be not needed -\(.,.)/-
        // Store host name as uint_32 on AF_INET
        if (inet_pton(AF_INET, hostname, &serv_addr.sin_addr))
            return -1;
        */

        // catch the "localhost" string and convert it to an IP address
        if (std::strcmp(hostname, "localhost") == 0)
            hostname = "127.0.0.1";

        // Store server address and port
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(hostname);
        serv_addr.sin_port = htons(port);

        // printf("Server address: %s (%d)\n", hostname, serv_addr.sin_addr.s_addr);

        // Connects socket to server; returns -1 if fails
        if (connect(client_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            return -1;
        // printf("Connected!\n");

        // return connected socket if successful
        return client_sockfd;
    }

    int accept_connection(int sockfd)
    {
        int addrlen = sizeof(address);

        // accept sockfd and return the new connected socket
        if ((new_sockfd =
                 accept(sockfd,
                        (struct sockaddr *)&address,
                        (socklen_t *)&addrlen)) < 0)
            return -1;
        // printf("Connection accepted!\n");

        return new_sockfd;
    }

    int recv_msg(int sockfd, int32_t *operation_type, int64_t *argument)
    {
        // Initialize message object and needed buffer
        sockets::message message;
        char buffer[BUFF_MAX];
        std::string debugBuffer;

        // read byte stream from socket
        if (read(sockfd, buffer, BUFF_MAX) < 0)
        {
            // printf("read() Failed!\n");
        }

        // Parse transmitted byte stream into a message object
        if (!message.ParseFromArray(buffer, BUFF_MAX))
        {
            // printf("ParseFromString failed\n\n");
        }

        // check if transmitted message is propper formatted
        if (!message.IsInitialized())
        {
            // printf("Message not initialized! (after received)\n");
            return 1;
        }

        // Change the pointers value to transmitted OperationType
        switch (message.type())
        {
        case sockets::message::OperationType::message_OperationType_ADD:
            // printf("-----ADD Received-----\n");
            *operation_type = 1;
            break;
        case sockets::message::OperationType::message_OperationType_SUB:
            // printf("-----SUB Received-----\n");
            *operation_type = 2;
            break;
        case sockets::message::OperationType::message_OperationType_TERMINATION:
            // printf("-TERMINATION Received-\n");
            *operation_type = 3;
            break;
        case sockets::message::OperationType::message_OperationType_COUNTER:
            // printf("---COUNTER Received---\n");
            *operation_type = 4;
            break;
        }

        debugBuffer = message.DebugString();
        // printf("Debug Buffer: \n%s", debugBuffer.c_str());

        // Set transmitted argument
        *argument = message.argument();

        return 0;
    }

    int send_msg(int sockfd, int32_t operation_type, int64_t argument)
    {
        // Initialize message object
        sockets::message message;

        // Set OperationType according to passed variable
        switch (operation_type)
        {
        case 1:
            // printf("-------Send ADD-------\n");
            message.set_type(sockets::message_OperationType_ADD);
            break;
        case 2:
            // printf("-------Send SUB-------\n");
            message.set_type(sockets::message_OperationType_SUB);
            break;
        case 3:
            // printf("---Send TERMINATION---\n");
            message.set_type(sockets::message_OperationType_TERMINATION);
            break;
        case 4:
            // printf("\n-----Send COUNTER-----\n");
            message.set_type(sockets::message_OperationType_COUNTER);
            break;
        }

        // Set argument
        message.set_argument(argument);

        // check if transmitted message is propper formatted
        if (!message.IsInitialized())
        {
            printf("Message not initialized! (Before send)\n");
        }

        // Initialize to transmit byte-arrray
        std::string buffer;
        char buffer_char[BUFF_MAX]; //= {0};
        std::string debugBuffer;

        debugBuffer = message.DebugString();
        // printf("Debug Buffer: \n%s", debugBuffer.c_str());

        // Serialize object to byte array
        message.SerializeToArray(buffer_char, BUFF_MAX);
        
        // Write / send byte array to socket
        write(sockfd, buffer_char, BUFF_MAX);

        return 0;
    }

    void close_socket(int socket)
    {
        // helper method to close a socket
        if (close(socket) == -1)
        {
            // printf("Socket close failed!\n");
        }
        else
        {
            // printf("Socket Closed!\n\n");
        }
    }
}