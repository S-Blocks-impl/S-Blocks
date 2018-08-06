#include "../include/config.h"
#include "./ddos.h"
#include "../lib/packet.h"
#include "../enclave_t.h"

DDOS::DDOS()
{
}
DDOS::~DDOS()
{
    }

#if HAVE_BATCH
PacketBatch *
DDOS::simple_action_batch(PacketBatch *head)
{
    return head;
}
#endif

Packet* 
DDOS::simple_action(Packet *p)
{
    return p;
}

void
DDOS::push(int port, Packet *p)
{
    PRINT("DDOS::push");
    if(port == 0)
    {
        unsigned char* data =const_cast<unsigned char*>(p->data());
        data[0] = 0xf0;
        //output(0).push(p);
        ocall_to_device(const_cast<unsigned char*>(p->data()),p->length());
    }
    else if(port == 1)
    {
        unsigned char* data =const_cast<unsigned char*>(p->data());
        data[0] = 0xf1;
        //output(0).push(p);
        ocall_to_device(const_cast<unsigned char*>(p->data()),p->length());
    }
    else if(port == 2)
    {
        unsigned char* data =const_cast<unsigned char*>(p->data());
        data[0] = 0xf2;
        //output(0).push(p);
        ocall_to_device(const_cast<unsigned char*>(p->data()),p->length());
    }
    else if(port == 3)
    {
        unsigned char* data =const_cast<unsigned char*>(p->data());
        data[0] = 0xf3;
        //output(0).push(p);
        ocall_to_device(const_cast<unsigned char*>(p->data()),p->length());
    }
    else if(port == 4)
    {
        unsigned char* data =const_cast<unsigned char*>(p->data());
        data[0] = 0xf4;
        //output(0).push(p);
        ocall_to_device(const_cast<unsigned char*>(p->data()),p->length());
    } 
    return;
}


#if HAVE_BATCH
void
DDOS::push_batch(int, PacketBatch * batch)
{
    PRINT("DDOS::push_batch");
    return ;

}

PacketBatch*
DDOS::pull_batch(int port, unsigned max)
{
    PacketBatch *batch;
    PRINT("DDOS::pull_batch");
    return batch;

}
#endif

Packet*
DDOS::pull(int port)
{
    if(port == 0)
        return input(0).pull();
    else if(port == 1)
        return input(1).pull();
    else
        return NULL;
}

        
 int 
DDOS::configure(stlpmtx_std::vector<String> &, ErrorHandler *)
{}

 int 
DDOS::initialize(ErrorHandler *)
{}

        



