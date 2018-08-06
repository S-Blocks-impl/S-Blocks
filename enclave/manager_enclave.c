#include "manager_enclave.h"
#include "./lib/error.h"
#include "./lib/String.h"
#include "./lib/confparse.h"
#include <vector>

#include "./element/element_enclave.h"
#include "./element/classifier.h"
#include "./element/ddos.h"
#include "./element/checklength.h"
#include "./element/checkipheader.h"
#include "./element/strip.h"
#include "./element/ipclassifier.h"
#include "./element/nameinfo.h"
#include "./element/ipfieldinfo.h"
#include "./element/ipnameinfo.h"
Manager_enclave::Manager_enclave(int i)
   :_state(ROUTER_NEW)
{
}


int 
Manager_enclave::generat_router()
{
    //to build the port of manager_enclave
    NameInfo::static_initialize();
    IPFilter::static_initialize();
    IPNameInfo::static_initialize();
    IPFieldInfo::static_initialize();


    _main_nports = 1;
    _main_ports = new Element::Port[_main_nports];
   
    _attachment_names.clear();
    _attachments.clear();

    Element* tmp;
    //0 1,2
    tmp = new Classifier();
    add_element(tmp, String(tmp->class_name()), String("12/0800,-"));
    tmp->set_nports(1, 2);

    //1 1,1
    tmp = new Strip();
    add_element(tmp, String(tmp->class_name()), String("14"));
    tmp->set_nports(1,1);
    
    //2 1,1
    tmp = new CheckIPHeader();
    add_element(tmp, String(tmp->class_name()), String("CHECKSUM false"));
    tmp->set_nports(1,1);
    
    //3 1,4
    tmp = new IPClassifier();
    add_element(tmp, String(tmp->class_name()), String("rst,tcp,dst udp port 7 or 19,-"));
    tmp->set_nports(1,4);
    
    //4 1,2
    tmp = new CheckLength();
    add_element(tmp, String(tmp->class_name()), String("12"));
    tmp->set_nports(1,2);

    //5 5,2
    tmp = new DDOS();
    add_element(tmp, String(tmp->class_name()), String(""));
    tmp->set_nports(5,2);
   
    
    
    
    this->connect_main_port(0,_elements[0],0); //input
    
    _elements[0]->connect_port(1,0,_elements[1],0);//Classifier 0->0 Strip
    _elements[0]->connect_port(1,1,_elements[5],0);//Classifier 1->0 DDOS
    _elements[1]->connect_port(1,0,_elements[2],0);//Strip 0->0 CheckIPHeader
    _elements[2]->connect_port(1,0,_elements[3],0);//CheckIPHeader 0->0 IPClassifier
    _elements[3]->connect_port(1,0,_elements[5],1);//IPClassifier 0->1 DDOS
    _elements[3]->connect_port(1,1,_elements[5],2);//IPClassifier 1->2 DDOS
    _elements[3]->connect_port(1,2,_elements[5],3);//IPClassifier 2->3 DDOS
    _elements[3]->connect_port(1,3,_elements[5],4);//IPClassifier 3->4 DDOS
    
    _elements[1]->connect_port(0,0,_elements[0],0);//Strip 0<-0 Classifier
    _elements[2]->connect_port(0,0,_elements[1],0);//CheckIPHeader 0<-0 Strip
    _elements[3]->connect_port(0,0,_elements[2],0);//IPClassifier 0<-0 CheckIPHeader
    _elements[5]->connect_port(0,0,_elements[0],1);//DDOS 0<-1 Classifier
    _elements[5]->connect_port(0,1,_elements[3],0);//DDOS 1<-0 IPClassifier
    _elements[5]->connect_port(0,2,_elements[3],1);//DDOS 2<-1 IPClassifier
    _elements[5]->connect_port(0,3,_elements[3],2);//DDOS 3<-2 IPClassifier
    _elements[5]->connect_port(0,4,_elements[3],3);//DDOS 4<-3 IPClassifier
    //ocall_print(_elements[1]->_ports[1][1]._e->class_name());
    //_elements[0]->connect_port(1,0,_elements[1],0);
    //_elements[0]->connect_port(1,1,_elements[1],1);
    //_elements[1]->connect_port(0,0,_elements[0],0);
    //_elements[1]->connect_port(0,1,_elements[0],1);
    
    
   

    return 1;
    
}

