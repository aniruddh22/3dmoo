/*
 * Copyright (C) 2014 - plutoo
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../util.h"
#include "../arm11/arm11.h"
#include "../mem.h"
#include "../handles.h"

// services/srv.c
u32 srv_InitHandle();
u32 srv_SyncRequest();
u32 services_SyncRequest();

static struct {
    const char* name;
    u32 subtype;
    u32 (*fnInitHandle)();
    u32 (*fnSyncRequest)();
} ports[] = {
    // Ports are declared here.
    {
	"srv:",
	PORT_TYPE_SRV,
	&srv_InitHandle,
	&srv_SyncRequest
    }
};

u32 svcConnectToPort() {
    //u32 handle_out   = arm11_R(0);
    u32 portname_ptr = arm11_R(1);;
    char name[12];

    u32 i;
    for(i=0; i<12; i++) {
	name[i] = mem_Read8(portname_ptr+i);
	if(name[i] == '\0')
	    break;
    }

    if(i == 12 && name[7] != '\0') {
	ERROR("requesting service with missing null-byte\n");
	arm11_Dump();
	PAUSE();
	return 0xE0E0181E;
    }

    for(i=0; i<ARRAY_SIZE(ports); i++) {
	if(strcmp(name, ports[i].name) == 0) {
	    return ports[i].fnInitHandle();
	}
    }

    DEBUG("Port %s: NOT IMPLEMENTED!\n", name);
    PAUSE();
    return 0;
}

u32 svcSendSyncRequest() {
    u32 handle = arm11_R(0);
    handleinfo* hi = handle_Get(handle);

    if(hi == NULL) {
	ERROR("handle %08x not found..\n", handle);
	PAUSE();
	exit(1);
    }

    if(hi->type == HANDLE_TYPE_SERVICE) {
        return services_SyncRequest();
    }
    if(hi->type != HANDLE_TYPE_PORT) {
	ERROR("handle %08x is not a port-handle..\n", handle);
	PAUSE();
	exit(1);
    }

    u32 i;
    for(i=0; i<ARRAY_SIZE(ports); i++) {
	if(hi->subtype == ports[i].subtype)
	    return ports[i].fnSyncRequest();
    }

    ERROR("no port found for sync..\n");
    arm11_Dump();
    PAUSE();
    exit(1);
    return 0;
}
