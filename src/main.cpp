#include "httprsp_run.hpp"

int main(int argc, char *argv[]) {
    const std::string root = (argc > 1) ? argv[1] : ".";
    HttpRsp::Run(8080, 4, root);
    return 0;
}
