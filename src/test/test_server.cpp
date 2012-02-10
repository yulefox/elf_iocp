#include "public/net.h"
#include "public/public.h"
#include "public/thread/thread.h"
#include "vld/vld.h"

#pragma comment (lib, "Winmm.lib")

volatile bool running = true;

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

static int
mynet_proc_event(SOCKET s, int type, int res, void *args)
{
    switch (type) {
    case NIT_ACPT: /* 主动断开已连接 socket */
        net_onnotify(s, type, res, args);
        break;
    case NIT_SNDF: /* 统计消息发送量 */
        break;
    default:
        net_onnotify(s, type, res, args);
    }
    return 0;
}

int main(void)
{
    net_register(mynet_proc_event);
    public_init();
    net_init();
    LOG_INFO("test.net", "-== SERVER START ==-");
    net_entity_start(256);
    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrl_handler, TRUE)) {
        LOG_INFO("test.net", "Console control handler is installed.");
        while (running) {
            Sleep(100);
        }
    }
    net_fini();
    public_fini();
    return 0;
}