int
Manager_enclave::configure(char* arg, int conf_size, ErrorHandler *errh)
{
    char* arg_tmp = arg; 
    int arg_size = 0;
    int conf_size_tmp = conf_size;
   /* while(conf_size_tmp > 0)
    {
        arg_size = *((int *)arg_tmp);
        arg_tmp += sizeof(int);
        _element_configurations.push_back(String(arg_tmp, arg_size+1));
        conf_size_tmp -= (sizeof(int) + arg_size +1);
        arg_tmp += arg_size+1;
    }*/
    
    stlpmtx_std::vector<String> conf_e;
    for(int i = 0; i < nelements(); i++)
    {
        if(conf_e.size()>0)
            conf_e.clear();
        cp_argvec(_element_configurations[i], conf_e);
        _elements[i]->configure(conf_e, errh);
    }

    return 2;
}

int 
Manager_enclave::initialize(ErrorHandler *errh)
{
    _state = ROUTER_LIVE;
    for(int i = 0; i< nelements(); i++)
        _elements[i]->initialize(errh);
    return 3;
}

Packet*
Manager_enclave::action(Packet * p)
{
    //unsigned char* data =const_cast<unsigned char*>(p->data());
    //data[0] = 0x22;
    PRINT("Manager_enclave::action");
    main_ports(0).push(p);
    return p;
}

PacketBatch*
Manager_enclave::batch_action(PacketBatch *head)
{
    //unsigned char* data =const_cast<unsigned char*>(p->data());
    //data[0] = 0x22;
    PRINT("Manager_enclave::batch_action");
    main_ports(0).push_batch(head);
    return head;
}


int
Manager_enclave::connect_main_port(int port, Element *e, int e_port)
{
    _main_ports[port]._e = e;
    _main_ports[port]._port = e_port;
    return 0;
    return 0;
}


int
Manager_enclave::command_routerTable()
{
    return 6;
}

Manager_enclave::~Manager_enclave()
{
}



// act as router



/** @brief  Return @a router's element with index @a eindex.
 *  @param  router  the router (may be null)
 *  @param  eindex  element index, or -1 for router->root_element()
 *
 *  This function returns @a router's element with index @a eindex.  If
 *  @a router is null or @a eindex is out of range, returns null. */
Element *
Manager_enclave::element(const Manager_enclave *manager, int eindex)
{
    if (manager && eindex >= 0 && eindex < manager->nelements())
        return manager->_elements[eindex];
    else if (manager && eindex == -1)
        return manager->root_element();
    else
        return 0;
}
/** @brief  Returns element index @a eindex's name.
 *  @param  eindex  element index
 *
 *  Returns the empty string if @a eindex is out of range. */
const String &
Manager_enclave::ename(int eindex) const
{
    if ((unsigned) eindex < (unsigned) nelements())
        return _element_names[eindex];
    else
        return String::make_empty();
}

/** @brief  Returns element index @a eindex's configuration string.
 *  @param  eindex  element index
 *
 *  Returns the empty string if @a eindex is out of range.
 *
 *  @note econfiguration() returns the element's most recently specified
 *  static configuration string, which might differ from the element's active
 *  configuration string.  For the active configuration, call
 *  Element::configuration(), which might include post-initialization
 *  changes. */
const String &
Manager_enclave::econfiguration(int eindex) const
{
    if (eindex < 0 || eindex >= nelements())
        return String::make_empty();
    else
        return _element_configurations[eindex];
}
/** @brief  Sets element index @a eindex's configuration string.
 *  @param  eindex  element index
 *  @param  conf    configuration string
 *
 *  Does nothing if @a eindex is out of range. */
void
Manager_enclave::set_econfiguration(int eindex, const String &conf)
{
    if (eindex >= 0 && eindex < nelements())
        _element_configurations[eindex] = conf;
}
//@zyh
//修改了内部代码和接口参数
int
Manager_enclave::add_element(Element *e, const String &ename, const String &conf)
{

    _elements.push_back(e);
    _element_names.push_back(ename);
    _element_configurations.push_back(conf);
    // router now owns the element
    int i = _elements.size() - 1;
    e->attach_manager_enclave(this, i);
    return i;
}




