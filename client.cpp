#include <iostream>
#include <string>

#include <sys/epoll.h>
#include <unistd.h>
#include <thread>
#include "utils.h"

using namespace std;

int handle_connection(string hostname, int port, int numMessages, int add, int sub);

int main(int args, char *argv[])
{
    if (args < 7)
    {
        std::cerr << "usage: ./client <num_threads> <hostname> <port> "
                     "<num_messages> <add> <sub>\n";
        exit(1);
    }

    int numClients = std::atoi(argv[1]);
    std::string hostname = argv[2];
    int port = std::atoi(argv[3]);
    int numMessages = std::atoi(argv[4]);
    int add = std::atoi(argv[5]);
    int sub = std::atoi(argv[6]);

    // printf("---------Input--------\n numClients: %d\n hostname: %s\n port: %d\n numMessages: %d\n add: %d\n sub: %d\n-------Input End------\n\n",
    //         numClients, hostname.c_str(), port, numMessages, add, sub);

    for (int i = 0; i < numClients; i++) {
        handle_connection(hostname, port, numMessages, add, sub);
    }
    /*
    thread threads[numClients]; // spawn numThreads

    for (int i = 0; i < numClients; i++)
    {
        // printf("Thread %d is starting!\n", i + 1);
        threads[i] = thread(handle_connection,
                            hostname, port, numMessages, add, sub);

        threads[i].join();
    }
    */
}

int handle_connection(string hostname, int port, int numMessages, int add, int sub)
{
    int client_socket = 0;
    int debug_Count = 0;

    if ((client_socket = connect_socket(hostname.c_str(), port)) < 0)
    {
        // printf("Connect failed!\n");
    }
    else
    {
        // printf("Socket %d is connected!\n\n", client_socket);

        // printf("-----Start Sending----\n");
        // alternating loop for add and sub
        for (int i = 0; i < numMessages; i++)
        {
            if (i % 2 == 0)
            {
                send_msg(client_socket, 1, add);
                debug_Count += add;
            }
            else
            {
                send_msg(client_socket, 2, sub);
                debug_Count -= sub;
            }
        }

        send_msg(client_socket, 3, debug_Count);
        // printf("---Termination Sent---\n\n");

        int32_t operation_type = -1;
        int64_t argument = -1;

        if (recv_msg(client_socket, &operation_type, &argument) == 0 && operation_type == 4)
        {
            // printf("\nClient Counter is: %lld\n\n", (long long)argument);
            printf("%lld\n", (long long)argument);
            fflush(NULL);
        }
        else
        {
            // printf("Error!\n");
        }
    }

    close_socket(client_socket);

    return 0;
}