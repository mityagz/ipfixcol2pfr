#include<map>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/shm.h>

#include <ipfixcol2.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "config.h"
#include "../../input/dummy/config.h"
#include "../../../core/message_ipfix.h"


#include "main.h"


/**
 * \file src/plugins/output/viewer/Reader.c
 * \author Jan Kala <xkalaj01@stud.fit.vutbr.cz>
 * \author Lukas Hutak <lukas.hutak@cesnet.cz>
 * \brief Viewer - output module for printing information about incoming packets on stdout (source file)
 * \date 2018
 */
/* Copyright (C) 2018 CESNET, z.s.p.o.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and/or other materials provided with the
*    distribution.
* 3. Neither the name of the Company nor the names of its contributors
*    may be used to endorse or promote products derived from this
*    software without specific prior written permission.
*
* ALTERNATIVELY, provided that this notice is retained in full, this
* product may be distributed under the terms of the GNU General Public
* License (GPL) version 2 or later, in which case the provisions
* of the GPL apply INSTEAD OF those given above.
*
* This software is provided ``as is'', and any express or implied
* warranties, including, but not limited to, the implied warranties of
* merchantability and fitness for a particular purpose are disclaimed.
* In no event shall the company or contributors be liable for any
* direct, indirect, incidental, special, exemplary, or consequential
* damages (including, but not limited to, procurement of substitute
* goods or services; loss of use, data, or profits; or business
* interruption) however caused and on any theory of liability, whether
* in contract, strict liability, or tort (including negligence or
* otherwise) arising in any way out of the use of this software, even
* if advised of the possibility of such damage.
*
*/

#include <iostream>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <libfds.h>
//#include "Reader.h"
#include <ipfixcol2.h>
#include "pfr_collector.h"
#include "pfr_ipx.h"

//      |dst_ip     |tuple5
extern std::map<std::string, tuple5 *> dst_ip;
//      |dst_ip     |doctets
extern std::map<std::string, int> dst_ip_t0;
//      //      |doctets  |dst_ip
extern std::map<int, std::string> tdst_ip;
//      |dst_ip                |src_ip               |dst_port     |src_port     |protocol|doctets
extern std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>> dst_ip_m0;
extern std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>> dst_ip_m1;
//      |dst_ip                |src_ip               |dst_port     |src_port     |protocol|dpkts
extern std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>> dst_ip_mp;

//extern class pfr_ipx ss_ipx;
extern struct pfr_sql_data pdata;
char src_addr[INET6_ADDRSTRLEN];

/*
 class pfr_collector {
  private:
  //      |dst_ip     |tuple5
  std::map<std::string, tuple5 *> dst_ip;
  //      |dst_ip                |src_ip               |dst_port     |src_port     |protocol|doctets
  std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>> dst_ip_m;
  //      |dst_ip                |src_ip               |dst_port     |src_port     |protocol|dpkts
  std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>> dst_ip_mp;
  //      |dst_ip     |doctets
  std::map<std::string, int> dst_ip_t;
  //      |doctets  |dst_ip
  std::map<int, std::string> tdst_ip;
*/

std::map<std::string, tuple5 *> pfr_collector::get_dst_ip() { return dst_ip; }
std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>> pfr_collector::get_dst_ip_m() { return dst_ip_m; }
std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>> pfr_collector::get_dst_ip_mp() { return dst_ip_mp; }
std::map<std::string, int> pfr_collector::get_dst_ip_t() { return dst_ip_t; }
std::map<int, std::string> pfr_collector::get_dst_tdst_ip() { return tdst_ip; }

void pfr_collector::set_dst_ip(std::string d_ip, tuple5 *t5) { dst_ip[d_ip] = t5; }

void pfr_collector::set_dst_ip_m(std::string d_ip, std::string s_ip, int d_port, int s_port, int proto, int doctets) { 
    dst_ip_m[d_ip][s_ip][d_port][s_port][proto] = doctets; 
}

void pfr_collector::set_dst_ip_mp(std::string d_ip, std::string s_ip, int d_port, int s_port, int proto, int dpkts) { 
    dst_ip_m[d_ip][s_ip][d_port][s_port][proto] = dpkts; 
}

void pfr_collector::set_dst_ip_t(std::string d_ip, int doctets) { dst_ip_t[d_ip] = doctets; }
void pfr_collector::set_dst_tdst_ip(int doctets, std::string d_ip) { tdst_ip[doctets] = d_ip; }