inline void
Handler::combine(const Handler &x)
{
    // Takes relevant data from 'x' and adds it to this handler.
    // If this handler has no read version, add x's read handler, if any.
    // If this handler has no write version, add x's write handler, if any.
    // If this handler has no read or write version, make sure we copy both
    // of x's user_data arguments, since set-handler may have set them both.
    if (!(_flags & f_read) && (x._flags & f_read)) {
        _read_hook = x._read_hook;
        _read_user_data = x._read_user_data;
        _flags |= x._flags & (f_read | f_read_comprehensive | ~f_special);
    }
    if (!(_flags & f_write) && (x._flags & f_write)) {
        _write_hook = x._write_hook;
        _write_user_data = x._write_user_data;
        _flags |= x._flags & (f_write | f_write_comprehensive | ~f_special);
    }
    if (!(_flags & (f_read | f_write))) {
        _read_user_data = x._read_user_data;
        _write_user_data = x._write_user_data;
    }
}

inline bool
Handler::compatible(const Handler &x) const
{
    return (_read_hook.r == x._read_hook.r
            && _write_hook.w == x._write_hook.w
            && _read_user_data == x._read_user_data
            && _write_user_data == x._write_user_data
            && _flags == x._flags);
}
int
Handler::call_write(const String& value, Element* e, ErrorHandler* errh) const
{
    LocalErrorHandler lerrh(errh);
    if ((_flags & (f_write | f_write_comprehensive)) == f_write)
        return _write_hook.w(value, e, _write_user_data, &lerrh);
    else if (_flags & f_write) {
        String s(value);
        return _write_hook.h(f_write, s, e, this, &lerrh);
    } else {
        lerrh.error("%<%s%> not a write handler", unparse_name(e).c_str());
        return -EACCES;
    }
}



String
Handler::unparse_name(Element *e, const String &hname)
{
    if (e && e->eindex() >= 0)
        return e->name() + "." + hname;
    else
        return hname;
}

String
Handler::unparse_name(Element *e) const
{
    if (this == the_blank_handler)
        return _name;
    else
        return unparse_name(e, _name);
}



inline Handler*
Manager_enclave::xhandler(int hi) const
{
    return &_handler_bufs[hi / HANDLER_BUFSIZ][hi % HANDLER_BUFSIZ];
}


int
Manager_enclave::find_ehandler(int eindex, const String &hname, bool allow_star) const
{
    int eh = _ehandler_first_by_element[eindex];
    int star_h = -1;
    while (eh >= 0) {
        int h = _ehandler_to_handler[eh];
        const String &hn = xhandler(h)->name();
        if (hn == hname)
            return eh;
        else if (hn.length() == 1 && hn[0] == '*')
            star_h = h;
        eh = _ehandler_next[eh];
    }
    if (allow_star && star_h >= 0 && xhandler(star_h)->writable()) {
        // BEWARE: hname might be a fake string pointing to mutable data, so
        // make a copy of the string before it might escape.
        String real_hname(hname.data(), hname.length());
        if (xhandler(star_h)->call_write(real_hname, element(eindex), ErrorHandler::default_handler()) >= 0)
            eh = find_ehandler(eindex, real_hname, false);
    }
    return eh;
}

inline Handler
Manager_enclave::fetch_handler(const Element* e, const String& name)
{
    if (const Handler* h = handler(e, name))
        return *h;
    else
        return Handler(name);
}

