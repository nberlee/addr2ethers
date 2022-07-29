#ifndef PROCESS_H
#define PROCESS_H

#include "addr2ethers.h"

void process_arp(struct pkt *p);
void process_ns(struct pkt *p);
void process_na(struct pkt *p);

#endif
