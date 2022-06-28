#include <atomic>
#include <iostream>
#include <string>
#include <sys/epoll.h>
#include <unistd.h>
#include <thread>

#include "message.pb.h"

#include "utils.h"

std::atomic<int64_t> number{0};
using namespace std;

// #define MAX_EVENTS 5

// struct epoll_event event, events[MAX_EVENTS];
// int epoll_fd = epoll_create1(0), event_count;

int handle_connection(int sockfd);

// Class for using threads, commeted all threads out
class thread_obj
{
public:
    void operator()(int sockfd)
    {
        // printf("Thread for socket %d is handling connection\n\n", sockfd);
        handle_connection(sockfd);
    }
};

int main(int args, char *argv[])
{
    if (args < 3)
    {
        std::cerr << "usage: ./server <numThreads> <port>\n";
        exit(1);
    }

    int numThreads = std::atoi(argv[1]);
    int port = std::atoi(argv[2]);

    // printf("---------Input--------\n numThreads: %d\n port: %d\n-------Input End------\n\n",
    //       numThreads, port);

    number.store(0);
    int server_socket = 0;

    /* if (epoll_fd == -1)
    {
        printf("Failed to create epoll file descriptor\n");
        return 1;
    } */

    // Set server socket listening
    if ((server_socket = listening_socket(port)) < 0)
    {
        printf("listen() failed!\n");
        return 0;
    }
    else
    {
        // printf("Socket %d is listening on %d\n\n", server_socket, port);
    }

    thread threads[numThreads]; // spawn numThreads

    // event.events = EPOLLIN;
    // event.data.fd = 0;

    int sockfd;
    // int threadCounter = 0;

    // "Long lasting" loop for accepting and handling connections
    while (true) // for (int i = 0; i < 1; i++)
    {
        // printf("Waiting for another connection...\n");
        if ((sockfd = accept_connection(server_socket)) < 0)
        {
            // printf("accept_connect() failed!\n");
        }
        else
        {
            // printf("Socket %d is connected!\n\n", sockfd);
        }

        // if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event))
        //    printf("Failed to add Sockfd to epoll!\n");
        // else
        //    printf("Sockfd is added to epoll! \n");

        handle_connection(sockfd);
        /*
        printf("Thread %d is starting:\n", threadCounter + 1);
        threads[threadCounter] = thread(thread_obj(), sockfd);

        [threadCounter].join();

        sockfd = server_socket;
        threadCounter = (threadCounter + 1) % numThreads;
        */
    }

    // if (close(epoll_fd))
    //     printf("Failed to close epoll file descriptor\n");

    close_socket(server_socket);

    return 0;
}

// helper method for handling a socket connection 
int handle_connection(int sockfd)
{
    int32_t operation_type = -1;
    int64_t argument = -1;

    // printf("\nPolling for input...\n");
    // event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 60000);
    // printf("%d ready events\n\n", event_count);

    int last_operation = -1;
    // Loop untill a termination is received
    while (last_operation != 3) // for (int i = 0; i < 3; i++)
    {

        if (recv_msg(sockfd, &operation_type, &argument) == 0)
        {
            // printf("Server received: %d, %lld\n\n",
            //       operation_type, (long long)argument);

            // For exiting the loop
            last_operation = operation_type;

            switch (last_operation)
            {
            case 1:
                number.fetch_add(argument);
                break;
            case 2:
                number.fetch_sub(argument);
                break;
            case 3:
                argument = number.load();
                send_msg(sockfd, 4, argument);
                // printf("Global Counter is: %lld and %lld should have been added\n\n",
                //       (long long)number.load(), (long long)argument);
                printf("%lld\n", (long long)argument);
                fflush(NULL);
                break;
            }
        }
        else
        {
            // printf("receive message failed in handle_connection!\n");
            return 1;
        }
    }
    close_socket(sockfd);
    return 0;
}