void
Manager_enclave::store_local_handler(int eindex, Handler &to_store)
{
    int old_eh = find_ehandler(eindex, to_store.name(), false);
    if (old_eh >= 0) {
        Handler *old_h = xhandler(_ehandler_to_handler[old_eh]);
        to_store.combine(*old_h);
        old_h->_use_count--;
    }

    // find the offset in _name_handlers
    int name_index;
    {
        int *l = _handler_first_by_name.begin();
        int *r = _handler_first_by_name.end();
        while (l < r) {
            int *m = l + (r - l) / 2;
            int cmp = String::compare(to_store._name, xhandler(*m)->_name);
            if (cmp < 0)
                r = m;
            else if (cmp > 0)
                l = m + 1;
            else {
                // discourage the storage of multiple copies of the same name
                to_store._name = xhandler(*m)->_name;
                l = m;
                break;
            }
        }
        if (l >= r)
            l = _handler_first_by_name.insert(l, -1);
        name_index = l - _handler_first_by_name.begin();
    }

    // find a similar handler, if any exists
    int* prev_h = &_handler_first_by_name[name_index];
    int h = *prev_h;
    int* blank_prev_h = 0;
    int blank_h = -1;
    int stored_h = -1;
    while (h >= 0) {
        Handler* han = xhandler(h);
        if (han->compatible(to_store))
            stored_h = h;
        else if (han->_use_count == 0)
            blank_h = h, blank_prev_h = prev_h;
        prev_h = &han->_next_by_name;
        h = *prev_h;
    }

    // if none exists, assign this one to a blank spot
    if (stored_h < 0 && blank_h >= 0) {
        stored_h = blank_h;
        *xhandler(stored_h) = to_store;
        xhandler(stored_h)->_use_count = 0;
    }

    // if no blank spot, add a handler
    if (stored_h < 0) {
        if (_free_handler < 0) {
            int n_handler_bufs = _nhandlers_bufs / HANDLER_BUFSIZ;
            Handler** new_handler_bufs = new Handler*[n_handler_bufs + 1];
            Handler* new_handler_buf = new Handler[HANDLER_BUFSIZ];
            if (!new_handler_buf || !new_handler_bufs) {        // out of memory
                delete[] new_handler_bufs;
                delete[] new_handler_buf;
                if (old_eh >= 0)        // restore use count
                    xhandler(_ehandler_to_handler[old_eh])->_use_count++;
                return;
            }
            for (int i = 0; i < HANDLER_BUFSIZ - 1; i++)
                new_handler_buf[i]._next_by_name = _nhandlers_bufs + i + 1;
            _free_handler = _nhandlers_bufs;
            memcpy(new_handler_bufs, _handler_bufs, sizeof(Handler*) * n_handler_bufs);
            new_handler_bufs[n_handler_bufs] = new_handler_buf;
            delete[] _handler_bufs;
            _handler_bufs = new_handler_bufs;
            _nhandlers_bufs += HANDLER_BUFSIZ;
        }
        stored_h = _free_handler;
        _free_handler = xhandler(stored_h)->_next_by_name;
        *xhandler(stored_h) = to_store;
        xhandler(stored_h)->_use_count = 0;
        xhandler(stored_h)->_next_by_name = _handler_first_by_name[name_index];
        _handler_first_by_name[name_index] = stored_h;
    }

    // point ehandler list at new handler
    if (old_eh >= 0)
        _ehandler_to_handler[old_eh] = stored_h;
    else {
        int new_eh = _ehandler_to_handler.size();
        _ehandler_to_handler.push_back(stored_h);
        _ehandler_next.push_back(_ehandler_first_by_element[eindex]);
        _ehandler_first_by_element[eindex] = new_eh;
    }

    // increment use count
    xhandler(stored_h)->_use_count++;

    // perhaps free blank_h
    if (blank_h >= 0 && xhandler(blank_h)->_use_count == 0) {
        *blank_prev_h = xhandler(blank_h)->_next_by_name;
        xhandler(blank_h)->_next_by_name = _free_handler;
        _free_handler = blank_h;
    }
}

//handlers
const Handler* Handler::the_blank_handler;
static Handler* globalh;
static int nglobalh;
static int globalh_cap;

void
Manager_enclave::store_global_handler(Handler &h)
{
    for (int i = 0; i < nglobalh; i++)
        if (globalh[i]._name == h._name) {
            h.combine(globalh[i]);
            globalh[i] = h;
            globalh[i]._use_count = 1;
            return;
        }

    if (nglobalh >= globalh_cap) {
        int n = (globalh_cap ? 2 * globalh_cap : 4);
        Handler *hs = new Handler[n];
        if (!hs)                        // out of memory
            return;
        for (int i = 0; i < nglobalh; i++)
            hs[i] = globalh[i];
        delete[] globalh;
        globalh = hs;
        globalh_cap = n;
    }

    globalh[nglobalh] = h;
    globalh[nglobalh]._use_count = 1;
    nglobalh++;
}


