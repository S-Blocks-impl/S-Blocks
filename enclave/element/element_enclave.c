#include "element_enclave.h"
#include "../manager_enclave.h"
#include "../lib/straccum.h"
#include "../lib/args.h"
#include "../lib/error.h"
#include "routervisitor.h"
const char Element::PORTS_0_0[] = "0";
const char Element::PORTS_0_1[] = "0/1";
const char Element::PORTS_1_0[] = "1/0";
const char Element::PORTS_1_1[] = "1";
const char Element::PORTS_1_1X2[] = "1/1-2";

const char Element::AGNOSTIC[] = "a";
const char Element::PUSH[] = "h";
const char Element::PULL[] = "l";
const char Element::PUSH_TO_PULL[] = "h/l";
const char Element::PULL_TO_PUSH[] = "l/h";
const char Element::PROCESSING_A_AH[] = "a/ah";

const char Element::COMPLETE_FLOW[] = "x/x";

int Element::nelements_allocated = 0;


// Element::Element()
//     : _manager(0), _eindex(-1) //修改成manager
// {
//     nelements_allocated++;
//     _ports[0] = _ports[1] = &_inline_ports[0];
//     _nports[0] = _nports[1] = 0;

// #if CLICK_STATS >= 2
//     reset_cycles();
// #endif
// }

Element::Element()
    :
#if HAVE_BATCH && (HAVE_AUTO_BATCH == AUTO_BATCH_PORT)
    in_batch_mode(BATCH_MODE_IFPOSSIBLE),
#else
    in_batch_mode(BATCH_MODE_NO),
#endif
    receives_batch(false),
    _manager(0), _eindex(-1)
#if HAVE_FULLPUSH_NONATOMIC
    ,_is_fullpush(false)
#endif

{
    nelements_allocated++;
    _ports[0] = _ports[1] = &_inline_ports[0];
    _nports[0] = _nports[1] = 0;

#if CLICK_STATS >= 2
    reset_cycles();
#endif
}


Element::~Element()
{
    nelements_allocated--;
    if (_ports[0] < _inline_ports || _ports[0] > _inline_ports + INLINE_PORTS)
    delete[] _ports[0];
    if (_ports[1] < _inline_ports || _ports[1] > _inline_ports + INLINE_PORTS)
    delete[] _ports[1];
}

//去掉了cast 和port_cast

/** @brief Return the element's name.
 *
 * This is the name used to declare the element in the router configuration,
 * with all compound elements expanded. */
String
Element::name() const
{
    String s;
     if (Manager_enclave *r = manager_enclave())//替换router
	 s = r->ename(_eindex);
    return (s ? s : String::make_stable("<unknown>", 9));
}

/*** @cond never */
/** @brief Return the element's name (deprecated).
 *
 * @deprecated This function is deprecated; use name() instead. */
String

Element::id() const
{
    return name();
}

//去掉了 landmark 

    String
Element::declaration() const
{
    return name() + " :: " + class_name();
}

//对set_nports进行了修改，只用于简单连接，暂时不用于自动词法分析
int
Element::set_nports(int new_ninputs, int new_noutputs)
{
    // exit on bad counts, or if already initialized
    //@zyh
 //    if (new_ninputs < 0 || new_noutputs < 0)
	// return -EINVAL;
 //    if (_router && _router->_have_connections) {
	// if (_router->_state >= Router::ROUTER_PREINITIALIZE)
	//     return -EBUSY;
	// _router->_have_connections = false;
 //    }

    // decide if inputs & outputs were inlined
 //    bool old_in_inline =
	// (_ports[0] >= _inline_ports && _ports[0] <= _inline_ports + INLINE_PORTS);
 //    bool old_out_inline =
	// (_ports[1] >= _inline_ports && _ports[1] <= _inline_ports + INLINE_PORTS);
 //    bool prefer_pull = (processing() == PULL);

 //    // decide if inputs & outputs should be inlined
 //    bool new_in_inline =
	// (new_ninputs == 0
	//  || new_ninputs + new_noutputs <= INLINE_PORTS
	//  || (new_ninputs <= INLINE_PORTS && new_noutputs > INLINE_PORTS)
	//  || (new_ninputs <= INLINE_PORTS && prefer_pull));
 //    bool new_out_inline =
	// (new_noutputs == 0
	//  || new_ninputs + new_noutputs <= INLINE_PORTS
	//  || (new_noutputs <= INLINE_PORTS && !new_in_inline));

 //    // create new port arrays
 //    Port *new_inputs;
 //    if (new_in_inline)
	// new_inputs = _inline_ports + (!new_out_inline || prefer_pull ? 0 : new_noutputs);
 //    else if (!(new_inputs = new Port[new_ninputs]))
	// return -ENOMEM;

 //    Port *new_outputs;
 //    if (new_out_inline)
	// new_outputs = _inline_ports + (!new_in_inline || !prefer_pull ? 0 : new_ninputs);
 //    else if (!(new_outputs = new Port[new_noutputs])) {
	// if (!new_in_inline)
	//     delete[] new_inputs;
	// return -ENOMEM;
 //    }

 //    // install information
 //    if (!old_in_inline)
	// delete[] _ports[0];
 //    if (!old_out_inline)
	// delete[] _ports[1];
	
    
    if(new_ninputs > 0)
    {
	    Port *new_inputs;
        new_inputs = new Port[new_ninputs];
        _ports[0] = new_inputs;
        _nports[0] = new_ninputs;
    }else{
        _ports[0] = NULL;
        _nports[0] = 0;
    }
    if(new_noutputs > 0)
    {
        Port *new_outputs;
        new_outputs = new Port[new_noutputs];
        _ports[1] = new_outputs;
        _nports[1] = new_noutputs;
    }else{
        _ports[1] = NULL;
        _nports[1] = 0;
    }

    //active the port
    for(int i = 0; i < _nports[0]; i++)
        _ports[0][i].assign(0,NULL,i);
    for(int i = 0; i < _nports[1]; i++)
        _ports[1][i].assign(1,NULL,i);
    return 0;
}


