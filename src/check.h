#ifndef CHECK_H
#define CHECK_H

#include "addr2ethers.h"

int check_arp(struct pkt *p);
int check_ns(struct pkt *p);
int check_na(struct pkt *p);

#endif
