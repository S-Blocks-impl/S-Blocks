#ifndef _ENCLAVE_MANAGER_ENCLAVE
#define _ENCLAVE_MANAGER_ENCLAVE
#include "./lib/packet.h"
#include "./lib/packetbatch.h"
#include <vector>
#include "./include/config.h"
#include "./lib/String.h"
#include "./element/element_enclave.h"

class Handler;
class NameInfo;
class Manager_enclave
{
public:
/////////////haoshirong/////////////

	mutable NameInfo* _name_info;
	inline NameInfo* name_info() const;
	NameInfo* force_name_info();
	inline bool initialized() const;

//////////////////haoshirong+++//////////

    Manager_enclave(int i);
    ~Manager_enclave();
    int configure(char* arg,int conf_size, ErrorHandler *errh);
    int initialize(ErrorHandler *errh = 0);
   
    //int add_handler(const String & s, Enclave_Element * e, void *, ErrorHandler * errh);
    int get_state();
    //int set_state(const String & s, ErrorHandler * errh);
    //int set_state(const string & s);
    int switch_packet(Packet* p);
    Packet* action(Packet* p);
    PacketBatch* batch_action(PacketBatch *head);
    int copy_instance();
    int attack_alarm();
    int generat_router();
    int command_routerTable();

    int connect_main_port(int port, Element*, int e_port);


   //Elements
    inline const stlpmtx_std::vector<Element*>& elements() const;
    inline int nelements() const;
    inline Element* element(int i) const;
    inline Element* root_element() const;
    static Element* element(const Manager_enclave *manager, int i);


    const String& ename(int i) const;
    const String& econfiguration(int i) const;
    void set_econfiguration(int i, const String& conf);
    int add_element(Element *e, const String &ename, const String &conf);
    
    //int set_handler
    static const Handler *handler(const Element *e, const String &hname);
    static void store_global_handler(Handler &h);
    static inline void store_handler(const Element *element, Handler &h);
    static inline Handler fetch_handler(const Element*, const String&);
    void store_local_handler(int eindex, Handler &h);
    static void add_read_handler(const Element *e, const String &hname, ReadHandlerCallback callback, void *user_data, uint32_t flags = 0);
    static void add_write_handler(const Element *e, const String &hname, WriteHandlerCallback callback, void *user_data, uint32_t flags = 0);
    static void set_handler(const Element *e, const String &hname, uint32_t flags, HandlerCallback callback, void *read_user_data = 0, void *write_user_data = 0);
    static void add_get_state_handler(const Element *e, const String &hname, uint32_t flags, HandlerCallback callback, void *read_user_data = 0);
    static void add_set_state_handler(const Element *e, const String &hname, uint32_t flags, HandlerCallback callback, void *read_user_data = 0);
    void* attachment(const String& aname) const;//add by liyi
    void*& force_attachment(const String& aname);//liyi
    void* set_attachment(const String& aname, void* value);//liyi
    static int set_handler_flags(const Element *e, const String &hname, uint32_t set_flags, uint32_t clear_flags = 0);
    
   enum { FIRST_GLOBAL_HANDLER = 0x40000000 };
   static int hindex(const Element *e, const String &hname);
   static const Handler *handler(const Manager_enclave *router, int hindex);
   static void element_hindexes(const Element *e, stlpmtx_std::vector<int> &result);
//waitting for add
    class RuleTable
    {//need mutex
    public:
           void add_rule(void);
           void  delete_rule(void);
           void parse_rule(void);
    private:
        
    };
    //mainport
    inline Element::Port& main_ports(int port);
    


private:
    //handler** get_state_handlers[];
    //handler** set_state_handlers[];
    //handler** read_state_handlers[];
    //handler** write_state_handlers[];
    //Enclave_Element * elements[];
    //RuleTable * ruletable;
    
    //mainport
    Element::Port* _main_ports;
    int _main_nports;
    
    Element* _root_element;
    stlpmtx_std::vector<Element *> _elements;
    stlpmtx_std::vector<String> _element_names;
    stlpmtx_std::vector<String> _element_configurations;

    stlpmtx_std::vector<int> _ehandler_first_by_element;
    stlpmtx_std::vector<int> _ehandler_to_handler;
    stlpmtx_std::vector<int> _ehandler_next;
    stlpmtx_std::vector<int> _handler_first_by_name;
    enum { HANDLER_BUFSIZ = 256 };
    Handler** _handler_bufs;
    int _nhandlers_bufs;
    int _free_handler;
    inline Handler* xhandler(int) const;
    int find_ehandler(int, const String&, bool allow_star) const;


    //add by liyi
    stlpmtx_std::vector<String> _attachment_names;
    stlpmtx_std::vector<void*> _attachments;

   ///////////////////////haoshirong/////
	volatile int _state;
    enum {
        ROUTER_NEW, ROUTER_PRECONFIGURE, ROUTER_PREINITIALIZE,
        ROUTER_LIVE, ROUTER_DEAD                // order is important
    };
///////////////haoshirong//////////////// 

};

inline Element::Port&
Manager_enclave:: main_ports(int port)
{
  //  if(port < _main_nports)
  //  {
        return _main_ports[port];
   // }
}



inline Element*
Manager_enclave::root_element() const
{
    return _root_element;
}

inline int
Manager_enclave::nelements() const
{
    return _elements.size();
}

inline const stlpmtx_std::vector<Element*>&
Manager_enclave::elements() const
{
    return _elements;
}
inline Element*
Manager_enclave::element(int i) const
{
    return element(this, i);
}


inline
Handler::Handler(const String &name)
    : _name(name), _read_user_data(0), _write_user_data(0), _flags(0),
      _use_count(0), _next_by_name(-1)
{
    _read_hook.r = 0;
    _write_hook.w = 0;
}



//
inline NameInfo*
Manager_enclave::name_info() const
{
    return _name_info;
}

//
inline bool
Manager_enclave::initialized() const
{
    return _state == ROUTER_LIVE;
}
#endif

