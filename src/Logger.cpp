#include "Logger.h"

void log(LogLevel logLevel, const char* message, ...)
{
	const char* prefix[6] = { "[FATAL]:\t", "[ERROR]:\t", "[WARN]:\t\t", "[INFO]:\t\t", "[DEBUG]:\t", "[TRACE]:\t" };

	char unformatted_message[16 * 1024] = { 0 };

	va_list arg_ptr;
	va_start(arg_ptr, message);
	vsnprintf(unformatted_message, 16 * 1024, message, arg_ptr);
	va_end(arg_ptr);
	char out_message[16 * 1024] = { 0 };
	sprintf(out_message, "%s%s\n", prefix[logLevel], unformatted_message);

	printf("%s", out_message);
}