inline void
Manager_enclave::store_handler(const Element *e, Handler &to_store)
{
    if (e && e->eindex() >= 0)
        e->manager_enclave()->store_local_handler(e->eindex(), to_store);
    else
        store_global_handler(to_store);
}
// Public functions for finding handlers

/** @brief Return @a router's handler with index @a hindex.
 * @param router the router
 * @param hindex handler index (Router::hindex())
 * @return the Handler, or null if no such handler exists
 *
 * Returns the Handler object on @a router with handler index @a hindex.  If
 * @a hindex >= FIRST_GLOBAL_HANDLER, then returns a global handler.  If @a
 * hindex is < 0 or corresponds to no existing handler, returns null.
 *
 * The return Handler pointer remains valid until the named handler is changed
 * in some way (add_read_handler(), add_write_handler(), set_handler(), or
 * set_handler_flags()).
 */
const Handler*
Manager_enclave::handler(const Manager_enclave *manager, int hindex)
{
    if (manager && hindex >= 0 && hindex < manager->_nhandlers_bufs)
        return manager->xhandler(hindex);
    else if (hindex >= FIRST_GLOBAL_HANDLER
             && hindex < FIRST_GLOBAL_HANDLER + nglobalh)
        return &globalh[hindex - FIRST_GLOBAL_HANDLER];
    else
        return 0;
}

/** @brief Return element @a e's handler named @a hname.
 * @param e element, if any
 * @param hname handler name
 * @return the Handler, or null if no such handler exists
 *
 * Searches for element @a e's handler named @a hname.  Returns NULL if no
 * such handler exists.  If @a e is NULL or equal to some root_element(), then
 * this function searches for a global handler named @a hname.
 *
 * The return Handler pointer remains valid until the named handler is changed
 * in some way (add_read_handler(), add_write_handler(), set_handler(), or
 * set_handler_flags()).
 */
const Handler *
Manager_enclave::handler(const Element* e, const String& hname)
{
    if (e && e->eindex() >= 0) {
        const Manager_enclave *r = e->manager_enclave();
        int eh = r->find_ehandler(e->eindex(), hname, true);
        if (eh >= 0)
            return r->xhandler(r->_ehandler_to_handler[eh]);
    } else {                    // global handler
        for (int i = 0; i < nglobalh; i++)
            if (globalh[i]._name == hname)
                return &globalh[i];
    }
    return 0;
}


/** @brief Return the handler index for element @a e's handler named @a hname.
 * @param e element, if any
 * @param hname handler name
 * @return the handler index, or -1 if no such handler exists
 *
 * Searches for element @a e's handler named @a hname.  Returns -1 if no
 * such handler exists.  If @a e is NULL or equal to some root_element(), then
 * this function searches for a global handler named @a hname.
 *
 * The returned integer is a handler index, which is a number that identifies
 * the handler.  An integer >= FIRST_GLOBAL_HANDLER corresponds to a global
 * handler.
 */
int
Manager_enclave::hindex(const Element *e, const String &hname)
{
    // BEWARE: This function must work correctly even if hname is a fake
    // string pointing to mutable data, so find_ehandler() makes a copy of the
    // string at the point where it might escape.
    if (e && e->eindex() >= 0) {
        const Manager_enclave *r = e->manager_enclave();
        int eh = r->find_ehandler(e->eindex(), hname, true);
        if (eh >= 0)
            return r->_ehandler_to_handler[eh];
    } else {                    // global handler
        for (int i = 0; i < nglobalh; i++)
            if (globalh[i]._name == hname)
                return FIRST_GLOBAL_HANDLER + i;
    }
    return -1;
}

/** @brief Return the handler indexes for element @a e's handlers.
 * @param e element, if any
 * @param result collector for handler indexes
 *
 * Iterates over all element @a e's handlers, and appends their handler
 * indexes to @a result.  If @a e is NULL or equal to some root_element(),
 * then iterates over the global handlers.
 */
