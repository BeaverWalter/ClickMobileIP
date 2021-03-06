#ifndef CLICK_MOBILENODEINFOBASE_HH
#define CLICK_MOBILENODEINFOBASE_HH
#include <click/element.hh>
#include <click/hashmap.hh>

CLICK_DECLS

struct pending_request {
    // IP destination address of request
    in_addr ip_dst;
    // COA used in registration
    uint32_t co_addr;
    // identification value sent in registration
    uint64_t id;
    // originally requested lifetime
    uint16_t requested_lifetime;
    // remaining lifetime
    uint16_t remaining_lifetime;
    // source port on which mobile node is listening
    uint16_t src_port;
};

class MobileNodeInfobase : public Element {
    public:
        MobileNodeInfobase();
        ~MobileNodeInfobase();

        const char *class_name() const { return "MobileNodeInfobase"; }
        const char *port_count() const { return "0/0"; }

        int configure(Vector<String>&, ErrorHandler*);

    public:
        IPAddress homeAgent;
        IPAddress homeAddress;

        bool      connected;
        IPAddress foreignAgent;
        uint16_t  lifetime;

        HashMap<IPAddress, Packet*> advertisements;
        Vector<pending_request> pending;
};

CLICK_ENDDECLS
#endif
