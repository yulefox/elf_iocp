#include "public/net.hpp"
#include "public/public.h"
#include "public/thread/thread.h"
#include "vld/vld.h"

#pragma comment (lib, "Winmm.lib")

volatile bool running = true;

static unsigned int __stdcall write(void *args)
{
    int a = 0;
    DWORD st, et;

    LOG_INFO("test.net.write", "Write thread is running...");
    st = timeGetTime();
    while (!thread_isexit(args) && a < 1) {
        struct message_t *msg;
        CMessage msgex(0);

        msg = net_message_create(1);
        net_message_append_int(msg, 50);
        net_message_append_float(msg, 50);
        net_message_append_str(msg, "a = net_message_retrive_int(msg); \n"
            "f = net_message_retrive_float(msg); \n"
            "net_message_retrive_str(msg, str);");
        net_message_destroy(msg);
        msgex.Add(50);
        msgex.Add(50.0f);
        msgex.Add("50.0f");
        int n = msgex.GetInt();
        float f = msgex.GetFloat();
        char str[20];
        msgex.GetStr(str, 20);
        ++a;
   }
    et = timeGetTime();
    if (et == st) {
        ++et;
    }
    LOG_INFO("test.net.write", "Write thread pushes %d messages (%d/s).", a,
        a * 1000 / (et - st));
    return 0;
}

static unsigned int __stdcall read(void *args)
{
    return 0;
}

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

int main(void)
{
    struct thread_t *ts, *tv;

    net_init();
    LOG_INFO("test.net", "Net module initilized.");
    thread_create(&tv, read, NULL);
    thread_create(&ts, write, NULL);
    if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)ctrl_handler, TRUE)) {
        LOG_INFO("test.net", "Console control handler is installed.");
        while (running) {
            Sleep(100);
        }
    }
    thread_destroy(ts);
    thread_destroy(tv);
    net_fini();
    return 0;
}