/** @brief Return the element's port count specifier.
 *
 * An element class overrides this virtual function to return a C string
 * describing its port counts.  The string gives acceptable input and output
 * ranges, separated by a slash.  Examples:
 *
 * <dl>
 * <dt><tt>"1/1"</tt></dt> <dd>The element has exactly one input port and one
 * output port.</dd>
 * <dt><tt>"1-2/0"</tt></dt> <dd>One or two input ports and zero output
 * ports.</dd>
 * <dt><tt>"1/-6"</tt></dt> <dd>One input port and up to six output ports.</dd>
 * <dt><tt>"2-/-"</tt></dt> <dd>At least two input ports and any number of
 * output ports.</dd>
 * <dt><tt>"3"</tt></dt> <dd>Exactly three input and output ports.  (If no
 * slash appears, the text is used for both input and output ranges.)</dd>
 * <dt><tt>"1-/="</tt></dt> <dd>At least one input port and @e the @e same
 * number of output ports.</dd>
 * <dt><tt>"1-/=+"</tt></dt> <dd>At least one input port and @e one @e more
 * output port than there are input ports.</dd>
 * </dl>
 *
 * Port counts are used to determine whether a configuration uses too few or
 * too many ports, and lead to errors such as "'e' has no input 3" and "'e'
 * input 3 unused".
 *
 * Click extracts port count specifiers from the source for use by tools.  For
 * Click to find a port count specifier, the function definition must appear
 * inline, on a single line, inside the element class's declaration, and must
 * return a C string constant (or a name below).  It should also have public
 * accessibility.  Here's an acceptable port_count() definition:
 *
 * @code
 * const char *port_count() const     { return "1/1"; }
 * @endcode
 *
 * The default port_count() method returns @c "0/0".
 *
 * The following names are available for common port count specifiers.
 *
 * @arg @c PORTS_0_0 for @c "0/0"
 * @arg @c PORTS_0_1 for @c "0/1"
 * @arg @c PORTS_1_0 for @c "1/0"
 * @arg @c PORTS_1_1 for @c "1/1"
 * @arg @c PORTS_1_1X2 for @c "1/1-2"
 *
 * Since port_count() should simply return a C string constant, it shouldn't
 * matter when it's called; nevertheless, it is called before configure().
 */
const char *
Element::port_count() const
{
    return PORTS_0_0;
}
//@zyh 去掉了，暂时不支持自动词法分析
// static int
// notify_nports_pair(const char *&s, const char *ends, int &lo, int &hi)
// {
//     if (s == ends || *s == '-')
// 	lo = 0;
//     else if (isdigit((unsigned char) *s))
// 	s = cp_integer(s, ends, 10, &lo);
//     else
// 	return -1;
//     if (s < ends && *s == '-') {
// 	s++;
// 	if (s < ends && isdigit((unsigned char) *s))
// 	    s = cp_integer(s, ends, 10, &hi);
// 	else
// 	    hi = INT_MAX;
//     } else
// 	hi = lo;
//     return 0;
// }

//去掉了，暂时不支持自动词法分析
// int
// Element::notify_nports(int ninputs, int noutputs, ErrorHandler *errh)
// {
//     // Another version of this function is in tools/lib/processingt.cc.
//     // Make sure you keep them in sync.
//     const char *s_in = port_count();
//     const char *s = s_in, *ends = s + strlen(s);
//     int ninlo, ninhi, noutlo, nouthi, equal = 0;

//     if (notify_nports_pair(s, ends, ninlo, ninhi) < 0)
// 	goto parse_error;

//     if (s == ends)
// 	s = s_in;
//     else if (*s == '/')
// 	s++;
//     else
// 	goto parse_error;

//     if (*s == '=') {
// 	const char *plus = s + 1;
// 	do {
// 	    equal++;
// 	} while (plus != ends && *plus++ == '+');
// 	if (plus != ends)
// 	    equal = 0;
//     }
//     if (!equal)
// 	if (notify_nports_pair(s, ends, noutlo, nouthi) < 0 || s != ends)
// 	    goto parse_error;

//     if (ninputs < ninlo)
// 	ninputs = ninlo;
//     else if (ninputs > ninhi)
// 	ninputs = ninhi;

//     if (equal)
// 	noutputs = ninputs + equal - 1;
//     else if (noutputs < noutlo)
// 	noutputs = noutlo;
//     else if (noutputs > nouthi)
// 	noutputs = nouthi;

//     set_nports(ninputs, noutputs);
//     return 0;

//   parse_error:
//     if (errh) 
// 	errh->error("%p{element}: bad port count", this);
//     return -1;
// }


void
Element::initialize_ports(const int *in_v, const int *out_v)
{
    for (int i = 0; i < ninputs(); i++) {
	// allowed iff in_v[i] == VPULL
	int port = (in_v[i] == VPULL ? 0 : -1);
	_ports[0][i].assign(false, this, 0, port);
    }

    for (int o = 0; o < noutputs(); o++) {
	// allowed iff out_v[o] != VPULL
	int port = (out_v[o] == VPULL ? -1 : 0);
	_ports[1][o].assign(true, this, 0, port);
    }
}

int
Element::connect_port(bool isoutput, int port, Element* e, int e_port)
{
    if (port_active(isoutput, port)) {
	_ports[isoutput][port].assign(isoutput, this, e, e_port);
	return 0;
    } else
	return -1;
}
//去掉了对flow的支持
//const char *
//Element::flow_code() const
//    static void
 //   skip_flow_code(const char*& p)
    
//static int
//next_flow_code(const char*& p, int port, Bitvector& code, ErrorHandler* errh, const Element* e)
//    void
//    Element::port_flow(bool isoutput, int p, Bitvector* travels) const




// PUSH OR PULL PROCESSING

/** @brief Return the element's processing specifier.
 *
 * An element class overrides this virtual function to return a C string
 * describing which of its ports are push, pull, or agnostic.  The string
 * gives acceptable input and output ranges, separated by a slash; the
 * characters @c "h", @c "l", and @c "a" indicate push, pull, and agnostic
 * ports, respectively.  Examples:
 *
 * @arg @c "h/h" All input and output ports are push.
 * @arg @c "h/l" Push input ports and pull output ports.
 * @arg @c "a/ah" All input ports are agnostic.  The first output port is also
 * agnostic, but the second and subsequent output ports are push.
 * @arg @c "hl/hlh" Input port 0 and output port 0 are push.  Input port 1 and
 * output port 1 are pull.  All remaining inputs are pull; all remaining
 * outputs are push.
 * @arg @c "a" All input and output ports are agnostic.  (If no slash appears,
 * the text is used for both input and output ports.)
 *
 * Thus, each character indicates a single port's processing type, except that
 * the last character in the input section is used for all remaining input
 * ports (and similarly for outputs).  It's OK to have more characters than
 * ports; any extra characters are ignored.
 *
 * Click extracts processing specifiers from the source for use by tools.  For
 * Click to find a processing specifier, the function definition must appear
 * inline, on a single line, inside the element class's declaration, and must
 * return a C string constant.  It should also have public accessibility.
 * Here's an acceptable processing() definition:
 *
 * @code
 * const char *processing() const     { return "a/ah"; }
 * @endcode
 *
 * The default processing() method returns @c "a/a", which sets all ports to
 * agnostic.
 *
 * The following names are available for common processing specifiers.
 *
 * @arg @c AGNOSTIC for @c "a/a"
 * @arg @c PUSH for @c "h/h"
 * @arg @c PULL for @c "l/l"
 * @arg @c PUSH_TO_PULL for @c "h/l"
 * @arg @c PULL_TO_PUSH for @c "l/h"
 * @arg @c PROCESSING_A_AH for @c "a/ah"
 *
 * Since processing() should simply return a C string constant, it shouldn't
 * matter when it's called; nevertheless, it is called before configure().
 */
