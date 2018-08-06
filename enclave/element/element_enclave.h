#ifndef __ENCLAVE_ELEMENT_H
#define __ENCLAVE_ELEMENT_H

#include "../lib/packet.h"
#include "../lib/String.h"
#include "../include/config.h"
#include "../include/handler.h"
#include <assert.h>
#include <vector>
#include "../lib/ipaddress.h"
#include "../include/inet.h"
#include "../lib/packetbatch.h"

class ErrorHandler;
class Element;
class Manager_enclave;
class BatchElement;

#define BATCH_MAX_PULL 256

class Element
{
public:
    Element();
    virtual ~Element();
    static int nelements_allocated;
    String id() const CLICK_DEPRECATED;
    // RUNTIME
    virtual void push(int port, Packet *p);
    virtual Packet *pull(int port) CLICK_WARN_UNUSED_RESULT;
    virtual Packet *simple_action(Packet *p);
#if HAVE_BATCH
    virtual void push_batch(int port, PacketBatch *p);
    virtual PacketBatch* pull_batch(int port,unsigned max) CLICK_WARN_UNUSED_RESULT;
#endif
    inline void checked_output_push(int port, Packet *p) const;
    inline Packet* checked_input_pull(int port) const;

    inline bool is_fullpush() const;
    enum batch_mode {BATCH_MODE_NO, BATCH_MODE_IFPOSSIBLE, BATCH_MODE_NEEDED, BATCH_MODE_YES};


    // ELEMENT CHARACTERISTICS
    virtual const char *class_name() const = 0;

    virtual const char *port_count() const;
    static const char PORTS_0_0[];
    static const char PORTS_0_1[];
    static const char PORTS_1_0[];
    static const char PORTS_1_1[];
    static const char PORTS_1_1X2[];

    virtual const char *processing() const;
    static const char AGNOSTIC[];
    static const char PUSH[];
    static const char PULL[];
    static const char PUSH_TO_PULL[];
    static const char PULL_TO_PUSH[];
    static const char PROCESSING_A_AH[];
    
    virtual const char *flags() const;
    static const char COMPLETE_FLOW[];


    // CONFIGURATION, INITIALIZATION, AND CLEANUP
    enum ConfigurePhase {
        CONFIGURE_PHASE_FIRST = 0,
        CONFIGURE_PHASE_INFO = 20,
        CONFIGURE_PHASE_PRIVILEGED = 90,
        CONFIGURE_PHASE_DEFAULT = 100,
        CONFIGURE_PHASE_LAST = 2000
    };
    virtual int configure_phase() const;

   virtual int configure(stlpmtx_std::vector<String> &conf, ErrorHandler *errh);
    

    virtual void add_handlers();

   virtual int initialize(ErrorHandler *errh);




    //virtual void cleanup(CleanupStage stage);

    static inline void static_initialize();
    static inline void static_cleanup();

    // ELEMENT ROUTER CONNECTIONS
    String name() const;
    virtual String declaration() const;

    inline Manager_enclave *manager_enclave() const;
    inline int eindex() const;
    inline int eindex(Manager_enclave *r) const;
    inline void attach_manager_enclave(Manager_enclave *r, int eindex) {
        assert(!_manager);
        _manager = r;
        _eindex = eindex;
    }
    
    class Port;
    inline int nports(bool isoutput) const;
    inline int ninputs() const;
    inline int noutputs() const;
    inline const Port &port(bool isoutput, int port) const;
    inline const Port &input(int port) const;
    inline const Port &output(int port) const;

    inline bool port_active(bool isoutput, int port) const;
    inline bool input_is_push(int port) const;
    inline bool input_is_pull(int port) const;
    inline bool output_is_push(int port) const;
    inline bool output_is_pull(int port) const;

    // LIVE RECONFIGURATION
    //String configuration() const;

