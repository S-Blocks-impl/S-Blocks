#ifndef _CLICK_MANAGER_HH
#define _CLICK_MANAGER_HH

#include <click/element.hh>
#include <click/atomic.hh>
#include <unistd.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <click/glue.hh>
//for test
// #include <pthread.h>
// #include <time.h>
// #include <stdio.h>
// //for sgx
// #include "sgx_urts.h"
// #include "Enclave_u.h"
// #define ENCLAVE_FILE "enclave.signed.so"
// #define DEBUG


CLICK_DECLS
class Manager : public Element 
{
public:
    Manager() CLICK_COLD;
    ~Manager() CLICK_COLD;
    
    const char* class_name const { return "Manager"; }
    const char* port_count const { return PORTS_1_1; }
    const char* flags const { return "A"; }
    
    int configure(Vector<String> &conf, ErrorHandler *errh) CLICK_COLD;
    
    int initialize(ErrorHandler *) CLICK_COLD;
    Packet *simple_action(Packet *) CLICK_COLD;

    /* later
    void add_handlers() CLICK_COLD;
    static String read_handler(Element*, void*) CLICK_COLD;
    String get_state();
    int set_state(String s);
    int add_rule(string a_r);
    int delete_rule(string d_r);
    int copy_instance(string ip_port);
    */

    /*static void* com_module(void*);
    
    struct controller_ip{
        String ip;
        String port;
    };

private:
    //communicate
    static Communicate* com;
    pthread_t com_thread;

    //enclave
    sgx_enclave_id_t eid;
    sgx_launch_token_t token = {1234};
*/


/*};
class Communicate
{
public:
    void run(void);
private:
};*/


CLICK_ENDDECLS
#endif