char buffer[1024];

const char* pfr_collector::session_src_addr(const struct ipx_session *ipx_desc, char *src_addr, socklen_t size) {
    const struct ipx_session_net *net_desc;
    switch (ipx_desc->type) {
        case FDS_SESSION_UDP:
            net_desc = &ipx_desc->udp.net;
            break;
        case FDS_SESSION_TCP:
            net_desc = &ipx_desc->tcp.net;
            break;
        case FDS_SESSION_SCTP:
            net_desc = &ipx_desc->sctp.net;
            break;
        default:
            return nullptr;
                                                                                    }

        const char *ret;
        if (net_desc->l3_proto == AF_INET) {
            ret = inet_ntop(AF_INET, &net_desc->addr_src.ipv4, src_addr, size);
        } else {
            ret = inet_ntop(AF_INET6, &net_desc->addr_src.ipv6, src_addr, size);
        return ret;
        }
}

void pfr_collector::read_packet(ipx_msg_ipfix_t *msg, const fds_iemgr_t *iemgr) {
    const struct fds_ipfix_msg_hdr *ipfix_msg_hdr;
    ipfix_msg_hdr = (const struct fds_ipfix_msg_hdr*)ipx_msg_ipfix_get_packet(msg);

    if (ipfix_msg_hdr->length < FDS_IPFIX_MSG_HDR_LEN){
        return;
    }

    // Print packet header
    /*
    printf("--------------------------------------------------------------------------------\n");
    printf("IPFIX Message header:\n");
    printf("\tVersion:      %" PRIu16"\n",ntohs(ipfix_msg_hdr->version));
    printf("\tLength:       %" PRIu16"\n",ntohs(ipfix_msg_hdr->length));
    printf("\tExport time:  %" PRIu32"\n",ntohl(ipfix_msg_hdr->export_time));
    printf("\tSequence no.: %" PRIu32"\n",ntohl(ipfix_msg_hdr->seq_num));
    printf("\tODID:         %" PRIu32"\n", ntohl(ipfix_msg_hdr->odid));
    */

    const struct ipx_msg_ctx *msg_ctx = ipx_msg_ipfix_get_ctx(msg);
    pfr_collector::session_src_addr(msg_ctx->session, src_addr, INET6_ADDRSTRLEN);
    //std::cout << "Exporter ip:" << src_addr << std::endl;

    // Get number of sets
    struct ipx_ipfix_set *sets;
    size_t set_cnt;
    ipx_msg_ipfix_get_sets(msg, &sets, &set_cnt);

    // Record counter of total records in IPFIX message
    uint32_t rec_i = 0;

    // Iteration through all the sets
    for (uint32_t i = 0; i < set_cnt; ++i){
        pfr_collector::read_set(&sets[i], msg, iemgr, &rec_i);
    }

    fflush(stdout);
}