const char*
Element::processing() const
{
    return AGNOSTIC;
}
int
Element::next_processing_code(const char*& p, ErrorHandler* errh)
{
    switch (*p) {

      case 'h': case 'H':
	p++;
	return Element::VPUSH;

      case 'l': case 'L':
	p++;
	return Element::VPULL;

      case 'a': case 'A':
	p++;
	return Element::VAGNOSTIC;

      case '/': case 0:
	return -2;

      default:
	if (errh)
	    errh->error("bad processing code");
	p++;
	return -1;

    }
}
void
Element::processing_vector(int* in_v, int* out_v, ErrorHandler* errh) const
{
    const char* p_in = processing();
    int val = 0;

    const char* p = p_in;
    int last_val = 0;
    for (int i = 0; i < ninputs(); i++) {
	if (last_val >= 0)
	    last_val = next_processing_code(p, errh);
	if (last_val >= 0)
	    val = last_val;
	in_v[i] = val;
    }

    while (*p && *p != '/')
	p++;
    if (!*p)
	p = p_in;
    else
	p++;

    last_val = 0;
    for (int i = 0; i < noutputs(); i++) {
	if (last_val >= 0)
	    last_val = next_processing_code(p, errh);
	if (last_val >= 0)
	    val = last_val;
	out_v[i] = val;
    }
}



const char*
Element::flags() const
{
    return "";
}

//去掉了
//int
//Element::flag_value(int flag) const

// CLONING AND CONFIGURING

/** @brief Return the element's configure phase, which determines the
 * order in which elements are configured and initialized.
 *
 * Click configures and initializes elements in increasing order of
 * configure_phase().  An element with configure phase 1 will always be
 * configured (have its configure() method called) before an element with
 * configure phase 2.  Thus, if two element classes must be configured in a
 * given order, they should define configure_phase() functions to enforce that
 * order.  For example, the @e AddressInfo element defines address
 * abbreviations for other elements to use; it should thus be configured
 * before other elements, and its configure_phase() method returns a low
 * value.
 *
 * Configure phases should be defined relative to the following constants,
 * which are listed in increasing order.
 *
 * <dl>
 * <dt><tt>CONFIGURE_PHASE_FIRST</tt></dt>
 * <dd>Configure before other elements.  Used by @e AddressInfo.</dd>
 *
 * <dt><tt>CONFIGURE_PHASE_INFO</tt></dt>
 * <dd>Configure early.  Appropriate for most information elements, such as @e ScheduleInfo.</dd>
 *
 * <dt><tt>CONFIGURE_PHASE_PRIVILEGED</tt></dt>
 * <dd>Intended for elements that require root
 * privilege when run at user level, such as @e FromDevice and
 * @e ToDevice.  The @e ChangeUID element, which reliquishes root
 * privilege, runs at configure phase @c CONFIGURE_PHASE_PRIVILEGED + 1.</dd>
 *
 * <dt><tt>CONFIGURE_PHASE_DEFAULT</tt></dt> <dd>The default implementation
 * returns @c CONFIGURE_PHASE_DEFAULT, so most elements are configured at this
 * phase.  Appropriate for most elements.</dd>
 *
 * <dt><tt>CONFIGURE_PHASE_LAST</tt></dt>
 * <dd>Configure after other elements.</dd>
 * </dl>
 *
 * The body of a configure_phase() method should consist of a single @c return
 * statement returning some constant.  Although it shouldn't matter when it's
 * called, it is called before configure().
 */
int
Element::configure_phase() const
{
    return CONFIGURE_PHASE_DEFAULT;
}


/** @brief Parse the element's configuration arguments.
 *
 * @param conf configuration arguments
 * @param errh error handler
 *
 * The configure() method is passed the element's configuration arguments.  It
 * should parse them, report any errors, and initialize the element's internal
 * state.
 *
 * The @a conf argument is the element's configuration string, divided into
 * configuration arguments by splitting at commas and removing comments and
 * leading and trailing whitespace (see cp_argvec()).  If @a conf is empty,
 * the element was not supplied with a configuration string (or its
 * configuration string contained only comments and whitespace).  It is safe
 * to modify @a conf; modifications will be thrown away when the function
 * returns.
 *
 * Any errors, warnings, or messages should be reported to @a errh.  Messages
 * need not specify the element name or type, since this information will be
 * provided as context.  @a errh.nerrors() is initially zero.
 *
 * configure() should return a negative number if configuration fails.
 * Returning a negative number prevents the router from initializing.  The
 * default configure() method succeeds if and only if there are no
 * configuration arguments.
 *
 * configure() methods are called in order of configure_phase().  All
 * elements' configure() methods are called, even if an early configure()
 * method fails; this is to report all relevant error messages to the user,
 * rather than just the first.
 *
 * configure() is called early in the initialization process, and cannot check
 * whether a named handler exists.  That function must be left for
 * initialize().  Assuming all router connections are valid and all
 * configure() methods succeed, the add_handlers() functions will be called
 * next.
 *
 * A configure() method should avoid potentially harmful actions, such
 * as truncating files or attaching to devices.  These actions should be left
 * for the initialize() method, which is called later.  This avoids harm if
 * another element cannot be configured, or if the router is incorrectly
 * connected, since in those cases initialize() will never be called.
 *
 * Elements that support live reconfiguration (see can_live_reconfigure())
 * should expect configure() to be called at run time, when a user writes to
 * the element's @c config handler.  In that case, configure() must be careful
 * not to disturb the existing configuration unless the new configuration is
 * error-free.
 *
 * @note In previous releases, configure() could not determine whether a port
 * is push or pull or query the router for information about neighboring
 * elements.  Those functions had to be left for initialize().  Even in the
 * current release, if any element in a configuration calls the deprecated
 * set_ninputs() or set_noutputs() function from configure(), then all push,
 * pull, and neighbor information is invalidated until initialize() time.
 *
 * @sa live_reconfigure, args.hh for argument parsing
 */
 //去掉ErrorHandler
