#include "server.hpp"

int main() {
    auto server = FasterAPI::Server(8080, 4);
    server.Run();
    return 0;
}