void pfr_collector::read_set(struct ipx_ipfix_set *set, ipx_msg_ipfix_t *msg, const fds_iemgr_t *iemgr, uint32_t *rec_i) {
    uint8_t *set_end = (uint8_t *)set->ptr + ntohs(set->ptr->length);
    uint16_t set_id = ntohs(set->ptr->flowset_id);

    const uint32_t rec_cnt = ipx_msg_ipfix_get_drec_cnt(msg);
    const char *set_type = "<unknown>";
    if (set_id == FDS_IPFIX_SET_TMPLT) {
        set_type = "Template Set";
    } else if (set_id == FDS_IPFIX_SET_OPTS_TMPLT) {
        set_type = "Options Template Set";
    } else if (set_id >= FDS_IPFIX_SET_MIN_DSET) {
        set_type = "Data Set";
    }

    /*
    printf("\n");
    printf("Set Header:\n");
    printf("\tSet ID: %" PRIu16" (%s)\n", set_id, set_type);
    printf("\tLength: %" PRIu16"\n", ntohs(set->ptr->length));

    if (set_id == FDS_IPFIX_SET_TMPLT || set_id == FDS_IPFIX_SET_OPTS_TMPLT) {
        // Template set
        struct fds_tset_iter tset_iter;
        fds_tset_iter_init(&tset_iter, set->ptr);
        const char *rec_fmt = (set_id == FDS_IPFIX_SET_TMPLT)
            ? "- Template Record (#%u)\n"
            : "- Options Template Record (#%u)\n";
        unsigned int rec_cnt = 0;

        // Iteration through all templates in the set
        while (fds_tset_iter_next(&tset_iter) == FDS_OK){
            // Read and print single template
            printf(rec_fmt, ++rec_cnt);
            read_template_set(&tset_iter, set_id,iemgr);
            putchar('\n');
        }
        return;
    }
    */

    if (set_id >= FDS_IPFIX_SET_MIN_DSET) {
        // Data set
        struct ipx_ipfix_record *ipfix_rec = ipx_msg_ipfix_get_drec(msg, *rec_i);
        if (ipfix_rec == NULL) return;

        // All the records in the set has same template id, so we extract it from the first record and print it
        //printf("\tTemplate ID: %" PRIu16"\n", ipfix_rec->rec.tmplt->id);
        unsigned int iter_cnt = 0;

        // Iteration through the records which belongs to the current set
        while ((ipfix_rec != NULL) && (ipfix_rec->rec.data < set_end) && (*rec_i < rec_cnt)) {
            // Print record header
            //printf("- Data Record (#%u) [Length: %" PRIu16"]:\n", ++iter_cnt, ipfix_rec->rec.size);
            // Get the specific record and read all the fields
            pfr_collector::read_record(&ipfix_rec->rec, 1, iemgr);
            //putchar('\n');

            // Get the next record
            (*rec_i)++;
            ipfix_rec = ipx_msg_ipfix_get_drec(msg, *rec_i);
        }
        return;
    }

}


void print_indent(unsigned int n) {
    for (unsigned int i = 0; i < n; i++){
        putchar('\t');
    }
}

void pfr_collector::read_record(struct fds_drec *rec, unsigned int indent, const fds_iemgr_t *iemgr) {
    // Iterate through all the fields in record
    struct fds_drec_iter iter;
    struct tuple11 tpl11;
    struct tuple5 tpl5;
    fds_drec_iter_init(&iter, rec, FDS_DREC_PADDING_SHOW);

    int inpt = 2518;
    //std::string rbuf(buffer);
    while (fds_drec_iter_next(&iter) != FDS_EOC) {
        struct fds_drec_field field = iter.field;
        int finfo = field.info->id;
        //  src4addr    dst4addr    srcport dstport proto   input   output  srcas   dstas   doctets dpkts
        //  8           12          7       11      4       10      14      16      17      1       2
        if(finfo == 8 || finfo == 12 || finfo == 7 || finfo == 11 || finfo == 4 || finfo == 10 || finfo == 14 \
                || finfo == 16 || finfo == 17 || finfo == 1 || finfo == 2) {
            pfr_collector::read_field(&field, indent, iemgr, rec->snap);
            std::string rbuf(buffer);
            //std::cout << rbuf << ':';
            switch(finfo) {
                case 8:
                 tpl11.srcaddr = rbuf;
                 break;
                case 12:
                 tpl11.dstaddr = rbuf;
                 break;
                case 7:
                 tpl11.srcport = std::stoi(rbuf);
                 break;
                case 11:
                 tpl11.dstport = std::stoi(rbuf);
                 break;
                case 4:
                 tpl11.proto = std::stoi(rbuf);
                 break;
                case 10:
                 tpl11.input = std::stoi(rbuf);
                 break;
                case 14:
                 tpl11.output = std::stoi(rbuf);
                 break;
                case 16:
                 tpl11.srcas = std::stoi(rbuf);
                 break;
                case 17:
                 tpl11.dstas = std::stoi(rbuf);
                 break;
                case 1:
                 tpl11.doctets = std::stoi(rbuf);
                 break;
                case 2:
                 tpl11.dpkts = std::stoi(rbuf);
                 break;
            }
        }
    }

    if(pdata.intf.get(src_addr, inpt, inpt)) {
      tpl5.srcaddr = tpl11.srcaddr;
      tpl5.dstaddr = tpl11.dstaddr;
      tpl5.srcport = tpl11.srcport;
      tpl5.dstport = tpl11.dstport;
      tpl5.proto = tpl11.proto;
      tpl5.doctets = tpl11.doctets;
      tpl5.dpkts = tpl11.dpkts;
      dst_ip_m0[tpl5.srcaddr][tpl5.dstaddr][tpl5.srcport][tpl5.dstport][tpl5.proto] += tpl5.doctets;
      //dst_ip_t0[tpl5.dstaddr] += tpl5.doctets;
      dst_ip_t0[tpl5.srcaddr + "," + tpl5.dstaddr] += tpl5.doctets;
    }
    
/*
//      |dst_ip     |tuple5
extern std::map<std::string, tuple5 *> dst_ip;
//      |dst_ip     |doctets
extern std::map<std::string, int> dst_ip_t;
//      //      |doctets  |dst_ip
extern std::map<int, std::string> tdst_ip;

 class pfr_collector {
  private:
  //      |dst_ip     |tuple5
  std::map<std::string, tuple5 *> dst_ip;
  //      |dst_ip                |src_ip               |dst_port     |src_port     |protocol|doctets
  std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>> dst_ip_m;
  //      |dst_ip                |src_ip               |dst_port     |src_port     |protocol|dpkts
  std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>> dst_ip_mp;
  //      |dst_ip     |doctets
  std::map<std::string, int> dst_ip_t;
  //      |doctets  |dst_ip
  std::map<int, std::string> tdst_ip;


    //ss_ipx_int.get("10.229.4.0", 5560, 2519);
//      |dst_ip     |tuple5
extern std::map<std::string, tuple5 *> dst_ip;
//      |dst_ip     |doctets
extern std::map<std::string, int> dst_ip_t;
//      //      |doctets  |dst_ip
extern std::map<int, std::string> tdst_ip;

 class pfr_collector {
  private:
  //      |dst_ip     |tuple5
  std::map<std::string, tuple5 *> dst_ip;
  //      |dst_ip                |src_ip               |dst_port     |src_port     |protocol|doctets
  std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>> dst_ip_m;
  //      |dst_ip                |src_ip               |dst_port     |src_port     |protocol|dpkts
  std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>> dst_ip_mp;
  //      |dst_ip     |doctets
  std::map<std::string, int> dst_ip_t;
  //      |doctets  |dst_ip
  std::map<int, std::string> tdst_ip;

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
    */
}

