// Demo_WebSocket.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "WebSocket.h"


void OnReceive(listener::session* pSession, std::string content)
{
    std::cout << "Received message: " << content << std::endl;
    content = "server responses:\n" + content + "!!!";
    pSession->send(content);
}

void OnAccept(listener::session* pSession)
{
    std::cout << "Accept new session: " << std::endl;
    pSession->setCBReceive(OnReceive);
}

int main()
{
    listener* pListener = new listener(OnAccept);
    pListener->run(8080);
    int curPort = pListener->getPort();
    std::cout << "server use port: " << curPort << std::endl;

    bool bLoop = true;
    while (bLoop)
    {
        char ch = getchar();
        // skip ctrl char
        if (ch == '\r' || ch == '\n' || ch == '\t')
            continue;

        switch (ch)
        {
        case 'e':
        case 'E':
            bLoop = false;
            break;
        default:
            break;
        }
    }
}