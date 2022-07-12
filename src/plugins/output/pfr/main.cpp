/**
 * \file src/plugins/output/forwarder/src/main.cpp
 * \author Michal Sedlak <xsedla0v@stud.fit.vutbr.cz>
 * \brief Main plugin entry point
 * \date 2021
 */

/* Copyright (C) 2021 CESNET, z.s.p.o.
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


#include <ipfixcol2.h>
#include <cstdio>
#include <memory>
#include <map>
#include <vector>
#include <iostream>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <semaphore.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h> 
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "main.h"
#include "pfr_ipx.h"
#include "pfr_collector.h"
#include "config.h"

//      |dsp_ip     |tuple5
std::map<std::string, tuple5 *> dst_ip;
//      |dsp_ip     |doctets
std::map<std::string, int> dst_ip_t0;
std::map<std::string, int> *dst_ip_t0_ptr;
//      |doctets  |dsp_ip
std::map<int, std::string, std::greater<int>> tdst_ip;

//      |dst_ip                |src_ip               |dst_port     |src_port     |protocol|doctets
std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>> dst_ip_m0;
std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>> dst_ip_m1;
//      |dst_ip                |src_ip               |dst_port     |src_port     |protocol|dpkts
std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>> dst_ip_mp;


/// Plugin definition
IPX_API struct ipx_plugin_info ipx_plugin_info = {
    "pfr",
    "Provides data for pfr.",
    IPX_PT_OUTPUT,
    0,
    "0.0.1",
    "2.2.0"
};

const char *gxmlcfg;
struct pfr_sql_data pdata;
int sync_shm_s = 0;
int sync_shm_c = 0;
int sync_shm_new_data = 0;

const char *key_sem0 = "/key_sem0";
sem_t *sem0;

const char *shm_key_p0 = "/tmp/key0_shm0";
key_t shm_key0;
int shm_id = 0;
void *shm_addr;


void sig_proc(int sig_num) {
 signal(sig_num,sig_proc);
 alarm(600);
#ifdef DEBUG
 printf("alarm\n");
#endif
 sync_shm_s = 1;
}


int ipx_plugin_init(ipx_ctx_t *ctx, const char *xml_config) {
        gxmlcfg = xml_config;
        Config config(xml_config);
        pfr_ipx s_ipx(config);
        pfr_ipx_int s_ipx_int(config);

        pdata.net = s_ipx;
        pdata.intf = s_ipx_int;

        signal(SIGALRM, sig_proc);
        alarm(600);

        sem_unlink(key_sem0);
        do {
            sem0 = sem_open(key_sem0, O_CREAT|O_EXCL, S_IRWXU, 1);
            std::cout << "sem_open error: " << errno << std::endl;
        } while ((sem0 == SEM_FAILED) && (errno == EEXIST));
        std::cout << "sem_open: " << sem0 << std::endl;
        
        shm_key0 = ftok(shm_key_p0, 1);
        if(shm_key0 == -1) {
            perror("ftok error");
            exit(1);
        }
        shm_id = shmget(shm_key0, 0, 0);
        shmctl(shm_id, IPC_RMID, 0);
        shm_id = 0;

        std::cout << "ftok: " << shm_key0 << std::endl;
        
        std::cout << s_ipx_int.get("10.229.6.0", 3, 1) << std::endl;
        std::cout << s_ipx_int.get("10.229.4.0", 3, 1) << std::endl;
        std::cout << s_ipx_int.get("10.229.4.0", 2694, 2518) << std::endl;
        std::cout << s_ipx_int.get("10.229.4.0", 5560, 2519) << std::endl;
        std::cout << s_ipx_int.get("10.229.4.0", 1198, 407) << std::endl;
        std::cout << s_ipx_int.get("10.229.132.0", 1198, 407) << std::endl;

        std::cout  << std::endl;

        std::cout << s_ipx.isDst_prefix("10.229.132.0") << std::endl;
        std::cout << s_ipx.isDst_prefix("60.128.0.0/15") << std::endl;
        std::cout << s_ipx.isDst_prefix("6.128.0.0/15") << std::endl;
        std::cout << s_ipx.isDst_aut_num(3333) << std::endl;
        std::cout << s_ipx.isDst_aut_num(333) << std::endl;
        std::cout << s_ipx.isDst_aut_num(3333) << std::endl;

    return IPX_OK;
}

void ipx_plugin_destroy(ipx_ctx_t *ctx, void *priv) {
    (void) ctx;

    /*
    Forwarder *forwarder = (Forwarder *) priv;
    delete forwarder;
    */
}