   // virtual bool can_live_reconfigure() const;
    //virtual int live_reconfigure(Vector<String>&, ErrorHandler*);
    // virtual bool can_live_reconfigure() const;
    // virtual int live_reconfigure(stlpmtx_std::vector<String>&);

    
    // HANDLERS
    void add_read_handler(const String &name, ReadHandlerCallback read_callback, const void *user_data = 0, uint32_t flags = 0);
    void add_read_handler(const String &name, ReadHandlerCallback read_callback, int user_data, uint32_t flags = 0);
    void add_read_handler(const char *name, ReadHandlerCallback read_callback, int user_data = 0, uint32_t flags = 0);
    void add_write_handler(const String &name, WriteHandlerCallback write_callback, const void *user_data = 0, uint32_t flags = 0);
    void add_write_handler(const String &name, WriteHandlerCallback write_callback, int user_data, uint32_t flags = 0);
    void add_write_handler(const char *name, WriteHandlerCallback write_callback, int user_data = 0, uint32_t flags = 0);
    void set_handler(const String &name, int flags, HandlerCallback callback, const void *read_user_data = 0, const void *write_user_data = 0);
    void set_handler(const String &name, int flags, HandlerCallback callback, int read_user_data, int write_user_data = 0);
    void set_handler(const char *name, int flags, HandlerCallback callback, int read_user_data = 0, int write_user_data = 0);
    int set_handler_flags(const String &name, int set_flags, int clear_flags = 0);


    void add_data_handlers(const char *name, int flags, uint8_t *data);
    void add_data_handlers(const char *name, int flags, bool *data);
    void add_data_handlers(const char *name, int flags, uint16_t *data);
    void add_data_handlers(const char *name, int flags, int *data);
    void add_data_handlers(const char *name, int flags, unsigned *data);
    void add_data_handlers(const char *name, int flags, atomic_uint32_t *data);
    void add_data_handlers(const char *name, int flags, long *data);
    void add_data_handlers(const char *name, int flags, unsigned long *data);
#if HAVE_LONG_LONG
    void add_data_handlers(const char *name, int flags, long long *data);
    void add_data_handlers(const char *name, int flags, unsigned long long *data);
#endif

#if HAVE_FLOAT_TYPES
    void add_data_handlers(const char *name, int flags, double *data);
#endif
    void add_data_handlers(const char *name, int flags, String *data);
    void add_data_handlers(const char *name, int flags, IPAddress *data);
        void add_data_handlers(const char *name, int flags, Timestamp *data, bool is_interval = false);
//ipclassifire
    static String read_positional_handler(Element*, void*);
    static String read_keyword_handler(Element*, void*);
    //static int reconfigure_positional_handler(const String&, Element*, void*, ErrorHandler*);
    //static int reconfigure_keyword_handler(const String&, Element*, void*, ErrorHandler*);
    
  
        class Port { public:
            inline bool active() const;
            inline Element* element() const;
            inline int port() const;
    
            inline void push(Packet* p) const;
            inline Packet* pull() const;
#if HAVE_BATCH
        inline void push_batch(PacketBatch* p) const;
        inline PacketBatch* pull_batch(unsigned max) const;

        inline void start_batch();
        inline void end_batch();
#endif

#if CLICK_STATS >= 1
            unsigned npackets() const       { return _packets; }
#endif
            inline void assign(bool isoutput, Element *e, int port);
          private:
    
            Element* _e;
            int _port;
#if HAVE_BOUND_PORT_TRANSFER
        union {
            void (*push)(Element *e, int port, Packet *p);
            Packet *(*pull)(Element *e, int port);
        } _bound;
        union {
#if HAVE_BATCH
            void (*push_batch)(Element *e, int port, PacketBatch *p);
            PacketBatch* (*pull_batch)(Element *e, int port, unsigned max);
#endif
        } _bound_batch;
#endif

#if CLICK_STATS >= 1
        mutable unsigned _packets;      // How many packets have we moved?
#endif
#if CLICK_STATS >= 2
        Element* _owner;                // Whose input or output are we?
#endif

        inline Port();
        inline void assign(bool isoutput, Element *owner, Element *e, int port);

        friend class Element;
        friend class Manager_enclave;//for port add
        friend class BatchElement;

    };
    //String id() const CLICK_DEPRECATED;

protected:
    enum batch_mode in_batch_mode;
    bool receives_batch;

private:
    enum { INLINE_PORTS = 4 };

