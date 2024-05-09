#include "httpsrv.hpp"

int main() {
    HttpSrv::Run(8080, 4);
    return 0;
}
