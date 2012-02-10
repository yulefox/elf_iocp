#include "log.h"
#include <log4cplus/configurator.h>

static const char *filename = "CONFIG/log.ini";

int log_init(void)
{
    MODULE_IMPORT_SWITCH;
    ::CreateDirectory("log", NULL);
    log4cplus::PropertyConfigurator::doConfigure(filename);
    return 0;
}

int log_fini(void)
{
    MODULE_IMPORT_SWITCH;
	log4cplus::Logger::shutdown();
    return 0;
}

const char *log_fmt(char *buf, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf_s(buf, MAX_LOG_LENGTH, fmt, args);
	va_end(args);
    return buf;
}