int
Element::configure(stlpmtx_std::vector<String> &conf, ErrorHandler *errh)
{
    return Args(conf, this, errh).complete();
}



/** @brief Install the element's handlers.
 *
 * The add_handlers() method should install any handlers the element provides
 * by calling add_read_handler(), add_write_handler(), and set_handler().
 * These functions may also be called from configure(), initialize(), or even
 * later, during router execution.  However, it is better in most cases to
 * initialize handlers in configure() or add_handlers(), since elements that
 * depend on other handlers often check in initialize() whether those handlers
 * exist.
 *
 * add_handlers() is called after configure() and before initialize().  When
 * it runs, it is guaranteed that every configure() method succeeded and that
 * all connections are correct (push and pull match up correctly and there are
 * no unused or badly-connected ports).
 *
 * Most add_handlers() methods simply call add_read_handler(),
 * add_write_handler(), add_task_handlers(), and possibly set_handler() one or
 * more times.  The default add_handlers() method does nothing.
 *
 * Click automatically provides five handlers for each element: @c class, @c
 * name, @c config, @c ports, and @c handlers.  There is no need to provide
 * these yourself.
 */
void
Element::add_handlers()
{
}



/** @brief Initialize the element.
 *
 * @param errh error handler
 *
 * The initialize() method is called just before the router is placed on
 * line. It performs any final initialization, and provides the last chance to
 * abort router installation with an error.  Any errors, warnings, or messages
 * should be reported to @a errh.  Messages need not specify the element
 * name; this information will be supplied externally.  @a errh.nerrors()
 * is initially zero.
 *
 * initialize() should return zero if initialization succeeds, or a negative
 * number if it fails.  Returning a negative number prevents the router from
 * initializing.  The default initialize() method always returns zero
 * (success).
 *
 * initialize() methods are called in order of configure_phase(), using the
 * same order as for configure().  When an initialize() method fails, router
 * initialization stops immediately, and no more initialize() methods are
 * called.  Thus, at most one initialize() method can fail per router
 * configuration.
 *
 * initialize() is called after add_handlers() and before take_state().  When
 * it runs, it is guaranteed that every configure() method succeeded, that all
 * connections are correct (push and pull match up correctly and there are no
 * unused or badly-connected ports), and that every add_handlers() method has
 * been called.
 *
 * If every element's initialize() method succeeds, then the router is
 * installed, and will remain installed until another router replaces it.  Any
 * errors that occur later than initialize() -- during take_state(), push(),
 * or pull(), for example -- will not take the router off line.
 *
 * Strictly speaking, the only task that @e must go in initialize() is
 * checking whether a handler exists, since that information isn't available
 * at configure() time.  It's often convenient, however, to put other
 * functionality in initialize().  For example, opening files for writing fits
 * well in initialize(): if the configuration has errors before the relevant
 * element is initialized, any existing file will be left as is.  Common tasks
 * performed in initialize() methods include:
 *
 *   - Initializing Task objects.
 *   - Allocating memory.
 *   - Opening files.
 *   - Initializing network devices.
 *
 * @note initialize() methods may not create or destroy input and output
 * ports, but this functionality is deprecated anyway.
 *
 * @note In previous releases, configure() could not determine whether a port
 * was push or pull or query the router for information about neighboring
 * elements, so those tasks were relegated to initialize() methods.  In the
 * current release, configure() can perform these tasks too.
 */

 int
 Element::initialize(ErrorHandler *errh)
 {
     (void) errh;
     return 0;
 }

//去掉
//void
//Element::take_state(Element *old_element, ErrorHandler *errh)
//Element *
//Element::hotswap_element() const
//void
//Element::cleanup(CleanupStage stage)


// LIVE CONFIGURATION

/** @brief Return whether an element supports live reconfiguration.
 *
 * Returns true iff this element can be reconfigured as the router is running.
 * Click will make the element's "config" handler writable if
 * can_live_reconfigure() returns true; when that handler is written, Click
 * will call the element's live_reconfigure() function.  The default
 * implementation returns false.
 */
// bool
// Element::can_live_reconfigure() const
// {
//   return false;
// }

/** @brief Reconfigure the element while the router is running.
 *
 * @param conf configuration arguments
 * @param errh error handler
 *
 * This function should parse the configuration arguments in @a conf, set the
 * element's state accordingly, and report any error messages or warnings to
 * @a errh.  This resembles configure().  However, live_reconfigure() is
 * called when the element's "config" handler is written, rather than at
 * router initialization time.  Thus, the element already has a working
 * configuration.  If @a conf has an error, live_reconfigure() should leave
 * this previous working configuration alone.
 *
 * can_live_reconfigure() must return true for live_reconfigure() to work.
 *
 * Return >= 0 on success, < 0 on error.  On success, Click will set the
 * element's old configuration arguments to @a conf, so that later reads of
 * the "config" handler will return @a conf.  (An element can override this
 * by defining its own "config" handler.)
 *
 * The default implementation simply calls configure(@a conf, @a errh).  This
 * is OK as long as configure() doesn't change the element's state on error.
 *
 * @sa can_live_reconfigure
 */
// int 
// Element::live_reconfigure(Vector<String> &conf, ErrorHandler *errh)
// int
// Element::live_reconfigure(stlpmtx_std::vector<String> &conf)
// {
//   if (can_live_reconfigure())
//     return configure(conf);
//   // else
//   //   return errh->error("cannot reconfigure %p{element} live", this);
// }


/** @brief Return the element's current configuration string.
 *
 * The configuration string is obtained by calling the element's "config"
 * read handler.  The default read handler calls Router::econfiguration().
 */
// String
// Element::configuration() const
// {
//     if (const Handler *h = manager_enclave()->handler(this, "config"))
//     if (h->readable())
//         //return h->call_read(const_cast<Element *>(this), 0);
//         return h->call_read(const_cast<Element *>(this));
//     return manager_enclave()->econfiguration(eindex());
// }

//RouterThread *
//Element::home_thread() const
//    void
//    Element::selected(int fd, int mask)
//    void
 //   Element::selected(int fd)
//    int
//Element::add_select(int fd, int mask)
//    int
//    Element::remove_select(int fd, int mask)




// HANDLERS