int ipx_plugin_process(ipx_ctx_t *ctx, void *priv, ipx_msg_t *msg) {

    int type = ipx_msg_get_type(msg);
    if (type != IPX_MSG_IPFIX) {
       return IPX_OK;
    }

    const fds_iemgr_t *iemgr = ipx_ctx_iemgr_get(ctx);

    //Convert the message to the IPFIX message and read it
    ipx_msg_ipfix_t *ipfix_msg = ipx_msg_base2ipfix(msg);
    pfr_collector::read_packet(ipfix_msg, iemgr);
    
    /*
    if(sync_shm_s) {
     for(std::map<std::string, int>::iterator it0 = dst_ip_t0.begin(); it0 != dst_ip_t0.end(); it0++) {
      std::string dstaddr = it0->first;
      int doctets = it0->second;
      std::cout << "DSP_IP_M0: " << dstaddr << ":" << doctets << std::endl;
     }
     std::cout << "-----------------------------------------------: " <<  std::endl;
     sync_shm_s = 0;
    }
    */

     int dst_ip_t0_s = 0;
     int dst_ip_size = 0;
     char *dst_ip_str;
     std::string dstaddr;
     char *ret_shm_addr;

     if(sync_shm_s) {
        std::cout << "sync_shm_s: Trying!" << std::endl;
        if(sem_trywait(sem0) == 0) {
          std::cout << "SEM0: Locked!" << std::endl;
          std::cout << "dst_ip_t0 sizeof: " << dst_ip_t0.size() << std::endl;
          int dst_ip_size = dst_ip_t0.size();

        if(dst_ip_t0.size() > 0) {
            for(std::map<std::string, int>::iterator it0 = dst_ip_t0.begin(); it0 != dst_ip_t0.end(); it0++) {
                dstaddr += it0->first + ':';
                tdst_ip[it0->second] = it0->first;
            }

            /*
            for(std::map<int, std::string>::iterator it0 = tdst_ip.begin(); it0 != tdst_ip.end(); it0++) {
                dstaddr += it0->second + ':';
            }
            */
            
            dst_ip_str = (char *)dstaddr.c_str();
            std::cout << "DSP_IP_STR: " << dst_ip_str << std::endl;
            int dst_ip_str_len = strlen(dst_ip_str);
            //dst_ip_t0_s = sizeof(dst_ip_t0) + dst_ip_t0.size() * (sizeof(decltype(dst_ip_t0)::key_type) + sizeof(decltype(dst_ip_t0)::mapped_type));

            std::cout << "shm_id0: " << shm_id << std::endl;
            if(shm_id == 0) {
                shm_id = shmget(shm_key0, dst_ip_str_len + 1, IPC_CREAT);
                if(shm_id == -1) {
                    perror("Shared memory 0");
                    exit(1);
                }
            }
            else if(shm_id > 0) {
                std::cout << "shm_id1: " << shm_id << std::endl;
                shmdt(shm_addr);
                shmctl(shm_id, IPC_RMID, 0);
                shm_id = shmget(shm_key0, dst_ip_str_len + 1, IPC_CREAT|IPC_EXCL);
                if(shm_id == -1) {
                    perror("Shared memory 1");
                    exit(1);
                }
            }
            shm_addr = shmat(shm_id, NULL, 0);
            if(shm_addr == (void *) -1) {
                perror("Shared memory attach");
                exit(1);
            }
            std::cout << "shm_addr: " << shm_addr << std::endl;
            memset(shm_addr, 0, dst_ip_str_len);
            ret_shm_addr = (char *)std::memcpy(shm_addr, (void *) dst_ip_str, dst_ip_str_len + 1);
            std::cout << "ret_shm_addr: " << ret_shm_addr << std::endl;
            std::cout << "-----------------------------------------------: " <<  std::endl;
            shmdt(shm_addr);
            
            // free map
            for(std::map<std::string, int>::iterator it0 = dst_ip_t0.begin(); it0 != dst_ip_t0.end(); it0++) {
                dst_ip_t0.erase(it0); 
            }

            for(std::map<int, std::string>::iterator it0 = tdst_ip.begin(); it0 != tdst_ip.end(); it0++) {
                tdst_ip.erase(it0); 
            }
        }

        }

        if(sem_post(sem0) == 0) {
          std::cout << "SEM0: UnLocked!" << std::endl;
        }
     sync_shm_new_data = 1;
     sync_shm_s = 0;
    } 

    /*
    for(std::map<std::string, std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>>::iterator it0 = dst_ip_m0.begin(); it0 != dst_ip_m0.end(); it0++) {
     std::string srcaddr = it0->first;
     for(std::map<std::string, std::map<int, std::map<int, std::map<int, int>>>>::iterator it1 = dst_ip_m0[srcaddr].begin(); it1 != dst_ip_m0[srcaddr].end(); it1++) {
      std::string dstaddr = it1->first;
      for(std::map<int, std::map<int, std::map<int, int>>>::iterator it2 = dst_ip_m0[srcaddr][dstaddr].begin(); it2 != dst_ip_m0[srcaddr][dstaddr].end(); it2++) {
      int srcport = it2->first;
       for(std::map<int, std::map<int, int>>::iterator it3 = dst_ip_m0[srcaddr][dstaddr][srcport].begin(); it3 != dst_ip_m0[srcaddr][dstaddr][srcport].end(); it3++) {
       int dstport = it3->first;
        for(std::map<int, int>::iterator it4 = dst_ip_m0[srcaddr][dstaddr][srcport][dstport].begin(); it4 != dst_ip_m0[srcaddr][dstaddr][srcport][dstport].end(); it4++) {
        int proto = it4->first;
        int doctets = it4->second;
        std::cout << "DSP_IP_M0: " << srcaddr << ":" << dstaddr << ":" <<srcport << ":" << dstport << ":" << proto << ":" << doctets << std::endl;
        } 
       }
      }
     }
    }
    */

    return IPX_OK;

}
