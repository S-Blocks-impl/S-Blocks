#include "manager.hh"
//#define ECALL_CONF_SIZE 128
//#define _conf_size(n) (n + ECALL_CONF_SIZE -1) & ~(ECALL_CONF_SIZE -1) //对齐ECALL_CONF_SIZE

CLICK_DECLS

//Communicate* Manager::com = new Communicate();

Manager::Manager()
{

}

Manager::~Manager()
{
    // if(sgx_destroy_enclave(eid) != SGX_SUCCESS)
    // {
    //     printf("error: destroying enclave!\n");
    // }
    // pthread_join(com_thread);
}

int
Manager::configure(Vector<String> &conf, ErrorHandler* errh)
{
    //创建connection实例，初始化socket链接
    
    // struct controller_ip c_ip;
    // if(Args(conf, this, errh)
    //     .read("ip", c_ip.ip)
    //     .read("port", c_ip.port)
    //     .complete() < 0)
    //     return -1
        
    // if(pthread_create(&com_thread, NULL, manager::com_module, &c_ip))
    // {
    //     printf("error: creating com_module thread!\n");
    //     exit(Exit_FAILURE);
    // }

    // //create enclave
    // sgx_status_t ret = SGX_SUCCESS;
    // ret = sgx_create_enclave(ENCLAVE_FILE, DEBUG, token, &eid, NULL);
    // if(ret != SGX_SUCCESS)
    // {
    //     printf("error: creating enclave!\n");
    //     exit(Exit_FAILURE);
    // }

    // //create router
    // int generate_ret = 0;
    // ret = ecall_generat_router(eid, &generate_ret);
    // if(ret != SGX_SUCCESS || generate_ret != 0)
    // {
    //     printf("ERROR: generat_router!\n");
    //     exit(Exit_FAILURE);
    // }


    // //enclave configure
    // int generate_ret = 0;
    // //int size = conf.size();
    // int conf_length = 0;
    // char* conf_begin = NULL;
    // char* conf_ptr = NULL;
    // for(int i = 0; i < conf.size(); i++)
    //     conf_length += (conf[i].length + 1 + sizeof(int));
    // conf_begin = malloc(conf_length);
    // if(conf_begin == NULL)
    // {
    //     printf("Error: malloc for conf !\n");
    //     exit(Exit_FAILURE);
    // }
    // conf_ptr = conf_begin;
    // for(int i = 0; i < conf.size(); i++)
    // {
    //     int length =  conf[i].length;
    //     (int)*conf_ptr = length;
    //     conf_ptr += sizeof(int);
    //     memcpy(conf_ptr, conf[i].data(),length+1);
    //     if(i != conf.size()-1)
    //         conf_ptr += length+1+sizeof(int);        
    // }
    
    // int configure_ret = 0;
    // ret = ecall_configure(eid, &configure_ret, conf_begin, conf_length);
    // if(ret != SGX_SUCCESS || configure_ret != 0) 
    // {
    //     printf("Error: generate_router!\n");
    //     exit(Exit_FAILURE);
    // } 
    // return 0;


}
int
Manager::initialize(ErrorHandler* errh)
{
    // sgx_status_t ret = SGX_SUCCESS;
    // int initialize_ret = 0;
    // ret = ecall_initialize(eid, &initialize_ret);
    // if(ret != SGX_SUCCESS || initialize_ret != 0)
    // {
    //     printf("Error: enclave_initialize!\n");
    // }
}

/*void
Manager::com_module(void* arg)
{
    // struct controller c_ip = (struct controller_ip *)arg;
    // printf("%s\n",c_ip.ip);
    // printf("%s\n",c_ip.port);
    // com->run();
}*/

Packet*
Manager::simple_action(Packet* p)
{
    // sgx_status_t ret =SGX_SUCCESS;
    // int simple_ret = 0;
    // ret = ecall_simple_action(eid, &simple_ret);
    // if(ret != SGX_SUCCESS || simple_ret != 0)
    // {
    //     printf("Error: ecall_simple_action!\n");
    // }
    // return p;
}

//void
//Communicate :: run(void)
// {
//     int num = 0;
//     do{
//         num++;
//         pritf("%d",num);
//         sleep(10);
//     }while(1)
//}

CLICK_ENDDECLS
EXPORT_ELEMENT(print_SGX)
ELEMENT_REQUIRES(userlevel)