    Port* _ports[2];
    Port _inline_ports[INLINE_PORTS];

    int _nports[2];

    Manager_enclave* _manager;
    int _eindex;
    Element(const Element &);
    Element &operator=(const Element &);

    // METHODS USED BY ROUTER
    int set_nports(int, int);
//    int notify_nports(int, int, ErrorHandler *);

    enum Processing { VAGNOSTIC, VPUSH, VPULL };
   static int next_processing_code(const char*& p, ErrorHandler* errh);
   void processing_vector(int* input_codes, int* output_codes, ErrorHandler*) const;



    void initialize_ports(const int* input_codes, const int* output_codes);
    int connect_port(bool isoutput, int port, Element*, int);

    static String read_handlers_handler(Element *e, void *user_data);
    void add_default_handlers(bool writable_config);
    inline void add_data_handlers(const char *name, int flags, HandlerCallback callback, void *data);
    friend class Manager_enclave;
    friend class BatchElement;
};









inline void
Element::static_initialize()
{
}

inline void
Element::static_cleanup()
{
}

inline Manager_enclave*
Element::manager_enclave() const
{
    return _manager;
}

inline int
Element::eindex() const
{
    return _eindex;
}

/** @brief Return the element's index within router @a r.
 *
 * Returns -1 if @a r != router(). */
inline int
Element::eindex(Manager_enclave* r) const
{
    return (manager_enclave() == r ? _eindex : -1);
}

/** @brief Return the number of input or output ports.
 * @param isoutput false for input ports, true for output ports */
inline int
Element::nports(bool isoutput) const
{
    return _nports[isoutput];
}

/** @brief Return the number of input ports. */
inline int
Element::ninputs() const
{
    return _nports[0];
}

/** @brief Return the number of output ports. */
inline int
Element::noutputs() const
{
    return _nports[1];
}

/** @brief Return one of the element's ports.
 * @param isoutput false for input ports, true for output ports
 * @param port port number
 *
 * An assertion fails if @a p is out of range. */
inline const Element::Port&
Element::port(bool isoutput, int port) const
{
    assert((unsigned) port < (unsigned) _nports[isoutput]);
    return _ports[isoutput][port];
}

/** @brief Return one of the element's input ports.
 * @param port port number
 *
 * An assertion fails if @a port is out of range.
 *
 * @sa Port, port */
inline const Element::Port&
Element::input(int port) const
{
    return Element::port(false, port);
}

/** @brief Return one of the element's output ports.
 * @param port port number
 *
 * An assertion fails if @a port is out of range.
 *
 * @sa Port, port */
inline const Element::Port&
Element::output(int port) const
{
    return Element::port(true, port);
}
/** @brief Check whether a port is active.
 * @param isoutput false for input ports, true for output ports
 * @param port port number
 *
 * Returns true iff @a port is in range and @a port is active.  Push outputs
 * and pull inputs are active; pull outputs and push inputs are not.
 *
 * @sa Element::Port::active */
inline bool
Element::port_active(bool isoutput, int port) const
{
    return (unsigned) port < (unsigned) nports(isoutput)
        && _ports[isoutput][port].active();
}

/** @brief Check whether output @a port is push.
 *
 * Returns true iff output @a port exists and is push.  @sa port_active */
inline bool
Element::output_is_push(int port) const
{
    return port_active(true, port);
}

/** @brief Check whether output @a port is pull.
 *
 * Returns true iff output @a port exists and is pull. */
inline bool
Element::output_is_pull(int port) const
{
    return (unsigned) port < (unsigned) nports(true)
        && !_ports[1][port].active();
}

/** @brief Check whether input @a port is pull.
 *
 * Returns true iff input @a port exists and is pull.  @sa port_active */
inline bool
Element::input_is_pull(int port) const
{
    return port_active(false, port);
}

/** @brief Check whether input @a port is push.
 *
 * Returns true iff input @a port exists and is push. */
