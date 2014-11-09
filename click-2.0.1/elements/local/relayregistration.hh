#ifndef CLICK_RELAY_REGISTRATION_HH
#define CLICK_RELAY_REGISTRATION_HH

#include <click/element.hh>
#include <click/timer.hh>
#include "foreignagentinfobase.hh"

CLICK_DECLS

/**
* Element allows mobile agent acting as foreign agent to
* relay mobile registration request and reply messages
*/
class RelayRegistration: public Element {
public:
    RelayRegistration();
    ~RelayRegistration();
    
    const char *class_name() const { return "RelayRegistration"; }
    const char *port_count() const { return "1/1"; }
    const char *processing() const { return PUSH; }

    int configure(Vector<String>&, ErrorHandler*);

    void push(int, Packet*);

    void run_timer(Timer*);

private:
    ForeignAgentInfobase *_infobase;
    Timer _timer;
};

CLICK_ENDDECLS

#endif