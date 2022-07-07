#include<map>
#include<string>
#include "config.h"

class pfr_ipx_int {
 private:
  std::map<std::string, std::map<int, int>> ifi;
  void sql_get_int(Config &);
 public:
  pfr_ipx_int();
  pfr_ipx_int(Config &);
  void add(std::string node, int snmp_idx, int if_idx);
  int get(std::string node, int snmp_idx, int if_idx);
};


class pfr_ipx {
 private:
  std::map<std::string, std::map<int, bool>> pfx;
  std::map<int, std::string> aut;
  std::map<std::string, bool> pfx_dst;
  std::map<int, bool> aut_dst;
  void sql_get_pfx(Config &);
 public:
  pfr_ipx();
  pfr_ipx(Config &);
  void add(std::string prefix, int aut_num, bool dst);
  bool isDst_prefix(std::string pfx);
  bool isDst_aut_num(int aut_num);
  bool isOutput(int ifidx);
};

struct pfr_sql_data {
 class pfr_ipx net;
 class pfr_ipx_int intf;
};
