#include "config.h"

#include <memory>
#include <iostream>
#include <cctype>
#include <cstring>
#include <netdb.h>
#include <unistd.h>


//ERROR: Configurator: Collector failed to start: Failed to parse the configuration: Parser opts aren't set, first must be used fds_xml_set_args


enum {
    MODE,
    PROTOCOL,
    RECONNECT_SECS,
    TEMPLATES_RESEND_SECS,
    TEMPLATES_RESEND_PKTS,
    CONNECTION_BUFFER_SIZE,
    HOSTS,
    HOST,
    NAME,
    ADDRESS,
    PORT,
    PREMADE_CONNECTIONS,
    DBLOGIN,
    DBPASS,
    DBHOST,
    DBPORT,
    MAXDST,
    DBNAME
};

static fds_xml_args params_schema[] = {
    FDS_OPTS_ROOT("params"),
    FDS_OPTS_ELEM(DBLOGIN, "dblogin", FDS_OPTS_T_STRING, 0             ),
    FDS_OPTS_ELEM(DBPASS, "dbpass", FDS_OPTS_T_STRING, 0             ),
    FDS_OPTS_ELEM(DBNAME, "dbname", FDS_OPTS_T_STRING, 0             ),
    FDS_OPTS_ELEM(DBHOST, "dbhost", FDS_OPTS_T_STRING, 0             ),
    FDS_OPTS_ELEM(DBPORT   , "dbport"   , FDS_OPTS_T_UINT  , 0             ),
    FDS_OPTS_ELEM(MAXDST, "maxdst"   , FDS_OPTS_T_UINT  , 0             ),
    FDS_OPTS_END
};

void config_db(void) {
// get cred from start sql
}


void Config::parse_host(fds_xml_ctx_t *host_ctx) {
//DBHostConfig host;
const fds_xml_cont *content;
 while (fds_xml_next(host_ctx, &content) != FDS_EOC) {
    switch (content->id) {
        case DBLOGIN:
            host.dblogin = std::string(content->ptr_string);
            break;
        case DBPASS:
            host.dbpass = std::string(content->ptr_string);
            break;
        case DBNAME:
            host.dbname = std::string(content->ptr_string);
            break;
        case DBHOST:
            host.dbhost = std::string(content->ptr_string);
            break;
        case DBPORT:
            if (content->val_uint > UINT16_MAX) {
                throw std::invalid_argument("ins    valid host port " + std::to_string(content->val_uint));
            }
            host.dbport = static_cast<uint16_t>(content->val_uint);
            break;
        case MAXDST:
            if (content->val_uint > UINT16_MAX) {
                throw std::invalid_argument("ins    valid host port " + std::to_string(content->val_uint));
            }
            maxdst = static_cast<uint16_t>(content->val_uint);
            break;
    }
 }
}

Config::Config(const char *xml_config) {
    //set_defaults();
    auto parser = std::unique_ptr<fds_xml_t, decltype(&fds_xml_destroy)>(fds_xml_create(), &fds_xml_destroy);
    if (!parser) {
        throw std::runtime_error("Failed to create an XML parser!");
    }

    if (fds_xml_set_args(parser.get(), params_schema) != FDS_OK) {
        throw std::runtime_error("Failed to parse the description of an XML document!");
    }

    fds_xml_ctx_t *params_ctx = fds_xml_parse_mem(parser.get(), xml_config, true);
    if (!params_ctx) {
        std::string err = fds_xml_last_err(parser.get());
        throw std::runtime_error("Failed to parse the configuration: " + err);
    }

    try {
        parse_host(params_ctx);
        //ensure_valid();
    } catch (const std::invalid_argument &ex) {
        throw std::runtime_error("Config params error: " + std::string(ex.what()));
    }
}