/** @brief Register a read handler named @a name.
 *
 * @param name handler name
 * @param read_callback function called when handler is read
 * @param user_data user data parameter passed to @a read_callback
 * @param flags flags to set
 *
 * Adds a read handler named @a name for this element.  Reading the handler
 * returns the result of the @a read_callback function, which is called like
 * this:
 *
 * @code
 * String result = read_callback(e, user_data);
 * @endcode
 *
 * @a e is this element pointer.
 *
 * add_read_handler(@a name) overrides any previous
 * add_read_handler(@a name) or set_handler(@a name), but any previous
 * add_write_handler(@a name) remains in effect.
 *
 * The added read handler takes no parameters.  To create a read handler with
 * parameters, use set_handler() or set_handler_flags().
 *
 * @sa read_positional_handler, read_keyword_handler: standard read handler
 * callback functions
 * @sa add_write_handler, set_handler, add_task_handlers
 */
 void
 Element::add_read_handler(const String &name, ReadHandlerCallback read_callback, const void *user_data, uint32_t flags)
{
    Manager_enclave::add_read_handler(this, name, read_callback, (void *) user_data, flags);
}

/** @brief Register a read handler named @a name.
 *
 * This version of add_read_handler() is useful when @a user_data is an
 * integer.  Note that the @a read_callback function must still cast its
 * <tt>void *</tt> argument to <tt>intptr_t</tt> to obtain the integer value.
 */
void
Element::add_read_handler(const String &name, ReadHandlerCallback read_callback, int user_data, uint32_t flags)
{
    uintptr_t u = (uintptr_t) user_data;
    Manager_enclave::add_read_handler(this, name, read_callback, (void *) u, flags);
}

/** @brief Register a read handler named @a name.
 *
 * This version of add_read_handler() is useful when @a name is a static
 * constant string.  @a name is passed to String::make_stable.  The memory
 * referenced by @a name must remain valid for as long as the router containing
 * this element.
 */
void
Element::add_read_handler(const char *name, ReadHandlerCallback read_callback, int user_data, uint32_t flags)
{
    uintptr_t u = (uintptr_t) user_data;
    Manager_enclave::add_read_handler(this, String::make_stable(name), read_callback, (void *) u, flags);
}

/** @brief Register a write handler named @a name.
 *
 * @param name handler name
 * @param write_callback function called when handler is written
 * @param user_data user data parameter passed to @a write_callback
 * @param flags flags to set
 *
 * Adds a write handler named @a name for this element.  Writing the handler
 * calls the @a write_callback function like this:
 *
 * @code
 * int r = write_callback(data, e, user_data, errh);
 * @endcode
 *
 * @a e is this element pointer.  The return value @a r should be negative on
 * error, positive or zero on success.  Any messages should be reported to the
 * @a errh ErrorHandler object.
 *
 * add_write_handler(@a name) overrides any previous
 * add_write_handler(@a name) or set_handler(@a name), but any previous
 * add_read_handler(@a name) remains in effect.
 *
 * @sa reconfigure_positional_handler, reconfigure_keyword_handler: standard
 * write handler callback functions
 * @sa add_read_handler, set_handler, add_task_handlers
 */
void
Element::add_write_handler(const String &name, WriteHandlerCallback write_callback, const void *user_data, uint32_t flags)
{
    Manager_enclave::add_write_handler(this, name, write_callback, (void *) user_data, flags);
}

/** @brief Register a write handler named @a name.
 *
 * This version of add_write_handler() is useful when @a user_data is an
 * integer.  Note that the @a write_callback function must still cast its
 * <tt>void *</tt> argument to <tt>intptr_t</tt> to obtain the integer value.
 */
void
Element::add_write_handler(const String &name, WriteHandlerCallback write_callback, int user_data, uint32_t flags)
{
    uintptr_t u = (uintptr_t) user_data;
    Manager_enclave::add_write_handler(this, name, write_callback, (void *) u, flags);
}

/** @brief Register a write handler named @a name.
 *
 * This version of add_write_handler() is useful when @a name is a static
 * constant string.  @a name is passed to String::make_stable.  The memory
 * referenced by @a name must remain valid for as long as the router containing
 * this element.
 */
void
Element::add_write_handler(const char *name, WriteHandlerCallback write_callback, int user_data, uint32_t flags)
{
    uintptr_t u = (uintptr_t) user_data;
    Manager_enclave::add_write_handler(this, String::make_stable(name), write_callback, (void *) u, flags);
}

/** @brief Register a comprehensive handler named @a name.
 *
 * @param name handler name
 * @param flags handler flags
 * @param callback function called when handler is written
 * @param read_user_data read user data parameter stored in the handler
 * @param write_user_data write user data parameter stored in the handler
 *
 * Registers a comprehensive handler named @a name for this element.  The
 * handler handles the operations specified by @a flags, which can include
 * Handler::f_read, Handler::f_write, Handler::f_read_param, and others.
 * Reading the handler calls the @a callback function like this:
 *
 * @code
 * String data;
 * int r = callback(Handler::f_read, data, e, h, errh);
 * @endcode
 *
 * Writing the handler calls it like this:
 *
 * @code
 * int r = callback(Handler::f_write, data, e, h, errh);
 * @endcode
 *
 * @a e is this element pointer, and @a h points to the Handler object for
 * this handler.  The @a data string is an out parameter for reading and an in
 * parameter for writing; when reading with parameters, @a data has the
 * parameters on input and should be replaced with the result on output.  The
 * return value @a r should be negative on error, positive or zero on success.
 * Any messages should be reported to the @a errh ErrorHandler object.
 *
 * set_handler(@a name) overrides any previous
 * add_read_handler(@a name), add_write_handler(@a name), or set_handler(@a
 * name).
 */
void
Element::set_handler(const String& name, int flags, HandlerCallback callback, const void *read_user_data, const void *write_user_data)
{
    Manager_enclave::set_handler(this, name, flags, callback, (void *) read_user_data, (void *) write_user_data);
}

/** @brief Register a comprehensive handler named @a name.
 *
 * This version of set_handler() is useful when @a user_data is an integer.
 * Note that the Handler::user_data() methods still return <tt>void *</tt>
 * values.
 */
void
Element::set_handler(const String &name, int flags, HandlerCallback callback, int read_user_data, int write_user_data)
{
    uintptr_t u1 = (uintptr_t) read_user_data, u2 = (uintptr_t) write_user_data;
    Manager_enclave::set_handler(this, name, flags, callback, (void *) u1, (void *) u2);
}

/** @brief Register a comprehensive handler named @a name.
 *
 * This version of set_handler() is useful when @a name is a static
 * constant string.  @a name is passed to String::make_stable.  The memory
 * referenced by @a name must remain valid for as long as the router containing
 * this element.
 */
void
Element::set_handler(const char *name, int flags, HandlerCallback callback, int read_user_data, int write_user_data)
{
    uintptr_t u1 = (uintptr_t) read_user_data, u2 = (uintptr_t) write_user_data;
    Manager_enclave::set_handler(this, String::make_stable(name), flags, callback, (void *) u1, (void *) u2);
}

