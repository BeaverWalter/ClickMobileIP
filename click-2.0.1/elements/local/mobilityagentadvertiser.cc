#include <time.h>
#include <stdlib.h>
#include <click/config.h>
#include <click/confparse.hh>
#include <click/error.hh>
#include <clicknet/ether.h>
#include "mobilityagentadvertiser.hh"

CLICK_DECLS
MobilityAgentAdvertiser::MobilityAgentAdvertiser()
    : _interval(0), _timer(this), _sequenceNr(0)
{
    srand(time(NULL));
}

MobilityAgentAdvertiser::~MobilityAgentAdvertiser()
{}

int MobilityAgentAdvertiser::configure(Vector<String> &conf, ErrorHandler *errh) {

    bool maxAdvertisementIntervalGiven;
    bool minAdvertisementIntervalGiven;
    bool advertisementLifetimeGiven;
    bool registrationLifetimeGiven;
    bool busyGiven;

    if (cp_va_kparse(conf, this, errh,
                     "SRC_IP", cpkM, cpIPAddress, &_srcIp,
                     "CARE_OF_ADDRESS", cpkM, cpIPAddress, &_careOfAddress,
                     "HOME_AGENT", cpkM, cpBool, &_homeAgent,
                     "FOREIGN_AGENT", cpkM, cpBool, &_foreignAgent,
                     "MaxAdvertisementInterval", cpkC, &maxAdvertisementIntervalGiven, cpUnsigned, &_maxAdvertisementInterval,
                     "MinAdvertisementInterval", cpkC, &minAdvertisementIntervalGiven, cpUnsigned, &_minAdvertisementInterval,
                     "AdvertisementLifetime", cpkC, &advertisementLifetimeGiven, cpUnsigned, &_advertisementLifetime,
                     "RegistrationLifetime", cpkC, &registrationLifetimeGiven, cpUnsigned, &_registrationLifetime,
                     "Busy", cpkC, &busyGiven, cpBool, &_busy,
                     cpEnd) < 0)
        return -1;

    if (!maxAdvertisementIntervalGiven)
        _maxAdvertisementInterval = 600;
    if (!minAdvertisementIntervalGiven)
        _minAdvertisementInterval = (unsigned)(0.75 * _maxAdvertisementInterval);
    if (!advertisementLifetimeGiven)
        _advertisementLifetime = 3 * _maxAdvertisementInterval;
    if (!registrationLifetimeGiven)
        _registrationLifetime = 0xffff;
    if (!busyGiven)
        _busy = false;

    if (!_homeAgent && !_foreignAgent)
    {
        errh->error("Either home agent or foreign agent option has to be set!");
        return -1;
    }
    if (_maxAdvertisementInterval < 4 || _maxAdvertisementInterval > 1800)
    {
        errh->error("MaxAdvertisementInterval must be no less than 4 seconds and no greater than 1800.");
        return -1;
    }
    if (_minAdvertisementInterval < 3 || _minAdvertisementInterval > _maxAdvertisementInterval)
    {
        errh->error("MinAdvertisementInterval must be no less than 3 seconds and no greater than MaxAdvertisementInterval.");
        return -1;
    }
    if (_advertisementLifetime < _maxAdvertisementInterval || _advertisementLifetime > 9000)
    {
        errh->error("AdvertisementLifetime must be no less than MaxAdvertisementInterval and no greater than 9000 seconds.");
        return -1;
    }
    if (_maxAdvertisementInterval * 3 > _advertisementLifetime)
    {
        errh->error("The interval at which Agent Advertisements are sent SHOULD be no longer than 1/3 of the advertisement Lifetime.");
        return -1;
    }
    if (_busy && !_foreignAgent)
    {
        errh->error("Only foreign agents can set the busy bit!");
        return -1;
    }

    // Send an advertisement immediately after the router starts
    _timer.initialize(this);
    _timer.schedule_after_msec(0);

    return 0;
}

String MobilityAgentAdvertiser::readMaxAdvertisementInterval(Element* e, void* thunk)
{
    MobilityAgentAdvertiser* me = (MobilityAgentAdvertiser*)e;
    return String(me->_maxAdvertisementInterval);
}

