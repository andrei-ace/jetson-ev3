#include "ping.capnp.h"
#include <capnp/ez-rpc.h>
#include <capnp/message.h>
#include <iostream>
#include "ev3dev.h"

class PongEv3Server final: public Ev3Ping::Server {
  public:
    ::kj::Promise<void> ping(PingContext context) override {
      auto message = context.getParams().getPing();
      std::cout << message.cStr() << std::endl;
      ev3dev::sound::speak(message.cStr(), true);
      context.getResults().setValue("Pong!");
      return kj::READY_NOW;
    }
};


int main(int argc, const char* argv[]) {
  // We expect one argument specifying the address to which
  // to bind and accept connections.
  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " ADDRESS[:PORT]"
              << std::endl;
    return 1;
  }

  // Set up the EzRpcServer, binding to port 5923 unless a
  // different port was specified by the user.  Note that the
  // first parameter here can be any "Client" object or anything
  // that can implicitly cast to a "Client" object.  You can even
  // re-export a capability imported from another server.
  capnp::EzRpcServer server(kj::heap<PongEv3Server>(), argv[1], 5923);
  auto& waitScope = server.getWaitScope();

//   // Run forever, accepting connections and handling requests.
  kj::NEVER_DONE.wait(waitScope);
}