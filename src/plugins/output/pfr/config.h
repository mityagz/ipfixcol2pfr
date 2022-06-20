#ifndef CONFIG_H
#define CONFIG_H

#include <ipfixcol2.h>
#include "stdint.h"

#include <libpq-fe.h>
#include <string>

struct DBHostConfig {
    std::string dblogin;
    std::string dbpass;
    std::string dbhost;
    uint16_t dbport;
    std::string dbname;
};


class Config {
    public:
    DBHostConfig host;
    Config() {};
    Config(const char *xml_config);
    
    private:
    void parse_params(fds_xml_ctx_t *params_ctx);
    void parse_hosts(fds_xml_ctx_t *hosts_ctx);
    void parse_host(fds_xml_ctx_t *host_ctx);
    void set_defaults();
};

/** Configuration of a instance of the dummy plugin      */
struct instance_config {
        /** Sleep time                                       */
        struct timespec sleep_time;
};

/**
 *  * \brief Parse configuration of the plugin
 *   * \param[in] ctx    Instance context
 *    * \param[in] params XML parameters
 *     * \return Pointer to the parse configuration of the instance on success
 *      * \return NULL if arguments are not valid or if a memory allocation error has occurred
 *       */
struct instance_config *
config_parse(ipx_ctx_t *ctx, const char *params);

/**
 *  * \brief Destroy parsed configuration
 *   * \param[in] cfg Parsed configuration
 *    */
void
config_destroy(struct instance_config *cfg);

void config_db(void);

#endif // CONFIG_H
