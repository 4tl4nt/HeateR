#ifndef COMMUNICATIONS_H
#define COMMUNICATIONS_H
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <HeateR.h>

#define SIZE_LOOP_BUF 30

void CommAPI(EthernetClient client);

#endif