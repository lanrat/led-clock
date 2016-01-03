#ifndef BANDWIDTH_H
#define BANDWIDTH_H

// DD-WRT
#define SNMP_HOST "192.168.1.1"
#define SNMP_WAN_UP_OID "iso.3.6.1.2.1.2.2.1.10.3"
#define SNMP_WAN_DN_OID "iso.3.6.1.2.1.2.2.1.16.3"
#define OID_LEN 11
#define COMMUNITY "public"

struct bandwidth {
    u_int down;
    u_int up;
};

void bandwidthInit();

bandwidth bandwidthRun();

#endif