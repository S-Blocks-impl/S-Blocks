#include <click/config.h>
#include <click/args.hh>
#include <click/error.hh>
#include <iostream>
#include <ctime>

using namespace std;

#include "manager.hh"

//#define ECALL_CONF_SIZE 128
//#define _conf_size(n) (n + ECALL_CONF_SIZE -1) & ~(ECALL_CONF_SIZE -1) //对齐ECALL_CONF_SIZE

CLICK_DECLS

Communicate* Manager::com = new Communicate();
sgx_enclave_id_t eid;
Manager::Manager()
{
  printf("Manager::Manager()\n");
}

Manager::~Manager()
{
    
    printf("Manager::~Manager()\n");
    if(sgx_destroy_enclave(eid) != SGX_SUCCESS)
    {
        printf("error: destroying enclave!\n");
    }
    
    //sleep(10);
    //printf("Manager::pthread_join start\n");
    //pthread_join(com_thread,NULL);
    //printf("Manager::pthread_join end\n");
}

int
Manager::configure(Vector<String> &conf, ErrorHandler* errh)
{
    //创建connection实例，初始化socket链接
    printf("Manager::configure\n");
    struct controller_ip c_ip;
    Args(conf, this, errh)
        .read("ip", c_ip.ip)
        .read("port", c_ip.port)
        .complete();
      /*  
    if(pthread_create(&com_thread, NULL, Manager::com_module, &c_ip))
    {
        printf("Manager::error: creating com_module thread!\n");
        exit(Exit_FAILURE);
    }
    */

    //create enclave
    
    sgx_launch_token_t token = {12};
    sgx_status_t ret = SGX_SUCCESS;
    int update = 0;
    ret = sgx_create_enclave(ENCLAVE_FILE, DEBUG, &token, &update, &eid, NULL);
    if(ret != SGX_SUCCESS)
    {
        printf("Manager::error: creating enclave!\n");
        exit(Exit_FAILURE);
    }
    printf("Manager::ok: creating enclave!\n");
    
    //create router
    int generate_ret = 0;
    ret = ecall_generat_router(eid, &generate_ret);
    if(ret != SGX_SUCCESS || generate_ret != 1)
    {
        printf("Manager::error: generat_router!\n");
        exit(Exit_FAILURE);
    }
    printf("Manager::ok: generat_router!\n");

    //enclave configure
    //char *conf 组成：
    //（int）length1 (char*) conf1 ('/0') 
    //（int）length2 (char*) conf2 ('/0') 
/*
    //test
    conf.clear();
    conf.push_back(String("12/0800.-"));
    conf.push_back(String("123"));
    //
    int conf_size = 0;
    char* conf_begin = NULL;
    char* conf_ptr = NULL;
    for(int i = 0; i < conf.size(); i++)
        conf_size += conf[i].length() + 1 + sizeof(int);
    conf_begin = (char *)malloc(conf_size);
    
    if(conf_begin == NULL)
    {
        printf("Manager::error: malloc for conf !\n");
        exit(Exit_FAILURE);
    }
    conf_ptr = conf_begin;
    for(int i = 0; i < conf.size(); i++)
    {
        int length =  conf[i].length();
        *(int*)conf_ptr = length;
        conf_ptr += sizeof(int);
        memcpy(conf_ptr, conf[i].data(),length+1);
        if(i != conf.size()-1)
            conf_ptr += length+1+sizeof(int);        
    }
    */

    int conf_size = 0;
    char* conf_begin = NULL;
    printf("Manager::start: ecall_configure!\n"); 
    int configure_ret = 0;
    ret = ecall_configure(eid, &configure_ret, conf_begin, conf_size);
    if(ret != SGX_SUCCESS || configure_ret != 2) 
    {
        printf("Manager::error: ecall_configure!\n");
        exit(Exit_FAILURE);
    }
    printf("Manager::ok: ecall_configure!\n"); 
    return 0;


}
int
Manager::initialize(ErrorHandler* errh)
{
    sgx_status_t ret = SGX_SUCCESS;
    int initialize_ret = 0;
    printf("Manager::initialize start!\n");
    ret = ecall_initialize(eid, &initialize_ret);
    if(ret != SGX_SUCCESS || initialize_ret != 3)
    {
        printf("Manager::error: enclave_initialize!\n");
    }
}


/*
void *
Manager::com_module(void* arg)
{
    printf("Manager::com_module\n");
    struct controller_ip* c_ip = (struct controller_ip *)arg;
    
    printf("Manager::%s\n",c_ip->ip.c_str());
    printf("Manager::%s\n",c_ip->port.c_str());
    com->run();
}
*/

#if HAVE_BATCH
PacketBatch *
Manager::simple_action_batch(PacketBatch *head)
{
    Packet* current = head;
    sgx_status_t ret =SGX_SUCCESS;
    int simple_ret = 0;
    printf("****************Manager::ecall_batch_simple_action*********** \n");
    clock_t start=clock();
    ret = ecall_batch_simple_action(eid, &simple_ret,(void *)const_cast<unsigned char*>(head->data()),head->length());
    //cout<<head->data()<<"eeeeeeeee"<<head->length()<<endl;
    
    //Packet* current = head;
    //while (current != NULL) {
        //printf("ocall_print:%s\n",current->data());
      //  current = current->next();
    //}
    //cout<<head->data()<<endl;
    //cout<<head->count()<<endl;
    clock_t finish=clock();
    cout<<(finish-start)*1000000/CLOCKS_PER_SEC<<"us"<<endl;
    return head;
}
#endif

Packet*
Manager::simple_action(Packet* p)
{
    sgx_status_t ret =SGX_SUCCESS;
    int simple_ret = 0;
    clock_t start=clock();
    ret = ecall_simple_action(eid, &simple_ret,(void *)const_cast<unsigned char*>(p->data()),p->length());
    if(ret != SGX_SUCCESS || simple_ret != 0)
    {
        printf("Error: ecall_simple_action!\n");
    }
   // printf("Manager::after %x \n",*(p->data()));

    
    //printf("Manager::ok: ecall_simple_action!\n");

    clock_t finish=clock();
    cout<<(finish-start)*1000000/CLOCKS_PER_SEC<<"us"<<endl;
    return p;
}

/*
void
Communicate :: run(void)
{
    int num = 0;
    do{
        num++;
        printf("Communicate ::%d\n",num);
        sleep(1);
    }while(1);3
    
}
*/
void
ocall_to_device(void *packet, int packet_length)
{
    //void *tmp =packet;
    //Packet* p;
    //p->data()=const_cast<unsigned char *>tmp;
    
    //for(int i = 0; i < packet_length; i++)
    //{
    //    printf("ocall_to_device:after %x\n",*((unsigned char *)tmp));
     //   tmp++;
    //}
    //printf("ocall_to_device:after %x\n",*((unsigned char *)tmp));
    return;
}

void
ocall_print(const char* data)
{
    printf("ocall_print:%s\n",data);
    return;
}
/*
void
ocall_print(int data, int data_length
{
    printf("%d\n",data);
}
*/



CLICK_ENDDECLS
EXPORT_ELEMENT(Manager)
ELEMENT_REQUIRES(userlevel)