void
Manager_enclave::element_hindexes(const Element *e, stlpmtx_std::vector<int> &result)
{
    if (e && e->eindex() >= 0) {
        const Manager_enclave *r = e->manager_enclave();
        for (int eh = r->_ehandler_first_by_element[e->eindex()];
             eh >= 0;
             eh = r->_ehandler_next[eh])
            result.push_back(r->_ehandler_to_handler[eh]);
    } else {
        for (int i = 0; i < nglobalh; i++)
            result.push_back(FIRST_GLOBAL_HANDLER + i);
    }
}


// Public functions for storing handlers

/** @brief Add an @a e.@a hname read handler.
 * @param e element, if any
 * @param hname handler name
 * @param callback read callback
 * @param user_data user data for read callback
 * @param flags additional flags to set (Handler::flags())
 *
 * Adds a read handler named @a hname for element @a e.  If @a e is NULL or
 * equal to some root_element(), then adds a global read handler.  The
 * handler's callback function is @a callback.  When the read handler is
 * triggered, Click will call @a callback(@a e, @a user_data).
 *
 * Any previous read handler with the same name and element is replaced.  Any
 * comprehensive handler function (see set_handler()) is replaced.  Any
 * write-only handler (add_write_handler()) remains.
 *
 * The new handler's flags equal the old flags or'ed with @a flags.  Any
 * special flags in @a flags are ignored.
 *
 * To create a read handler with parameters, you must use @a set_handler().
 *
 * @sa add_write_handler(), set_handler(), set_handler_flags()
 */
void
Manager_enclave::add_read_handler(const Element *e, const String &hname,
                         ReadHandlerCallback callback, void *user_data,
                         uint32_t flags)
{
    Handler to_add(hname);
    to_add._read_hook.r = callback;
    to_add._read_user_data = user_data;
    to_add._flags = Handler::f_read | (flags & ~Handler::f_special);
    store_handler(e, to_add);
}

/** @brief Add an @a e.@a hname write handler.
 * @param e element, if any
 * @param hname handler name
 * @param callback read callback
 * @param user_data user data for write callback
 * @param flags additional flags to set (Handler::flags())
 *
 * Adds a write handler named @a hname for element @a e.  If @a e is NULL or
 * equal to some root_element(), then adds a global write handler.  The
 * handler's callback function is @a callback.  When the write handler is
 * triggered, Click will call @a callback(data, @a e, @a user_data, errh).
 *
 * Any previous write handler with the same name and element is replaced.  Any
 * comprehensive handler function (see set_handler()) is replaced.  Any
 * read-only handler (add_read_handler()) remains.
 *
 * The new handler's flags equal the old flags or'ed with @a flags.  Any
 * special flags in @a flags are ignored.
 *
 * @sa add_read_handler(), set_handler(), set_handler_flags()
 */
void
Manager_enclave::add_write_handler(const Element *e, const String &hname,
                          WriteHandlerCallback callback, void *user_data,
                          uint32_t flags)
{
    Handler to_add(hname);
    to_add._write_hook.w = callback;
    to_add._write_user_data = user_data;
    to_add._flags = Handler::f_write | (flags & ~Handler::f_special);
    store_handler(e, to_add);
}

/** @brief Add a comprehensive @a e.@a hname handler.
 * @param e element, if any
 * @param hname handler name
 * @param flags flags to set (Handler::flags())
 * @param callback comprehensive handler callback
 * @param read_user_data read user data for @a callback
 * @param write_user_data write user data for @a callback
 *
 * Sets a handler named @a hname for element @a e.  If @a e is NULL or equal
 * to some root_element(), then sets a global handler.  The handler's callback
 * function is @a callback.  The resulting handler is a read handler if @a flags
 * contains Handler::f_read, and a write handler if @a flags contains
 * Handler::f_write.  If the flags contain Handler::f_read_param, then any read
 * handler will accept parameters.
 *
 * When the handler is triggered, Click will call @a callback(operation, data,
 * @a e, h, errh), where:
 *
 * <ul>
 * <li>"operation" is Handler::f_read or Handler::f_write;</li>
 * <li>"data" is the handler data (empty for reads without parameters);</li>
 * <li>"h" is a pointer to a Handler object; and</li>
 * <li>"errh" is an ErrorHandler.</li>
 * </ul>
 *
 * Any previous handlers with the same name and element are replaced.
 *
 * @sa add_read_handler(), add_write_handler(), set_handler_flags()
 */