const char * fds_semantic2str(enum fds_ipfix_list_semantics semantic) {
    switch (semantic){
        case (FDS_IPFIX_LIST_ALL_OF):
            return "All of";
        case (FDS_IPFIX_LIST_EXACTLY_ONE_OF):
            return "Exactly one of";
        case (FDS_IPFIX_LIST_ORDERED):
            return "Ordered";
        case (FDS_IPFIX_LIST_NONE_OF):
            return "None of";
        case (FDS_IPFIX_LIST_ONE_OR_MORE_OF):
            return "One or more of";
        default:
            return "Undefined";
    }
}

void pfr_collector::read_field(struct fds_drec_field *field, unsigned int indent, const fds_iemgr_t *iemgr, const fds_tsnapshot_t *snap) {
    // Write info from header about field
    //print_indent(indent);
   // printf("EN: %-*" PRIu32" ID: %-*" PRIu16" ", WRITER_EN_SPACE, field->info->en,
   //     WRITER_ID_SPACE, field->info->id);

    enum fds_iemgr_element_type type;
    const char *org;
    const char *field_name;
    const char *unit;

    // Get the organisation name, field name and unit of the data.
    if (field->info->def == NULL) {
        type = FDS_ET_OCTET_ARRAY;
        field_name = "<unknown>";
        org = "<unknown>";
        unit = "";

        // Try to find the scope
        const struct fds_iemgr_scope *scope_ptr = fds_iemgr_scope_find_pen(iemgr, field->info->en);
        if (scope_ptr != NULL) {
            org = scope_ptr->name;
        }
    } else {
        type = field->info->def->data_type;
        org = field->info->def->scope->name;
        field_name = field->info->def->name;
        if (field->info->def->data_unit != FDS_EU_NONE) {
            unit = fds_iemgr_unit2str(field->info->def->data_unit);
        } else {
            unit = "";
        }
    }

    if (fds_iemgr_is_type_list(type)) {
        // Process lists
        //printf("%*s:%s", WRITER_ORG_NAME_SPACE, org, field_name);
        switch (type) {
        case FDS_ET_BASIC_LIST:
            // Note: header description will be complete in the function
            pfr_collector::read_list_basic(field, indent, iemgr, snap);
            break;
        case FDS_ET_SUB_TEMPLATE_LIST:
            printf(" (subTemplateList, see below)\n");
            //read_list_stl(field, indent, iemgr, snap);
            break;
        case FDS_ET_SUB_TEMPLATE_MULTILIST:
            printf(" (subTemplateMultiList, see below)\n");
            //read_list_stml(field, indent, iemgr, snap);
            break;
        default:
            printf("*Unsupported list type*\n");
            break;
        }

        return;
    }

    //printf("%*s:%-*s : ", WRITER_ORG_NAME_SPACE, org, WRITER_FIELD_NAME_SPACE, field_name);
    // Read and write the data from the field
    //char buffer[1024];
    int res = fds_field2str_be(field->data, field->size, type, buffer, sizeof(buffer));

    if(res >= 0){
        // Conversion was successful
        /*
        if (type == FDS_ET_STRING) {
            printf("\"%s\"", buffer);
        } else if (type == FDS_ET_OCTET_ARRAY) {
            printf("0x%s",buffer);
        } else {
            printf("%s", buffer);
        }

        if (*unit != 0) {
        //    printf(" %s", unit);
        }
        putchar('\n');
        */

        return;
    }
    else if (res == FDS_ERR_BUFFER) {
        // Buffer too small
        printf("<Data is too long to show>\n");
        return;
    }
    else {
        // Any other error
        printf("*Invalid value*\n");
        return;
    }
}