/** @brief Set flags for the handler named @a name.
 * @param name handler name
 * @param set_flags handler flags to set
 * @param clear_flags handler flags to clear
 *
 * Sets flags for any handlers named @a name for this element.  Fails if no @a
 * name handler exists.
 */
int
Element::set_handler_flags(const String& name, int set_flags, int clear_flags)
{
    return Manager_enclave::set_handler_flags(this, name, set_flags, clear_flags);
}

//*被 -add_default_handlers调用

static String
read_class_handler(Element *e, void *)
{
    return String(e->class_name());
}

static String
read_name_handler(Element *e, void *)
{
    return e->name();
}
//@zyh
static String
read_config_handler(Element *e, void *)
{
    return e->manager_enclave()->econfiguration(e->eindex());
}
//error
// static int
// write_config_handler(const String &str, Element *e, void *)
// {
//     stlpmtx_std::vector<String> conf;
//     cp_argvec(str, conf);
//     int r = e->live_reconfigure(conf);
//     if (r >= 0)
// 	e->manager_enclave()->set_econfiguration(e->eindex(), str);
//     return r;
// }

// static String
// read_ports_handler(Element *e, void *)
// {
//     return e->manager_enclave()->element_ports_string(e);
// }

String
Element::read_handlers_handler(Element *e, void *)
{
    stlpmtx_std::vector<int> hindexes;
    Manager_enclave::element_hindexes(e, hindexes);
    StringAccum sa;
    for (int* hip = hindexes.begin(); hip < hindexes.end(); hip++) {
	const Handler* h = Manager_enclave::handler(e->manager_enclave(), *hip);
	if (h->read_visible() || h->write_visible()) {
	    sa << h->name() << '\t';
	    if (h->read_visible())
		sa << 'r';
	    if (h->write_visible())
		sa << 'w';
	    if (h->read_param())
		sa << '+';
	    if (h->flags() & Handler::f_raw)
		sa << '%';
	    if (h->flags() & Handler::f_calm)
		sa << '.';
	    if (h->flags() & Handler::f_expensive)
		sa << '$';
	    if (h->flags() & Handler::f_uncommon)
		sa << 'U';
	    if (h->flags() & Handler::f_deprecated)
		sa << 'D';
	    if (h->flags() & Handler::f_button)
		sa << 'b';
	    if (h->flags() & Handler::f_checkbox)
		sa << 'c';
	    sa << '\n';
	}
    }
    return sa.take_string();
}


#if CLICK_STATS >= 1

static String
read_icounts_handler(Element *f, void *)
{
  StringAccum sa;
  for (int i = 0; i < f->ninputs(); i++)
    if (f->input(i).active() || CLICK_STATS >= 2)
      sa << f->input(i).npackets() << "\n";
    else
      sa << "??\n";
  return sa.take_string();
}

static String
read_ocounts_handler(Element *f, void *)
{
  StringAccum sa;
  for (int i = 0; i < f->noutputs(); i++)
    if (f->output(i).active() || CLICK_STATS >= 2)
      sa << f->output(i).npackets() << "\n";
    else
      sa << "??\n";
  return sa.take_string();
}

#endif /* CLICK_STATS >= 1 */

#if CLICK_STATS >= 2
String
Element::read_cycles_handler(Element *e, void *)
{
    StringAccum sa;
    if (e->_task_calls)
	sa << "tasks " << e->_task_calls << ' ' << e->_task_own_cycles << '\n';
    if (e->_timer_calls)
	sa << "timers " << e->_timer_calls << ' ' << e->_timer_own_cycles << '\n';
    if (e->_xfer_calls)
	sa << "xfer " << e->_xfer_calls << ' ' << e->_xfer_own_cycles << '\n';
    return sa.take_string();
}

// int
// Element::write_cycles_handler(const String &, Element *e, void *, ErrorHandler *)
// {
//     e->reset_cycles();
//     return 0;
// }
#endif



void
Element::add_default_handlers(bool allow_write_config)
{
  add_read_handler("name", read_name_handler, 0, Handler::f_calm);
  add_read_handler("class", read_class_handler, 0, Handler::f_calm);
  add_read_handler("config", read_config_handler, 0, Handler::f_calm);
//  if (allow_write_config && can_live_reconfigure())
//    add_write_handler("config", write_config_handler, 0);
//  add_read_handler("ports", read_ports_handler, 0, Handler::f_calm);
  add_read_handler("handlers", read_handlers_handler, 0, Handler::f_calm);
#if CLICK_STATS >= 1
  add_read_handler("icounts", read_icounts_handler, 0);
  add_read_handler("ocounts", read_ocounts_handler, 0);
# if CLICK_STATS >= 2
  add_read_handler("cycles", read_cycles_handler, 0);
  add_write_handler("cycles", write_cycles_handler, 0);
# endif
#endif
}
static int
uint8_t_data_handler(int op, String &str, Element *element, const Handler *h, ErrorHandler *errh)
{
    uint8_t *ptr = reinterpret_cast<uint8_t *>(reinterpret_cast<uintptr_t>(element) + reinterpret_cast<uintptr_t>(h->user_data(op)));
    int x;
    if (op == Handler::f_read) {
	str = String((int) *ptr);
	return 0;
    } else if (IntArg().parse(str, x) && x >= 0 && x < 256) {
	*ptr = x;
	return 0;
    } 
     else
	 return errh->error("expected uint8_t");
}

static int
bool_data_handler(int op, String &str, Element *element, const Handler *h, ErrorHandler *errh)
{
    bool *ptr = reinterpret_cast<bool *>(reinterpret_cast<uintptr_t>(element) + reinterpret_cast<uintptr_t>(h->user_data(op)));
    if (op == Handler::f_read) {
	str = String(*ptr);
	return 0;
    } else if (BoolArg().parse(str, *ptr))
	return 0;
    else
	return errh->error("expected boolean");
}
static int
uint16_t_data_handler(int op, String &str, Element *element, const Handler *h, ErrorHandler *errh)
{
    uint16_t *ptr = reinterpret_cast<uint16_t *>(reinterpret_cast<uintptr_t>(element) + reinterpret_cast<uintptr_t>(h->user_data(op)));
    int x;
    if (op == Handler::f_read) {
	str = String((int) *ptr);
	return 0;
    } else if (IntArg().parse(str, x) && x >= 0 && x < 65536) {
	*ptr = x;
	return 0;
    }
     else
	return errh->error("expected uint16_t");
}
static int
uint16_t_net_data_handler(int op, String &str, Element *element, const Handler *h, ErrorHandler *errh)
{
    uint16_t *ptr = reinterpret_cast<uint16_t *>(reinterpret_cast<uintptr_t>(element) + reinterpret_cast<uintptr_t>(h->user_data(op)));
    int x;
    if (op == Handler::f_read) {
	str = String((int) ntohs(*ptr));
	return 0;
    } else if (IntArg().parse(str, x) && x >= 0 && x < 65536) {
	*ptr = htons(x);
	return 0;
    } 
    else
	return errh->error("expected uint16_t");
}