void
Manager_enclave::set_handler(const Element *e, const String &hname, uint32_t flags,
                    HandlerCallback callback,
                    void *read_user_data, void *write_user_data)
{
    Handler to_add(hname);
    if (flags & Handler::f_read) {
        to_add._read_hook.h = callback;
        flags |= Handler::f_read_comprehensive;
    } else
        flags &= ~(Handler::f_read_comprehensive | Handler::f_read_param);
    if (flags & Handler::f_write) {
        to_add._write_hook.h = callback;
        flags |= Handler::f_write_comprehensive;
    } else
        flags &= ~Handler::f_write_comprehensive;
    to_add._read_user_data = read_user_data;
    to_add._write_user_data = write_user_data;
    to_add._flags = flags;
    store_handler(e, to_add);
}

/** @brief Change the @a e.@a hname handler's flags.
 * @param e element, if any
 * @param hname handler name
 * @param set_flags flags to set (Handler::flags())
 * @param clear_flags flags to clear (Handler::flags())
 * @return 0 if the handler existed, -1 otherwise
 *
 * Changes the handler flags for the handler named @a hname on element @a e.
 * If @a e is NULL or equal to some root_element(), then changes a global
 * handler.  The handler's flags are changed by clearing the @a clear_flags
 * and then setting the @a set_flags, except that the special flags
 * (Handler::f_special) are unchanged.
 *
 * @sa add_read_handler(), add_write_handler(), set_handler()
 */
int
Manager_enclave::set_handler_flags(const Element *e, const String &hname,
                          uint32_t set_flags, uint32_t clear_flags)
{
    Handler to_add = fetch_handler(e, hname);
    if (to_add._use_count > 0) {        // only modify existing handlers
        clear_flags &= ~Handler::f_special;
        set_flags &= ~Handler::f_special;
        to_add._flags = (to_add._flags & ~clear_flags) | set_flags;
        store_handler(e, to_add);
        return 0;
    } else
        return -1;
}

void
Manager_enclave::add_get_state_handler(const Element *e, const String &hname, uint32_t flags,
                    HandlerCallback callback,
                    void *get_user_data)
{
    Handler to_add(hname);
    to_add._read_hook.h = callback;
    to_add._read_user_data = get_user_data;
    to_add._flags = Handler::f_read;//暂时用f_read表示
    store_handler(e, to_add);
    
}

void
Manager_enclave::add_set_state_handler(const Element *e, const String &hname, uint32_t flags,
                    HandlerCallback callback,
                    void *set_user_data)
{
    Handler to_add(hname);
    to_add._write_hook.h = callback;
    to_add._write_user_data = set_user_data;
    to_add._flags = Handler::f_write;//暂时用f_write替代
    store_handler(e, to_add);
}

//add by liyi 
void*
Manager_enclave::attachment(const String &name) const
{
    for (int i = 0; i < _attachments.size(); i++)
        if (_attachment_names[i] == name)
            return _attachments[i];
    return 0;
}
//liyi
void*&
Manager_enclave::force_attachment(const String &name)
{
    for (int i = 0; i < _attachments.size(); i++)
        if (_attachment_names[i] == name)
            return _attachments[i];
    _attachment_names.push_back(name);
    _attachments.push_back(0);
    return _attachments.back();
}
//liyi
void *
Manager_enclave::set_attachment(const String &name, void *value)
{
    for (int i = 0; i < _attachments.size(); i++)
        if (_attachment_names[i] == name) {
            void *v = _attachments[i];
            _attachments[i] = value;
            return v;
        }
    _attachment_names.push_back(name);
    _attachments.push_back(value);
    return 0;
}


////////////////haoshirong///////////
NameInfo*
Manager_enclave::force_name_info()
{
    if (!_name_info)
        _name_info = new NameInfo;
    return _name_info;
}

                    