String MobilityAgentAdvertiser::readMinAdvertisementInterval(Element* e, void* thunk)
{
    MobilityAgentAdvertiser* me = (MobilityAgentAdvertiser*)e;
    return String(me->_minAdvertisementInterval);
}

String MobilityAgentAdvertiser::readAdvertisementLifetime(Element* e, void* thunk)
{
    MobilityAgentAdvertiser* me = (MobilityAgentAdvertiser*)e;
    return String(me->_advertisementLifetime);
}

String MobilityAgentAdvertiser::readRegistrationLifetime(Element* e, void* thunk)
{
    MobilityAgentAdvertiser* me = (MobilityAgentAdvertiser*)e;
    return String(me->_registrationLifetime);
}

String MobilityAgentAdvertiser::readBusy(Element* e, void* thunk)
{
    MobilityAgentAdvertiser* me = (MobilityAgentAdvertiser*)e;
    return String(me->_busy);
}

int MobilityAgentAdvertiser::writeMaxAdvertisementInterval(const String& conf, Element* e, void* thunk, ErrorHandler* errh)
{
    MobilityAgentAdvertiser* me = (MobilityAgentAdvertiser*)e;
    if(cp_va_kparse(conf, me, errh, "MaxAdvertisementInterval", cpkM + cpkP, cpUnsigned, &me->_maxAdvertisementInterval, cpEnd) < 0)
        return -1;
    return 0;
}

int MobilityAgentAdvertiser::writeMinAdvertisementInterval(const String& conf, Element* e, void* thunk, ErrorHandler* errh)
{
    MobilityAgentAdvertiser* me = (MobilityAgentAdvertiser*)e;
    if(cp_va_kparse(conf, me, errh, "MinAdvertisementInterval", cpkM + cpkP, cpUnsigned, &me->_minAdvertisementInterval, cpEnd) < 0)
        return -1;
    return 0;
}

int MobilityAgentAdvertiser::writeAdvertisementLifetime(const String& conf, Element* e, void* thunk, ErrorHandler* errh)
{
    MobilityAgentAdvertiser* me = (MobilityAgentAdvertiser*)e;
    if(cp_va_kparse(conf, me, errh, "AdvertisementLifetime", cpkM + cpkP, cpUnsigned, &me->_advertisementLifetime, cpEnd) < 0)
        return -1;
    return 0;
}

int MobilityAgentAdvertiser::writeRegistrationLifetime(const String& conf, Element* e, void* thunk, ErrorHandler* errh)
{
    MobilityAgentAdvertiser* me = (MobilityAgentAdvertiser*)e;
    if(cp_va_kparse(conf, me, errh, "RegistrationLifetime", cpkM + cpkP, cpUnsigned, &me->_registrationLifetime, cpEnd) < 0)
        return -1;
    return 0;
}

int MobilityAgentAdvertiser::writeBusy(const String& conf, Element* e, void* thunk, ErrorHandler* errh)
{
    MobilityAgentAdvertiser* me = (MobilityAgentAdvertiser*)e;
    if(cp_va_kparse(conf, me, errh, "Busy", cpkM + cpkP, cpBool, &me->_busy, cpEnd) < 0)
        return -1;
    return 0;
}

void MobilityAgentAdvertiser::add_handlers()
{
    add_read_handler("MaxAdvertisementInterval", &readMaxAdvertisementInterval, (void*)0);
    add_read_handler("MinAdvertisementInterval", &readMinAdvertisementInterval, (void*)0);
    add_read_handler("AdvertisementLifetime", &readAdvertisementLifetime, (void*)0);
    add_read_handler("RegistrationLifetime", &readRegistrationLifetime, (void*)0);
    add_read_handler("Busy", &readBusy, (void*)0);

    add_write_handler("MaxAdvertisementInterval", &writeMaxAdvertisementInterval, (void*)0);
    add_write_handler("MinAdvertisementInterval", &writeMinAdvertisementInterval, (void*)0);
    add_write_handler("AdvertisementLifetime", &writeAdvertisementLifetime, (void*)0);
    add_write_handler("RegistrationLifetime", &writeRegistrationLifetime, (void*)0);
    add_write_handler("Busy", &writeBusy, (void*)0);
}

