/*
 * libEtPan! -- a mail stuff library
 *
 * Copyright (C) 2001, 2014 - DINH Viet Hoa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the libEtPan! project nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

 #include "xallmail.h"

 #include <stdlib.h>
 #include <string.h>
 #include <stdio.h>

 #include "mailimap_parser.h"
 #include "mailimap_sender.h"
 #include "mailimap.h"
 #include "mailimap_keywords.h"
 #include "mailimap_extension.h"
 #include "enable.h"

 static int fetch_data_mailboxid_parse(mailstream * fd,
                                      MMAPString * buffer, struct mailimap_parser_context * parser_ctx, size_t * indx,
                                      uint32_t * result, size_t progr_rate, progress_function * progr_fun);

 static int
 mailimap_xallmail_extension_parse(int calling_parser, mailstream * fd,
                                  MMAPString * buffer, struct mailimap_parser_context * parser_ctx, size_t * indx,
                                  struct mailimap_extension_data ** result,
                                  size_t progr_rate, progress_function * progr_fun);

 static void
 mailimap_xallmail_extension_data_free(struct mailimap_extension_data * ext_data);

 LIBETPAN_EXPORT
 struct mailimap_extension_api mailimap_extension_xallmail = {
   /* name */          "X-ALL-MAIL",
   /* extension_id */  MAILIMAP_EXTENSION_XALLMAIL,
   /* parser */        mailimap_xallmail_extension_parse,
   /* free */          mailimap_xallmail_extension_data_free
 };

 static int fetch_data_mailboxid_parse(mailstream * fd,
                                      MMAPString * buffer, struct mailimap_parser_context * parser_ctx, size_t * indx,
                                      uint32_t * result, size_t progr_rate, progress_function * progr_fun)
 {
     size_t cur_token;
     uint32_t mailboxid;
     int r;

     cur_token = * indx;

     r = mailimap_token_case_insensitive_parse(fd, buffer,
                                               &cur_token, "MAILBOXID");
     if (r != MAILIMAP_NO_ERROR)
         return r;

     r = mailimap_space_parse(fd, buffer, &cur_token);
     if (r != MAILIMAP_NO_ERROR)
         return r;

     r = mailimap_oparenth_parse(fd, buffer, parser_ctx, &cur_token);
     if (r != MAILIMAP_NO_ERROR)
         return r;

     /* Try to parse as a number first */
     r = mailimap_number_parse(fd, buffer, &cur_token, &mailboxid);
     if (r != MAILIMAP_NO_ERROR) {
         /* If it's not a number, just use a default value for now */
         /* The actual string value will be handled in mailimap_xallmail_extension_parse */
         mailboxid = 0;
     }

     r = mailimap_cparenth_parse(fd, buffer, parser_ctx, &cur_token);
     if (r != MAILIMAP_NO_ERROR)
         return r;

     * indx = cur_token;
     * result = mailboxid;

     return MAILIMAP_NO_ERROR;
 }

 static int
 mailimap_xallmail_extension_parse(int calling_parser, mailstream * fd,
                                  MMAPString * buffer, struct mailimap_parser_context * parser_ctx, size_t * indx,
                                  struct mailimap_extension_data ** result,
                                  size_t progr_rate, progress_function * progr_fun)
 {
     struct mailimap_extension_data * ext_data;
     size_t cur_token;
     int r;
     int type;
     char * mailboxid_str = NULL;

     cur_token = * indx;

     switch (calling_parser)
     {
         case MAILIMAP_EXTENDED_PARSER_FETCH_DATA:
         {
             /* For now, we'll still use the numeric parser for fetch data */
             uint32_t mailboxid;
             r = fetch_data_mailboxid_parse(fd, buffer, parser_ctx, &cur_token, &mailboxid, progr_rate, progr_fun);
             if (r != MAILIMAP_NO_ERROR)
                 return r;

             /* Convert the numeric ID to a string */
             mailboxid_str = malloc(32); /* Enough for any uint32_t */
             if (mailboxid_str == NULL)
                 return MAILIMAP_ERROR_MEMORY;

             snprintf(mailboxid_str, 32, "%u", mailboxid);
             type = MAILIMAP_XALLMAIL_TYPE_MAILBOXID;
             break;
         }

         case MAILIMAP_EXTENDED_PARSER_RESP_TEXT_CODE:
             /* Parse [MAILBOXID (value)] response where value can be alphanumeric */
             r = mailimap_token_case_insensitive_parse(fd, buffer, &cur_token, "MAILBOXID");
             if (r != MAILIMAP_NO_ERROR)
                 return r;

             r = mailimap_space_parse(fd, buffer, &cur_token);
             if (r != MAILIMAP_NO_ERROR)
                 return r;

             r = mailimap_oparenth_parse(fd, buffer, parser_ctx, &cur_token);
             if (r != MAILIMAP_NO_ERROR)
                 return r;

             /* Parse the MAILBOXID value which can be alphanumeric */
             size_t begin_id = cur_token;
             size_t end_id = begin_id;
             int found_closing = 0;

             /* Find the closing parenthesis */
             while (end_id < buffer->len) {
                 if (buffer->str[end_id] == ')') {
                     found_closing = 1;
                     break;
                 }
                 end_id++;
             }

             if (!found_closing)
                 return MAILIMAP_ERROR_PARSE;

             /* Extract the MAILBOXID value */
             size_t id_len = end_id - begin_id;
             if (id_len == 0)
                 return MAILIMAP_ERROR_PARSE;

             mailboxid_str = malloc(id_len + 1);
             if (mailboxid_str == NULL)
                 return MAILIMAP_ERROR_MEMORY;

             memcpy(mailboxid_str, buffer->str + begin_id, id_len);
             mailboxid_str[id_len] = '\0';

             /* Update the current token position */
             cur_token = end_id;

             r = mailimap_cparenth_parse(fd, buffer, parser_ctx, &cur_token);
             if (r != MAILIMAP_NO_ERROR) {
                 free(mailboxid_str);
                 return r;
             }

             type = MAILIMAP_XALLMAIL_TYPE_RESP_TEXT_CODE;
             break;

         default:
             return MAILIMAP_ERROR_PARSE;
     }

     /* Create the extension data with the string MAILBOXID */
     ext_data = mailimap_extension_data_new(&mailimap_extension_xallmail,
                                           type, mailboxid_str);
     if (ext_data == NULL) {
         free(mailboxid_str);
         return MAILIMAP_ERROR_MEMORY;
     }

     * result = ext_data;
     * indx = cur_token;

     return MAILIMAP_NO_ERROR;
 }

 static void
 mailimap_xallmail_extension_data_free(struct mailimap_extension_data * ext_data)
 {
     free(ext_data->ext_data);
     free(ext_data);
 }

 LIBETPAN_EXPORT
 int mailimap_has_xallmail(mailimap * session)
 {
     return mailimap_has_extension(session, "X-ALL-MAIL");
 }

 LIBETPAN_EXPORT
 int mailimap_enable_xallmail(mailimap * session)
 {
     struct mailimap_capability_data * capability = NULL;
     struct mailimap_capability_data * result = NULL;
     clist * cap_list = NULL;
     struct mailimap_capability * cap = NULL;
     char * cap_name = NULL;
     int r;
     int original_state;

     if (!mailimap_has_enable(session)) {
         return MAILIMAP_ERROR_EXTENSION;
     }

     /* The ENABLE command can only be used in the AUTHENTICATED state,
        so we need to check the current state and temporarily change it if needed */
     original_state = session->imap_state;

     if (original_state != MAILIMAP_STATE_AUTHENTICATED &&
         original_state != MAILIMAP_STATE_SELECTED) {
         /* If we're not in AUTHENTICATED or SELECTED state, we can't proceed */
         return MAILIMAP_ERROR_BAD_STATE;
     }

     /* Temporarily change the state to AUTHENTICATED if needed */
     if (original_state == MAILIMAP_STATE_SELECTED) {
         session->imap_state = MAILIMAP_STATE_AUTHENTICATED;
     }

     /* Following the pattern from mailcore2 */
     cap_list = clist_new();
     if (cap_list == NULL) {
         if (original_state == MAILIMAP_STATE_SELECTED) {
             session->imap_state = original_state;
         }
         return MAILIMAP_ERROR_MEMORY;
     }

     cap_name = strdup("X-ALL-MAIL");
     if (cap_name == NULL) {
         clist_free(cap_list);
         if (original_state == MAILIMAP_STATE_SELECTED) {
             session->imap_state = original_state;
         }
         return MAILIMAP_ERROR_MEMORY;
     }

     cap = mailimap_capability_new(MAILIMAP_CAPABILITY_NAME, NULL, cap_name);
     if (cap == NULL) {
         free(cap_name);
         clist_free(cap_list);
         if (original_state == MAILIMAP_STATE_SELECTED) {
             session->imap_state = original_state;
         }
         return MAILIMAP_ERROR_MEMORY;
     }

     r = clist_append(cap_list, cap);
     if (r < 0) {
         mailimap_capability_free(cap);
         clist_free(cap_list);
         if (original_state == MAILIMAP_STATE_SELECTED) {
             session->imap_state = original_state;
         }
         return MAILIMAP_ERROR_MEMORY;
     }

     capability = mailimap_capability_data_new(cap_list);
     if (capability == NULL) {
         clist_foreach(cap_list, (clist_func) mailimap_capability_free, NULL);
         clist_free(cap_list);
         if (original_state == MAILIMAP_STATE_SELECTED) {
             session->imap_state = original_state;
         }
         return MAILIMAP_ERROR_MEMORY;
     }

     r = mailimap_enable(session, capability, &result);

     /* Always free capability since we're done with it */
     mailimap_capability_data_free(capability);

     /* Restore the original state if needed */
     if (original_state == MAILIMAP_STATE_SELECTED) {
         session->imap_state = original_state;
     }

     if (r != MAILIMAP_NO_ERROR) {
         if (result != NULL) {
             mailimap_capability_data_free(result);
         }
         return r;
     }

     if (result != NULL) {
         mailimap_capability_data_free(result);
     }

     return MAILIMAP_NO_ERROR;
 }

 LIBETPAN_EXPORT
 struct mailimap_fetch_att * mailimap_fetch_att_new_mailboxid(void)
 {
     char * keyword;
     struct mailimap_fetch_att * att;

     keyword = strdup("MAILBOXID");
     if (keyword == NULL)
         return NULL;

     att = mailimap_fetch_att_new_extension(keyword);
     if (att == NULL) {
         free(keyword);
         return NULL;
     }

     return att;
 }

 LIBETPAN_EXPORT
 int mailimap_get_mailboxid(mailimap * session, char ** p_mailboxid)
 {
     clistiter * cur;

     if (session->imap_selection_info == NULL)
         return MAILIMAP_ERROR_BAD_STATE;

     if (session->imap_response_info == NULL)
         return MAILIMAP_ERROR_BAD_STATE;

     for(cur = clist_begin(session->imap_response_info->rsp_extension_list) ; cur != NULL ; cur = clist_next(cur)) {
         struct mailimap_extension_data * ext_data;

         ext_data = clist_content(cur);
         if (ext_data->ext_extension->ext_id != MAILIMAP_EXTENSION_XALLMAIL) {
             continue;
         }

         if (ext_data->ext_type != MAILIMAP_XALLMAIL_TYPE_RESP_TEXT_CODE) {
             continue;
         }

         /* The MAILBOXID is stored as a string */
         *p_mailboxid = strdup((char *) ext_data->ext_data);
         if (*p_mailboxid == NULL) {
             return MAILIMAP_ERROR_MEMORY;
         }
         return MAILIMAP_NO_ERROR;
     }

     return MAILIMAP_ERROR_EXTENSION;
 }
