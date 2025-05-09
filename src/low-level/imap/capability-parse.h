#ifndef CAPABILITY_PARSE_H
#define CAPABILITY_PARSE_H

#include "mailimap.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * mailimap_has_capability checks if a capability exists in the IMAP session
 *
 * @param session IMAP session
 * @param cap_name name of the capability to look for
 * @param match_partial if true, matches cap_name as a prefix (e.g., "MESSAGELIMIT" matches "MESSAGELIMIT=1000")
 *                     if false, requires exact match
 *
 * @return 1 if capability exists, 0 otherwise
 */
LIBETPAN_EXPORT
int mailimap_has_capability(mailimap * session, const char * cap_name, int match_partial);

/*
 * mailimap_get_capability_value retrieves the value part of a capability with format "NAME=VALUE"
 *
 * @param session IMAP session
 * @param cap_name name of the capability to look for (e.g., "MESSAGELIMIT")
 *
 * @return the value part if found (e.g., "1000" for "MESSAGELIMIT=1000"), NULL otherwise
 * The returned string must be freed by the caller.
 */
LIBETPAN_EXPORT
char * mailimap_get_capability_value(mailimap * session, const char * cap_name);

#ifdef __cplusplus
}
#endif

#endif /* CAPABILITY_PARSE_H */