void pfr_collector::read_list_basic(struct fds_drec_field *field, unsigned int indent, const fds_iemgr_t *iemgr, const fds_tsnapshot_t *snap) {
    printf(" (basicList");

    struct fds_blist_iter it;
    fds_blist_iter_init(&it, field, iemgr);

    // Print information about the list -> make sure that the field definition is filled
    int rc = fds_blist_iter_next(&it);
    if (rc != FDS_EOC && rc != FDS_OK) {
        // Malformed
        printf(")\n");
        print_indent(indent);
        printf("  *Malformed data structure: %s*\n", fds_blist_iter_err(&it));
        return;
    }

    uint32_t ie_en = it.field.info->en;
    uint16_t ie_id = it.field.info->id;
    const char *name_scope = "<unknown>";
    const char *name_field = "<unknown>";
    if (it.field.info->def != NULL) {
        // Definition exists
        name_scope = it.field.info->def->scope->name;
        name_field = it.field.info->def->name;
    } else {
        // Definition not found... try to find the scope
        const struct fds_iemgr_scope *scope_ptr = fds_iemgr_scope_find_pen(iemgr, ie_en);
        if (scope_ptr != NULL) {
            name_scope = scope_ptr->name;
        }
    }

    printf(", List Semantic: %s)\n", fds_semantic2str(it.semantic));
    //print_indent(indent);
    //printf("> List fields: EN:%*"PRIu32" ID:%*"PRIu16" %*s:%-*s\n",
    //    WRITER_EN_SPACE, ie_en, WRITER_ID_SPACE, ie_id,
    //    WRITER_ORG_NAME_SPACE, name_scope, WRITER_FIELD_NAME_SPACE, name_field);

    // Re-initialize the iterator
    fds_blist_iter_init(&it, field, iemgr);
    unsigned int cnt_value = 0;
    bool more_values = true;

    while (more_values) {
        int rc = fds_blist_iter_next(&it);
        switch (rc) {
        case FDS_OK:  // Process the record
            break;
        case FDS_EOC: // End of the list
            more_values = false;
            continue;
        case FDS_ERR_FORMAT: // Something is wrong with the record
            printf("*Unable to continue due to malformed data: %s*\n", fds_blist_iter_err(&it));
            return;
        default:
            printf("*Internal error: fds_blist_iter_next(): unexpected return code*\n");
            return;
        }

        read_field(&it.field, indent + 1, iemgr, snap);
        cnt_value++;
    }

    /*
    if (cnt_value == 0) {
        print_indent(indent + 1);
        printf("EN: %-*" PRIu32" ID: %-*" PRIu16" ", WRITER_EN_SPACE, ie_en, WRITER_ID_SPACE, ie_id);
        printf("%*s:%-*s : ", WRITER_ORG_NAME_SPACE, name_scope, WRITER_FIELD_NAME_SPACE, name_field);
        printf("<empty>\n");
    }
    */
}

