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
