#include<map>
#include<string>
#include<iostream>
#include<libpq-fe.h>
#include "pfr_ipx.h"
#include "config.h"

/*
class pfr_ipx_int {
 private:
  std::map<std::string, std::map<int, int>> ifi;
 public:
  pfr_ipx_int();
  void add(std::string node, int snmp_idx, int if_idx);
  bool get(std::string node, int snmp_idx, int if_idx);
};


class pfr_ipx {
 private:
  std::map<std::string, std::map<int, bool>> pfx;
  std::map<int, std::string> aut;
  std::map<std::string, bool> pfx_dst;
  std::map<int, bool> aut_dst;
 public:
  pfr_ipx();
  void add(std::string prefix, int aut_num, bool dst);
  bool isDst_prefix(std::string pfx);
  bool isDst_aut_num(int aut_num);
  bool isOutput(int ifidx);
};
*/


pfr_ipx_int::pfr_ipx_int(Config &config) {
    pfr_ipx_int::sql_get_int(config);
}

void pfr_ipx_int::sql_get_int(Config &config) {
    const char  *pghost = config.host.dbhost.c_str(),
                *pgport = std::to_string(config.host.dbport).c_str(),
                *pgoptions = NULL,
                *pgtty = NULL,
                *dbName = config.host.dbname.c_str(),
                *login = config.host.dblogin.c_str(),
                *pwd = config.host.dbpass.c_str();

                PGconn *conn;
                PGresult *res;

    conn = PQsetdbLogin(pghost, pgport, pgoptions, pgtty, dbName, login, pwd);
    if(PQstatus(conn) == CONNECTION_BAD) {
                fprintf(stderr, "Connection to database '%s' failed.\n", dbName);
                        fprintf(stderr, "%s", PQerrorMessage(conn));
                            } else {
#ifdef DEBUG
                                        fprintf(stderr, "Connection to database '%s' Ok.\n", dbName);
#endif
        std::cout << "Connection to database " << config.host.dbname << " was successful" << std::endl;
    }    
    
    const char *q0 = "select  n.ip, si.intf_idx, si.snmp_idx from ipam_addresses a "
                     "join sub_interfaces si on a.interface_id = si.id "
                     "join interface i on i.id = si.parent_id "
                     "join node n on n.id = i.node_id "
                     "join pfr_peers pp on pp.interface_id = si.id "
                     "join pfr_dscp pd on pd.id = pp.dscp_id "
                     "join ipam_vrf vrf on vrf.id = a.vrf_id "
                     "join pfr_peer_group gr on gr.id = pp.peer_group_id order by gr.description;";

    res = PQexec(conn, q0);    
    int ncols = PQnfields(res);
    int nrows = PQntuples(res);
    for(int i = 0; i < nrows; i++) {
      std::string node_ip = PQgetvalue(res, i, 0);
      int intf_idx = std::stoi(PQgetvalue(res, i, 1));
      int snmp_idx = std::stoi(PQgetvalue(res, i, 2));
      ifi[node_ip][intf_idx] = snmp_idx;
      //std::cout << node_ip << ':' << intf_idx << ':' << snmp_idx << ':' << '\n';
    }
    PQclear(res);
    PQfinish(conn);

    for(std::map<std::string, std::map<int, int>>::iterator it0 = ifi.begin(); it0 != ifi.end(); it0++) {
     for(std::map<int, int>::iterator it1 = ifi[it0->first].begin(); it1 != ifi[it0->first].end(); it1++) {
      std::cout << "IFI_PRINT: node_ip: " << it0->first << " : intf_idx: " << it1->first << " : snmp_idx : " << it1->second << std::endl;
    } 
   }
}


void pfr_ipx_int::add(std::string node, int snmp_idx, int if_idx) { ifi[node][if_idx] = snmp_idx; }
int pfr_ipx_int::get(std::string node, int snmp_idx, int if_idx) {
    for(std::map<std::string, std::map<int, int>>::iterator it0 = ifi.begin(); it0 != ifi.end(); it0++) {
     if (it0->first == node) {
        for(std::map<int, int>::iterator it1 = ifi[it0->first].begin(); it1 != ifi[it0->first].end(); it1++) {
            if (it1->first == if_idx) {
                return 1;
            }
        }
     }
    }
    return 0;
}

pfr_ipx::pfr_ipx(Config &config) {
    const char  *pghost = config.host.dbhost.c_str(),
                *pgport = std::to_string(config.host.dbport).c_str(),
                *pgoptions = NULL,
                *pgtty = NULL,
                *dbName = config.host.dbname.c_str(),
                *login = config.host.dblogin.c_str(),
                *pwd = config.host.dbpass.c_str();

                PGconn *conn;
                PGresult *res;

    conn = PQsetdbLogin(pghost, pgport, pgoptions, pgtty, dbName, login, pwd);
    if(PQstatus(conn) == CONNECTION_BAD) {
                fprintf(stderr, "Connection to database '%s' failed.\n", dbName);
                        fprintf(stderr, "%s", PQerrorMessage(conn));
                            } else {
#ifdef DEBUG
                                        fprintf(stderr, "Connection to database '%s' Ok.\n", dbName);
#endif
        std::cout << "Connection to database " << config.host.dbname << " was successful" << std::endl;
    }    
    
    const char *q0 = "select prefix, aut_num, isdst from pfr_ipx;";

    res = PQexec(conn, q0);    
    int ncols = PQnfields(res);
    int nrows = PQntuples(res);
    for(int i = 0; i < nrows; i++) {
      std::string prefix = PQgetvalue(res, i, 0);
      int autn = std::stoi(PQgetvalue(res, i, 1));
      int isdst = std::stoi(PQgetvalue(res, i, 2));
      pfx[prefix][autn] = (bool)isdst;
      pfx_dst[prefix] = (bool)isdst;
      aut_dst[autn] = (bool)isdst;
    }
    PQclear(res);
    PQfinish(conn);

    for(std::map<std::string, std::map<int, bool>>::iterator it0 = pfx.begin(); it0 != pfx.end(); it0++) {
     for(std::map<int, bool>::iterator it1 = pfx[it0->first].begin(); it1 != pfx[it0->first].end(); it1++) {
      std::cout << "PFX_PRINT: prefix: " << it0->first << " : aut_num : " << it1->first << " : isDst : " << it1->second << std::endl;
     } 
    }
    for(std::map<std::string, bool>::iterator it0 = pfx_dst.begin(); it0 != pfx_dst.end(); it0++) {
      std::cout << "PFX_DST_PRINT: prefix: " << it0->first << " : isDst : " << it0->second << std::endl;
    } 
    for(std::map<int, bool>::iterator it0 = aut_dst.begin(); it0 != aut_dst.end(); it0++) {
      std::cout << "AUT_DST_PRINT: aut_num: " << it0->first << " : isDst : " << it0->second << std::endl;
    } 
};

void pfr_ipx::add(std::string prefix, int aut_num, bool dst) { 
    pfx[prefix][aut_num] = dst;
    pfx_dst[prefix] = dst;
    aut_dst[aut_num] = dst;
}

bool pfr_ipx::isDst_prefix(std::string pfx) { 
    auto s = pfx_dst.find(pfx);
    if(s != pfx_dst.end())
        return (bool)pfx_dst[pfx];
    return false; 
}

bool pfr_ipx::isDst_aut_num(int aut_n) { 
    auto s = aut_dst.find(aut_n);
    if(s != aut_dst.end())
        return (bool)aut_dst[aut_n];
    return false; 
}

bool pfr_ipx::isOutput(int ifidx) { return true; }