static int
uint32_t_net_data_handler(int op, String &str, Element *element, const Handler *h, ErrorHandler *errh)
{
    uint32_t *ptr = reinterpret_cast<uint32_t *>(reinterpret_cast<uintptr_t>(element) + reinterpret_cast<uintptr_t>(h->user_data(op)));
    uint32_t x;
    if (op == Handler::f_read) {
	str = String(ntohl(*ptr));
	return 0;
    } else if (IntArg().parse(str, x)) {
	*ptr = htonl(x);
	return 0;
    } 
    else
	return errh->error("expected integer");
}
template <typename T> static int
integer_data_handler(int op, String &str, Element *element, const Handler *h, ErrorHandler *errh)
{
    T *ptr = reinterpret_cast<T *>(reinterpret_cast<uintptr_t>(element) + reinterpret_cast<uintptr_t>(h->user_data(op)));
    if (op == Handler::f_read) {
	str = String(*ptr);
	return 0;
    } else if (IntArg().parse(str, *ptr))
	return 0;
    else
	return errh->error("expected integer");
}

static int
atomic_uint32_t_data_handler(int op, String &str, Element *element, const Handler *h, ErrorHandler *errh)
{
    atomic_uint32_t *ptr = reinterpret_cast<atomic_uint32_t *>(reinterpret_cast<uintptr_t>(element) + reinterpret_cast<uintptr_t>(h->user_data(op)));
    uint32_t value;
    if (op == Handler::f_read) {
	str = String(ptr->value());
	return 0;
    } else if (IntArg().parse(str, value)) {
	*ptr = value;
	return 0;
    } 
    else
	return errh->error("expected integer");
}

#if HAVE_FLOAT_TYPES
static int
double_data_handler(int op, String &str, Element *element, const Handler *h, ErrorHandler *errh)
{
    double *ptr = reinterpret_cast<double *>(reinterpret_cast<uintptr_t>(element) + reinterpret_cast<uintptr_t>(h->user_data(op)));
    if (op == Handler::f_read) {
	str = String(*ptr);
	return 0;
    } else if (DoubleArg().parse(str, *ptr))
	return 0;
    else
	return errh->error("expected real number");
}
#endif

static int
string_data_handler(int op, String &str, Element *element, const Handler *h, ErrorHandler *)
{
    String *ptr = reinterpret_cast<String *>(reinterpret_cast<uintptr_t>(element) + reinterpret_cast<uintptr_t>(h->user_data(op)));
    if (op == Handler::f_read)
	str = *ptr;
    else
	*ptr = str;
    return 0;
}

static int
ip_address_data_handler(int op, String &str, Element *element, const Handler *h, ErrorHandler *errh)
{
    IPAddress *ptr = reinterpret_cast<IPAddress *>(reinterpret_cast<uintptr_t>(element) + reinterpret_cast<uintptr_t>(h->user_data(op)));
    if (op == Handler::f_read) {
	str = ptr->unparse();
	return 0;
    } if (IPAddressArg().parse(str, *ptr, element))
	return 0;
    else
	return errh->error("expected IP address");
}

// static int
// ether_address_data_handler(int op, String &str, Element *element, const Handler *h, ErrorHandler *errh)
// {
//     EtherAddress *ptr = reinterpret_cast<EtherAddress *>(reinterpret_cast<uintptr_t>(element) + reinterpret_cast<uintptr_t>(h->user_data(op)));
//     if (op == Handler::f_read) {
// 	str = ptr->unparse();
// 	return 0;
//     } else if (cp_ethernet_address(str, ptr, element))
// 	return 0;
//     else
// 	return errh->error("expected Ethernet address");
// }
static int
timestamp_data_handler(int op, String &str, Element *element, const Handler *h,ErrorHandler *errh)
{
    Timestamp *ptr = reinterpret_cast<Timestamp *>(reinterpret_cast<uintptr_t>(element) + reinterpret_cast<uintptr_t>(h->user_data(op)));
    if (op == Handler::f_read) {
	str = ptr->unparse();
	return 0;
    } else if (cp_time(str, ptr))
	return 0;
    else
	return errh->error("expected timestamp");
}
static int
interval_data_handler(int op, String &str, Element *element, const Handler *h, ErrorHandler *errh)
{
    Timestamp *ptr = reinterpret_cast<Timestamp *>(reinterpret_cast<uintptr_t>(element) + reinterpret_cast<uintptr_t>(h->user_data(op)));
    if (op == Handler::f_read) {
	str = ptr->unparse_interval();
	return 0;
    } else if (cp_time(str, ptr, true))
	return 0;
    else
	return errh->error("expected time in seconds");
}

inline void
Element::add_data_handlers(const char *name, int flags, HandlerCallback callback, void *data)
{
    uintptr_t x = reinterpret_cast<uintptr_t>(data) - reinterpret_cast<uintptr_t>(this);
    set_handler(name, flags, callback, x, x);
}

/** @brief Register read and/or write handlers accessing @a data.
 *
 * @param name handler name
 * @param flags handler flags, containing at least one of Handler::f_read
 * and Handler::f_write
 * @param data pointer to data
 *
 * Registers read and/or write handlers named @a name for this element.  If
 * (@a flags & Handler::f_read), registers a read handler; if (@a flags &
 * Handler::f_write), registers a write handler.  These handlers read or set
 * the data stored at @a *data, which might, for example, be an element
 * instance variable.  This data is unparsed and/or parsed using the expected
 * functions; for example, the <tt>bool</tt> version uses BoolArg::unparse()
 * and BoolArg::parse().  @a name is passed to String::make_stable.  The memory
 * referenced by @a name must remain valid for as long as the router containing
 * this element.
 *
 * Overloaded versions of this function are available for many fundamental
 * data types.
 */
void
Element::add_data_handlers(const char *name, int flags, uint8_t *data)
{
    add_data_handlers(name, flags, uint8_t_data_handler, data);
}

/** @overload */
void
Element::add_data_handlers(const char *name, int flags, bool *data)
{
    add_data_handlers(name, flags, bool_data_handler, data);
}

