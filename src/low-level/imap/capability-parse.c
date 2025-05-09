#include "capability-parse.h"
#include <string.h>
#include <stdlib.h>

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
int mailimap_has_capability(mailimap * session, const char * cap_name, int match_partial)
{
  clist * cap_list;
  clistiter * iter;
  struct mailimap_capability * cap;
  size_t cap_name_len;
  
  if (session == NULL || cap_name == NULL)
    return 0;
  
  if (session->imap_connection_info == NULL)
    return 0;
  
  if (session->imap_connection_info->imap_capability == NULL)
    return 0;
  
  cap_list = session->imap_connection_info->imap_capability->cap_list;
  if (cap_list == NULL)
    return 0;
  
  cap_name_len = strlen(cap_name);
  
  for (iter = clist_begin(cap_list); iter != NULL; iter = clist_next(iter)) {
    cap = clist_content(iter);
    
    if (cap->cap_type == MAILIMAP_CAPABILITY_NAME) {
      if (match_partial) {
        /* Partial match - check if cap_name is a prefix of the capability */
        if (strncasecmp(cap->cap_data.cap_name, cap_name, cap_name_len) == 0) {
          return 1;
        }
      } else {
        /* Exact match */
        if (strcasecmp(cap->cap_data.cap_name, cap_name) == 0) {
          return 1;
        }
      }
    }
  }
  
  return 0;
}

/*
 * mailimap_get_capability_value retrieves the value part of a capability with format "NAME=VALUE"
 *
 * @param session IMAP session
 * @param cap_name name of the capability to look for (e.g., "MESSAGELIMIT")
 *
 * @return the value part if found (e.g., "1000" for "MESSAGELIMIT=1000"), NULL otherwise
 * The returned string must be freed by the caller.
 */
char * mailimap_get_capability_value(mailimap * session, const char * cap_name)
{
  clist * cap_list;
  clistiter * iter;
  struct mailimap_capability * cap;
  size_t cap_name_len;
  char * equals_pos;
  char * result = NULL;
  
  if (session == NULL || cap_name == NULL)
    return NULL;
  
  if (session->imap_connection_info == NULL)
    return NULL;
  
  if (session->imap_connection_info->imap_capability == NULL)
    return NULL;
  
  cap_list = session->imap_connection_info->imap_capability->cap_list;
  if (cap_list == NULL)
    return NULL;
  
  cap_name_len = strlen(cap_name);
  
  for (iter = clist_begin(cap_list); iter != NULL; iter = clist_next(iter)) {
    cap = clist_content(iter);
    
    if (cap->cap_type == MAILIMAP_CAPABILITY_NAME) {
      /* Check if the capability name starts with cap_name followed by = */
      if (strncasecmp(cap->cap_data.cap_name, cap_name, cap_name_len) == 0) {
        equals_pos = strchr(cap->cap_data.cap_name, '=');
        if (equals_pos != NULL) {
          /* Extract the value after the = sign */
          result = strdup(equals_pos + 1);
          break;
        }
      }
    }
  }
  
  return result;
}