inline bool
Element::input_is_push(int port) const
{
    return (unsigned) port < (unsigned) nports(false)
        && !_ports[0][port].active();
}

#if CLICK_STATS >= 2
# define PORT_ASSIGN(o) _packets = 0; _owner = (o)
#elif CLICK_STATS >= 1
# define PORT_ASSIGN(o) _packets = 0; (void) (o)
#else
# define PORT_ASSIGN(o) (void) (o)
#endif

inline
Element::Port::Port()
    : _e(0), _port(-2)
{
    PORT_ASSIGN(0);
}

inline void
Element::Port::assign(bool isoutput, Element *e, int port)
{
    _e = e;
    _port = port;
#ifdef HAVE_AUTO_BATCH
    for (unsigned i = 0; i < current_batch.weight() ; i++)
        current_batch.set_value(i,0);
#endif
    (void) isoutput;
#if HAVE_BOUND_PORT_TRANSFER
    if (e) {
        if (isoutput) {
            void (Element::*pusher)(int, Packet *) = &Element::push;
            _bound.push = (void (*)(Element *, int, Packet *)) (e->*pusher);
# if HAVE_BATCH
            void (Element::*pushbatcher)(int, PacketBatch *) = &Element::push_batch;
            _bound_batch.push_batch = (void (*)(Element *, int, PacketBatch *)) (e->*pushbatcher);
# endif
        } else {
            Packet *(Element::*puller)(int) = &Element::pull;
            _bound.pull = (Packet *(*)(Element *, int)) (e->*puller);
# if HAVE_BATCH
             PacketBatch *(Element::*pullbatcher)(int,unsigned) = &Element::pull_batch;
             _bound_batch.pull_batch = (PacketBatch *(*)(Element *, int, unsigned)) (e->*pullbatcher);
# endif
        }
    }
#endif
}

inline void
Element::Port::assign(bool isoutput, Element *owner, Element *e, int port)
{
    PORT_ASSIGN(owner);
    assign(isoutput, e, port);
}

/** @brief Returns whether this port is active (a push output or a pull input).
 *
 * @sa Element::port_active
 */
inline bool
Element::Port::active() const
{
    return _port >= 0;
}

/** @brief Returns the element connected to this active port.
 *
 * Returns 0 if this port is not active(). */
inline Element*
Element::Port::element() const
{
    return _e;
}

/** @brief Returns the port number of the port connected to this active port.
 *
 * Returns < 0 if this port is not active(). */
inline int
Element::Port::port() const
{
    return _port;
}

/** @brief Push packet @a p over this port.
 *
 * Pushes packet @a p downstream through the router configuration by passing
 * it to the next element's @link Element::push() push() @endlink function.
 * Returns when the rest of the router finishes processing @a p.
 *
 * This port must be an active() push output port.  Usually called from
 * element code like @link Element::output output(i) @endlink .push(p).
 *
 * When element code calls Element::Port::push(@a p), it relinquishes control
 * of packet @a p.  When push() returns, @a p may have been altered or even
 * freed by downstream elements.  Thus, you must not use @a p after pushing it
 * downstream.  To push a copy and keep a copy, see Packet::clone().
 *
 * output(i).push(p) basically behaves like the following code, although it
 * maintains additional statistics depending on how CLICK_STATS is defined:
 *
 * @code
 * output(i).element()->push(output(i).port(), p);
 * @endcode
 */