void MobilityAgentAdvertiser::push(int, Packet* packet)
{
    click_ether* ethh = (click_ether*)packet->data();
    click_ip* iph = (click_ip*)(ethh + 1);

    sendPacket(iph->ip_src);
    packet->kill();
}

void MobilityAgentAdvertiser::run_timer(Timer *)
{
    sendPacket(IPAddress::make_broadcast());

    // Schedule the next advertisement
    if (_maxAdvertisementInterval > _minAdvertisementInterval)
        _timer.schedule_after_msec((rand() % ((_maxAdvertisementInterval - _minAdvertisementInterval) * 1000)) + _minAdvertisementInterval * 1000);
    else // minimum = maximum
        _timer.schedule_after_msec((_minAdvertisementInterval * 1000) + ((rand() % 100) - 50));
}

void MobilityAgentAdvertiser::sendPacket(IPAddress destinationIP)
{
    int packetsize = sizeof(click_ip) + sizeof(advertisement_header) + sizeof(mobile_advertisement_header);
    int headroom = sizeof(click_ether);
    WritablePacket* packet = Packet::make(headroom, 0, packetsize, 0);
    if (packet == 0)
    {
        click_chatter("cannot make packet!");
        return;
    }

    memset(packet->data(), 0, packet->length());

    click_ip* iph = (click_ip*)packet->data();
    iph->ip_v = 4;
    iph->ip_hl = 5;
    iph->ip_tos = 0;
    iph->ip_len = htons(packetsize);
    iph->ip_id = htons(_sequenceNr);
    iph->ip_ttl = 1; // TTL must be 1 in advertisement
    iph->ip_p = 1; // protocol = ICMP
    iph->ip_src = _srcIp;
    iph->ip_dst = destinationIP;
    iph->ip_sum = click_in_cksum((unsigned char*)packet->data(), packet->length());

    packet->set_dst_ip_anno(iph->ip_dst);

    advertisement_header* advh = (advertisement_header*)(iph + 1);
    advh->type = 9; // Router Advertisement
    advh->code = 0; // Also handles normal routing
    advh->addresses = 1; // Only send one IP address
    advh->addr_size = 2; // Router Address + Preference Level
    advh->lifetime = htons(_advertisementLifetime);
    advh->address = _srcIp; // We only send 1 ip address (our private ip address)
    advh->addrPreference = 0; // Only one ip address, thus preference doesn't really matter

    mobile_advertisement_header* madvh = (mobile_advertisement_header*)(advh + 1);
    madvh->type = 16;
    madvh->length = 6 + 4 * 1; // 6 bytes + 4 bytes for every ip address
    madvh->seq_nr = htons(_sequenceNr);
    madvh->lifetime = htons(_registrationLifetime);
    madvh->address = _careOfAddress;
    madvh->flags =  (0 << 7) // Registration required (1 is to force registration when using co-located care-of address, only set on foreign agent)
                  + (_busy << 6) // Busy
                  + (_homeAgent << 5) // Home agent
                  + (_foreignAgent << 4) // Foreign agent
                  + (0 << 3) // Minimal encapsulation (not supported, rfc 2004)
                  + (0 << 2) // GRE encapsulation (not supported, rfc 2784)
                  + (0 << 1) // reserved
                  + 0;       // Reverse tunneling (not supported, rfc 3024)

    // Calculate the ICMP header checksum
    advh->checksum = click_in_cksum((unsigned char*)advh, packetsize - sizeof(click_ip));

    // Increment the sequence number
    if (_sequenceNr < 0xffff)
        _sequenceNr++;
    else
        _sequenceNr = 256;

    output(0).push(packet);
}

CLICK_ENDDECLS
EXPORT_ELEMENT(MobilityAgentAdvertiser)
