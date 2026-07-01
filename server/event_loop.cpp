#include "server.h"

struct sockaddr_storage their_addr;
//* socklen_t addr_size = sizeof their_addr;
// it is best to reset it every time, coz accept() can modify addr_size

void event_loop(int sockfd, unordered_map<string, string>& db)
{
    // * epoll instance
    int epfd = epoll_create1(0);

    if (epfd == -1)
    {
        perror("epoll_create1");
        // return 2;
        // exit(1);
        return;
    }

    int maxEvents = 10;
    // epoll_event events[maxEvents];
    vector<epoll_event> events(maxEvents);

    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev) == -1)
    {
        perror("epoll_ctl");
        // return 2;
        // exit(1); // or just return     [return will give back the control to main to handle the erro(it closes the connection), & exit just tells the OS to kill it without involving main]
        return; // let main do it. it'll clean up the sockets properly
    }

    while (1)
    {

        // int num_of_ready_fds = epoll_wait(epfd, &events, maxEvents, 10);
        int num_of_ready_fds = epoll_wait(epfd, events.data(), maxEvents, 10); // since it wants a pointer to first `epoll_event`; so vec.data() returns pointer to its first element

        if (num_of_ready_fds == -1)
        {
            perror("epoll_wait");
            break;
        }

        connect_to_client(sockfd, epfd, num_of_ready_fds, their_addr, events, db);

    }
}