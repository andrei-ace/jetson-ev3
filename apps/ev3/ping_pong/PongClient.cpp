#include "ping.capnp.h"
#include <capnp/ez-rpc.h>
#include <capnp/message.h>
#include <iostream>

int main(int argc, const char *argv[])
{
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " HOST:PORT\n"
            "Connects to the Pong server at the given address and "
            "does some RPCs." << std::endl;
        return 1;
    }
    capnp::EzRpcClient client(argv[1]);
    Ev3Ping::Client ping = client.getMain<Ev3Ping>();
    auto& waitScope = client.getWaitScope();
    {
        std::cout << "Sending ping... ";
        std::cout.flush();

        auto request = ping.pingRequest();
        request.setPing("Ping");

        // Send it, which returns a promise for the result (without blocking).
        auto pingPromise = request.send();

        auto response = pingPromise.wait(waitScope);
        std::cout << response.getValue().cStr() << std::endl;
    }
    return 0;

}