inline void
Element::Port::push(Packet* p) const
{
    assert(_e && p);
#ifdef HAVE_AUTO_BATCH
    if (likely(_e->in_batch_mode == BATCH_MODE_YES)) {
        if (*current_batch != 0) {
            if (*current_batch == (PacketBatch*)-1) {
                *current_batch = PacketBatch::make_from_packet(p);
            } else {
                (*current_batch)->append_packet(p);
            }
        } else {
            push_batch(PacketBatch::make_from_packet(p));
        }
    } else {
        assert(!_e->receives_batch);
#else
    {
#endif
#if CLICK_STATS >= 1
    ++_packets;
#endif
#if CLICK_STATS >= 2
    ++_e->input(_port)._packets;
    click_cycles_t start_cycles = click_get_cycles(),
        start_child_cycles = _e->_child_cycles;
# if HAVE_BOUND_PORT_TRANSFER
    _bound.push(_e, _port, p);
# else
    _e->push(_port, p);
# endif
    click_cycles_t all_delta = click_get_cycles() - start_cycles,
        own_delta = all_delta - (_e->_child_cycles - start_child_cycles);
    _e->_xfer_calls += 1;
    _e->_xfer_own_cycles += own_delta;
    _owner->_child_cycles += all_delta;
#else
# if HAVE_BOUND_PORT_TRANSFER
    _bound.push(_e, _port, p);
# else
    _e->push(_port, p);
# endif
    }
#endif
}
/** @brief Pull a packet over this port and return it.
 *
 * Pulls a packet from upstream in the router configuration by calling the
 * previous element's @link Element::pull() pull() @endlink function.  When
 * the router finishes processing, returns the result.
 *
 * This port must be an active() pull input port.  Usually called from element
 * code like @link Element::input input(i) @endlink .pull().
 *
 * input(i).pull() basically behaves like the following code, although it
 * maintains additional statistics depending on how CLICK_STATS is defined:
 *
 * @code
 * input(i).element()->pull(input(i).port())
 * @endcode
 */
inline Packet*
Element::Port::pull() const
{
    assert(_e);
#if CLICK_STATS >= 2
    click_cycles_t start_cycles = click_get_cycles(),
        old_child_cycles = _e->_child_cycles;
# if HAVE_BOUND_PORT_TRANSFER
    Packet *p = _bound.pull(_e, _port);
# else
    Packet *p = _e->pull(_port);
# endif
    if (p)
        _e->output(_port)._packets += 1;
    click_cycles_t all_delta = click_get_cycles() - start_cycles,
        own_delta = all_delta - (_e->_child_cycles - old_child_cycles);
    _e->_xfer_calls += 1;
    _e->_xfer_own_cycles += own_delta;
    _owner->_child_cycles += all_delta;
#else
# if HAVE_BOUND_PORT_TRANSFER
    Packet *p = _bound.pull(_e, _port);
# else
    Packet *p = _e->pull(_port);
# endif
#endif
#if CLICK_STATS >= 1
    if (p)
        ++_packets;
#endif
    return p;
}

#if HAVE_BATCH
/**
 * Push a batch through this port
 */
void
Element::Port::push_batch(PacketBatch* batch) const {
#if BATCH_DEBUG
    click_chatter("Pushing batch of %d packets to %p{element}",batch->count(),_e);
#endif
#if HAVE_BOUND_PORT_TRANSFER
    _bound_batch.push_batch(_e,_port,batch);
#else
    _e->push_batch(_port,batch);
#endif
}

#ifdef HAVE_AUTO_BATCH
inline void
Element::Port::start_batch() {
    /**
     * Port mode : start batching if e is BATCH_MODE_YES
     * List mode : start batching (if e is not BATCH_MODE_YES, we would not be calling this)
     * Jump mode : pass the message through all ports if not BATCH_MODE_YES, if it is, set to -1 to start batching
     */
#if HAVE_AUTO_BATCH == AUTO_BATCH_JUMP
# if BATCH_DEBUG
    click_chatter("Passing start batch message in port to %p{element}",_e);
# endif
    if (_e->in_batch_mode == BATCH_MODE_YES)  {
        if (*current_batch == 0)
            current_batch.set((PacketBatch*)-1);
    } else {
        for (int i = 0; i < _e->noutputs(); i++) {
            if (_e->output_is_push(i))
                _e->_ports[1][i].start_batch();
        }
    }
#elif HAVE_AUTO_BATCH == AUTO_BATCH_PORT
# if BATCH_DEBUG
    click_chatter("Starting batch in port to %p{element}",_e);
# endif
    if (_e->in_batch_mode == BATCH_MODE_YES) { //Rebuild for the next element
        if (*current_batch == 0)
            current_batch.set((PacketBatch*)-1);
    }
#elif HAVE_AUTO_BATCH == AUTO_BATCH_LIST
# if BATCH_DEBUG
    click_chatter("Starting batch in port to %p{element}",_e);
# endif
    assert(*current_batch == 0);
    current_batch.set((PacketBatch*)-1);
#else
    #error "Unknown batch auto mode"
#endif
}

inline void
Element::Port::end_batch() {
    PacketBatch* cur = *current_batch;
#if BATCH_DEBUG
    click_chatter("Ending batch in port to %p{element}",_e);
#endif
    /**
     * Auto mode : if 0 bug, if -1 nothing, else push the batch
     * List mode : if 0 bug, if -1 nothing, else push the batch
     * Jump mode : if BATCH_MODE_YES push the batch, else pass the message (and assert cur was 0)
     */
#if HAVE_AUTO_BATCH == AUTO_BATCH_JUMP
    if (_e->in_batch_mode == BATCH_MODE_YES)
#endif
    { //Send the buffered batch to the next element
       if (cur != 0) {
           current_batch.set((PacketBatch*)0);
           if (cur != (PacketBatch*)-1)
           {
               #if BATCH_DEBUG
               assert(cur->find_count() == cur->count());
               #endif
               _e->push_batch(_port,cur);
           }
       }

    }
#if HAVE_AUTO_BATCH == AUTO_BATCH_JUMP
    else { //Pass the message
        assert(cur == 0);
        for (int i = 0; i < _e->noutputs(); i++) {
            if (_e->output_is_push(i))
                _e->_ports[1][i].end_batch();
        }
    }
#endif
};
#endif

PacketBatch*
Element::Port::pull_batch(unsigned max) const {
    PacketBatch* batch = NULL;
#if HAVE_BOUND_PORT_TRANSFER
    batch = _bound_batch.pull_batch(_e,_port, max);
#else
    batch = _e->pull_batch(_port, max);
#endif
    return batch;
}
#endif

/**
 * @brief Tell if the path up to this element is a full push path, always
 * served by the same thread.
 *
 * Hence, it is not only a matter of having
 * only a push path, as some elements like Pipeliner may be push
 * but lead to thread switch.
 *
 * @pre get_passing_threads() have to be called on this element or any downstream element
 *
 * If this element is part of a full push path, it means that packets passing
 *  through will always be handled by the same thread. They may be shared, in
 *  the sense that the usage count could be bigger than one. But then shared
 *  only with the same thread. Therefore non-atomic operations can be involved.
 */
inline bool Element::is_fullpush() const {
#if HAVE_FULLPUSH_NONATOMIC
    return _is_fullpush;
#else
    return false;
#endif
}

/** @brief Push packet @a p to output @a port, or kill it if @a port is out of
 * range.
 *
 * @param port output port number
 * @param p packet to push
 *
 * If @a port is in range (>= 0 and < noutputs()), then push packet @a p
 * forward using output(@a port).push(@a p).  Otherwise, kill @a p with @a p
 * ->kill().
 *
 * @note It is invalid to call checked_output_push() on a pull output @a port.
 */
inline void
Element::checked_output_push(int port, Packet* p) const
{
    PRINT("Element::checked_output_push: the element:port:");
    PRINT(this->class_name());
    PRINT(String(port).data());
    if ((unsigned) port < (unsigned) noutputs())
        _ports[1][port].push(p);
    else
        p->kill();
}

/** @brief Pull a packet from input @a port, or return 0 if @a port is out of
 * range.
 *
 * @param port input port number
 *
 * If @a port is in range (>= 0 and < ninputs()), then return the result
 * of input(@a port).pull().  Otherwise, return null.
 *
 * @note It is invalid to call checked_input_pull() on a push input @a port.
 */
inline Packet*
Element::checked_input_pull(int port) const
{
    PRINT("Element::checked_input_pull: the element:port:");
    PRINT(this->class_name());
    PRINT(String(port).data());
    if ((unsigned) port < (unsigned) ninputs())
        return _ports[0][port].pull();
    else
        return 0;
}


#endif




