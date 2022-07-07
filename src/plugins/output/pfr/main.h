#include <string>
#include <map>


struct tuple5 {
    std::string srcaddr;
    std::string dstaddr;
    int srcport;
    int dstport;
    int proto;
    int srcas;
    int dstas;
    int doctets;
    int dpkts;
};

struct tuple11 {
    std::string srcaddr;
    std::string dstaddr;
    int srcport;
    int dstport;
    int proto;
    int srcas;
    int dstas;
    int input;
    int output;
    int doctets;
    int dpkts;
};
