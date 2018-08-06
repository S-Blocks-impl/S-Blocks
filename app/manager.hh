#ifndef _CLICK_MANAGER_HH
#define _CLICK_MANAGER_HH
//#include <click/element.hh>
#include <click/atomic.hh>
#include <unistd.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <click/glue.hh>
#include <pthread.h>
//for test
#include <pthread.h>
#include <time.h>
#include <stdio.h>
//for sgx
#include "sgx_urts.h"
#include "enclave_u.h"
#define ENCLAVE_FILE "enclave.signed.so"
#define DEBUG 1
#define Exit_FAILURE 1
#define HAVE_BATCH 1

//for batch 
#include <click/batchelement.hh>


CLICK_DECLS

class Communicate{
private:

public:
    void run(void);
};

 struct controller_ip{
        String ip;
        String port;
    };


class Manager : public BatchElement {
public:
    Manager() CLICK_COLD;
    ~Manager() CLICK_COLD;
    const char *class_name() const { return "Manager"; }
    const char *port_count() const { return PORTS_1_1; }
    //const char *processing() const { return  "PROCESSING_A_AH"; }
    const char *flags() const { return "A"; }

    int configure(Vector<String> &conf, ErrorHandler *errh) CLICK_COLD;
    int initialize(ErrorHandler *) CLICK_COLD;
    //void add_handlers() CLICK_COLD;

#if HAVE_BATCH
    PacketBatch *simple_action_batch(PacketBatch *);
#endif

    Packet *simple_action(Packet *);
    static void* com_module(void*);
    //static String read_handler(Element*, void*) CLICK_COLD;
    //void add_handlers() CLICK_COLD;
    void ocall_to_device(void *packet, int packet_length);
    void ocall_print(const char* data);
    //void push(int,Packet *p);


private:
    //communicate thread
    pthread_t com_thread;
    static Communicate* com;
    
    
    //int num = 22;
    //Vector<Port *> element_in_port;
   // Vector<Port *> element_out_port;
    //Vector<element*> test_elements;
};






CLICK_ENDDECLS
#endif