/** @overload */
void
Element::add_data_handlers(const char *name, int flags, uint16_t *data)
{
    add_data_handlers(name, flags, uint16_t_data_handler, data);
}

/** @overload */
void
Element::add_data_handlers(const char *name, int flags, int *data)
{
    add_data_handlers(name, flags, integer_data_handler<int>, data);
}

/** @overload */
void
Element::add_data_handlers(const char *name, int flags, unsigned *data)
{
    add_data_handlers(name, flags, integer_data_handler<unsigned>, data);
}

/** @overload */
void
Element::add_data_handlers(const char *name, int flags, atomic_uint32_t *data)
{
    add_data_handlers(name, flags, atomic_uint32_t_data_handler, data);
}

/** @overload */
void
Element::add_data_handlers(const char *name, int flags, long *data)
{
    add_data_handlers(name, flags, integer_data_handler<long>, data);
}

/** @overload */
void
Element::add_data_handlers(const char *name, int flags, unsigned long *data)
{
    add_data_handlers(name, flags, integer_data_handler<unsigned long>, data);
}

#if HAVE_LONG_LONG
/** @overload */
void
Element::add_data_handlers(const char *name, int flags, long long *data)
{
    add_data_handlers(name, flags, integer_data_handler<long long>, data);
}

/** @overload */
void
Element::add_data_handlers(const char *name, int flags, unsigned long long *data)
{
    add_data_handlers(name, flags, integer_data_handler<unsigned long long>, data);
}
#endif

#if HAVE_FLOAT_TYPES
/** @overload */
void
Element::add_data_handlers(const char *name, int flags, double *data)
{
    add_data_handlers(name, flags, double_data_handler, data);
}
#endif

// /** @overload */
void
Element::add_data_handlers(const char *name, int flags, IPAddress *data)
{
    add_data_handlers(name, flags, ip_address_data_handler, data);
}

// /** @overload */
// void
// Element::add_data_handlers(const char *name, int flags, EtherAddress *data)
// {
//     add_data_handlers(name, flags, ether_address_data_handler, data);
// }

/** @brief Register read and/or write handlers accessing @a data.
 *
 * This function's read handler returns *@a data unchanged, and its write
 * handler sets *@a data to the input string as received, without unquoting or
 * removing leading and trailing whitespace.
//  */
void
Element::add_data_handlers(const char *name, int flags, String *data)
{
    add_data_handlers(name, flags, string_data_handler, data);
}

/** @brief Register read and/or write handlers accessing @a data.
 * @param name handler name
 * @param flags handler flags, containing at least one of Handler::f_read
 * and Handler::f_write
 * @param data pointer to data
 * @param is_interval If true, the read handler unparses *@a data as an
 *   interval. */
void
Element::add_data_handlers(const char *name, int flags, Timestamp *data,
			   bool is_interval)
{
    if (is_interval)
	add_data_handlers(name, flags, interval_data_handler, data);
    else
	add_data_handlers(name, flags, timestamp_data_handler, data);
}




void
Element::push(int port, Packet *p)
{
 p = simple_action(p);
 if (p)
 output(port).push(p);
}

/** @brief Pull a packet from pull output @a port.
*
* @param port the output port number receiving the pull request.
* @return a packet
*
* A downstream element initiated a packet transfer from this element over a
* pull connection.  This element should return a packet pointer, or null if
* no packet is available.  The pull request arrived on output port @a port.
*
* Often, pull() methods will request packets from upstream using
* input(i).pull().  The default implementation calls simple_action().
*/
Packet *
Element::pull(int port)
{
 Packet *p = input(port).pull();
 if (p)
 p = simple_action(p);
 return p;
}

/////@hsr  delete some auto_batch 
#if HAVE_BATCH
void Element::push_batch(int port, PacketBatch* batch) {
// #if HAVE_AUTO_BATCH == AUTO_BATCH_PORT || HAVE_AUTO_BATCH == AUTO_BATCH_JUMP
//     for (int i = 0; i < noutputs(); i++) {
//         if (output_is_push(i))
//             _ports[1][i].start_batch();
//     }
// #elif HAVE_AUTO_BATCH == AUTO_BATCH_LIST
//     for each e in list
//         e->start_batch();
// #endif
    FOR_EACH_PACKET_SAFE(batch,p) {
        push(port,p);
    }
// #if HAVE_AUTO_BATCH == AUTO_BATCH_PORT || HAVE_AUTO_BATCH == AUTO_BATCH_JUMP
//     for (int i = 0; i < noutputs(); i++) {
//         if (output_is_push(i))
//             _ports[1][i].end_batch();
//     }
// #elif HAVE_AUTO_BATCH == AUTO_BATCH_LIST
//     for each e in list
//         e->end_batch();
// #endif
}

PacketBatch* Element::pull_batch(int port, unsigned max) {
    PacketBatch* batch;
    MAKE_BATCH(pull(port),batch,max);
    return batch;
}
#endif

/** @brief Process a packet for a simple packet filter.
*
* @param p the input packet
* @return the output packet, or null
*
* Many elements act as simple packet filters: they receive a packet from
* upstream using input 0, process that packet, and forward it downstream
* using output 0.  The simple_action() method automates this process.  The @a
* p argument is the input packet.  simple_action() should process the packet
* and return a packet pointer -- either the same packet, a different packet,
* or null.  If the return value isn't null, Click will forward that packet
* downstream.
*
* simple_action() must account for @a p, either by returning it, by freeing
* it, or by emitting it on some alternate push output port.  (An optional
* second push output port 1 is often used to emit erroneous packets.)
*
* simple_action() works equally well for push or pull port pairs.  The
* default push() method calls simple_action() this way:
*
* @code
* if ((p = simple_action(p)))
*     output(0).push(p);
* @endcode
*
* The default pull() method calls it this way instead:
*
* @code
* if (Packet *p = input(0).pull())
*     if ((p = simple_action(p)))
*         return p;
* return 0;
* @endcode
*
* An element that implements its processing with simple_action() should have
* a processing() code like AGNOSTIC or "a/ah", and a flow_code() like
* COMPLETE_FLOW or "x/x" indicating that packets can flow between the first
* input and the first output.
*
* Most elements that use simple_action() have exactly one input and one
* output.  However, simple_action() may be used for any number of inputs and
* outputs; a packet arriving on input port P will be emitted or output port
* P.
*
* For technical branch prediction-related reasons, elements that use
* simple_action() can perform quite a bit slower than elements that use
* push() and pull() directly.  The devirtualizer (click-devirtualize) can
* mitigate this effect.
*/
Packet *
Element::simple_action(Packet *p)
{
 return p;
}



