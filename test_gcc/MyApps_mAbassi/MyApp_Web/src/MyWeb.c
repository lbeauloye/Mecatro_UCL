/* ------------------------------------------------------------------------------------------------ */

#include "mAbassi.h"
#include "SysCall.h"								/* System call layer definitions				*/
#include "Platform.h"								/* Everything about the target platform is here	*/
#include "HWinfo.h"									/* Everything about the target board is here	*/

#include "ethernetif.h"
#include "lwip/api.h"
#include "lwip/arch.h"
#include "lwip/def.h"
#include "lwip/opt.h"
#include "lwip/dhcp.h"
#include "tcp.h"
#include "tcpip.h"
#include "netif/etharp.h"
#include "lwipopts.h"

#define DHCP_TASK_PRIO   	(OX_PRIO_MIN-2)			/* Priority of the DHCP task					*/
#define LED_TASK_PRIO    	(OX_PRIO_MIN-1)			/* Priority of the LED flashing task			*/

#define MTXLOCK_ALLOC()		MTXlock(G_OSmutex, -1)
#define MTXUNLOCK_ALLOC()	MTXunlock(G_OSmutex)
#define MTXLOCK_STDIO()		MTXlock(G_OSmutex, -1)
#define MTXUNLOCK_STDIO()	MTXunlock(G_OSmutex)

void LwIP_DHCP_task(void);
void NetAddr_Init(void);
void LwIP_Init(void);
void http_server_init(void);

/* ------------------------------------------------------------------------------------------------ */
/* EMAC & lwIP initialization																		*/

void DoInitLwIP(void)
{
    int     CurrentTick;
    
    MY_BUTTON_INIT();
    MY_LED_INIT();
    
    NetAddr_Init();                                 /* Default static Addresses & Mask              */
    
    MTXLOCK_STDIO();
    puts("Initializing Ethernet I/F");
    MTXUNLOCK_STDIO();
    
    CurrentTick = G_OStimCnt;
    
    LwIP_Init();
    
    if ((G_OStimCnt-CurrentTick) > (30*OS_TICK_PER_SEC)) {
        MTXLOCK_STDIO();
        printf("The Ethernet I/F set-up seems to have taken a long time\n");
        printf("Is there a cable on the \"HPS Ethernet\" RJ-45 connector?\n\n");
        MTXUNLOCK_STDIO();
    }
    
    http_server_init();
    
    MTXLOCK_STDIO();
    puts("\nWaiting for Ethernet I/F & Webserver & file system to be ready\n");
    MTXUNLOCK_STDIO();
    TSKcreate("DHCP Client", DHCP_TASK_PRIO, 16384, LwIP_DHCP_task, 1);
    
	EVTwait(3, 0, -1);								/* Wait for Ethernet & file system to be ready	*/

	MTXLOCK_STDIO();
	puts("The Webserver is ready");
	MTXUNLOCK_STDIO();

  #if ((TEST_RXPERF) != 0)
    extern void IperfRXinit(void);
	IperfRXinit();
  #elif ((TEST_TXPERF) != 0)
    extern void IperfTXinit(void);
	IperfTXinit();
  #endif
}

/* ------------------------------------------------------------------------------------------------ */


/* EOF */
