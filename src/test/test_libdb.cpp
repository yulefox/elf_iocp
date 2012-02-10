#include "game/libdb/db.h"
#include "mynet.h"

volatile bool running = true;
extern void LoadInit();
extern void LoadBase();

static BOOL ctrl_handler(DWORD type)
{
	BOOL res = TRUE;

	switch (type) {
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_BREAK_EVENT:
		LOG_INFO("test.net", "Process exits.");
		running = false;
		break;
	default:
		LOG_WARN("test.net", "UNKNOWN_EVENT (%d).", type);
		res = FALSE;
	}
	return res;
}

void TestMsg();
int main(void)
{
	mynet_init();


	//TestMsg();
	

	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrl_handler, TRUE)) {
		LOG_INFO("test.net", "Console control handler is installed, press Ctrl+C to exit.");
		while (running) {
			mynet_proc_msg();
			Sleep(100);
		}
	}
	mynet_fini();
    return 0;
}

void TestMsg()
{
	LoadInit();
	LoadBase();
	//loadDetail();
}
