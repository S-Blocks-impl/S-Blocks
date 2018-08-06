#ifndef ENCLAVE_DDOS_H
#define ENCLAVE_DDOS_H

//#include "./element_enclave.h"
#include "../lib/error.h"
#include <vector>
#include "../lib/String.h"
#include "./batchelement_enclave.h"

class DDOS:public Element{
public:
    DDOS();
    ~DDOS();
    const char *class_name() const {return "DDOS";}
    const char *port_count() const {return "-/-";}
    const char *processing() const {return PROCESSING_A_AH;}
    
    
    int configure(stlpmtx_std::vector<String> &, ErrorHandler *);
    int initialize(ErrorHandler *);
#if HAVE_BATCH
    void push_batch(int, PacketBatch *);
    PacketBatch* simple_action_batch(PacketBatch *);
#endif
    Packet* simple_action(Packet *);
    void push(int port, Packet *);
    Packet* pull(int port);
#if HAVE_BATCH
    PacketBatch* pull_batch(int, unsigned);
#endif


};
#endif