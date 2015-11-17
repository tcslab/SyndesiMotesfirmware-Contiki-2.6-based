#include "ero2manager.h"
#define SERVICE_POINTER(sp) #sp

#include "cc2420.h"
#define POWER 31
#define CHANNEL 22

PROCESS(main_process,"PF Process");
AUTOSTART_PROCESSES(&main_process);

PROCESS_THREAD(main_process, ev, data)
{
	PROCESS_BEGIN();
cc2420_set_txpower(POWER);
cc2420_set_channel(CHANNEL);
	start_ero2_service(SERVICE_POINTER(C1S7A4));
	PROCESS_END();
}


