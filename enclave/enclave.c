#include "manager_enclave.h"
#include "enclave_t.h"
#include "./lib/error.h"
static char* conf = NULL;
static Manager_enclave* manager = new Manager_enclave(1);
static ErrorHandler *errh  = new SilentErrorHandler();

int ecall_generat_router()
{
    return manager->generat_router();
}

int ecall_configure(char *conf, int conf_length)
{
    return manager->configure(conf, conf_length, errh);
}

int ecall_initialize()
{
    return manager->initialize(errh);
}

int ecall_command_routerTable()
{
    return manager->command_routerTable();
}

int ecall_destroy()
{
    delete manager;
    return 0;
}

//int ecall_simple_action(void* packet_in, int packet_length, void* time, int time_t_size)
int
ecall_simple_action(void *packet_in, int packet_length)
{
    PRINT("ecall_simple_action");
    WritablePacket *p_in = Packet::make(packet_in,packet_length);
    //p_in->timestamp_anno()
    Packet *tmp = manager->action(p_in);
    //memcpy(packet_in, tmp->data(), tmp->length());
    return 0; //
}


//
int ecall_batch_simple_action(void *packet_in, int packet_length){
    PRINT("ecall_batch_simple_action");
    WritablePacket *p_in = Packet::make(packet_in,packet_length);
    PacketBatch *pbatch_in;
    unsigned max=32;//#define BATCH_MAX_PULL 256 max>0?max:BATCH_MAX_PULL
    MAKE_BATCH(p_in, pbatch_in, max);
    PacketBatch *tmpbatch=manager->batch_action(pbatch_in);
    return 0;
}

