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

 #ifndef XALLMAIL_H

 #define XALLMAIL_H

 #ifdef __cplusplus
 extern "C" {
 #endif

 #include <libetpan/mailimap_extension.h>

 /* X-ALL-MAIL is a Yahoo Mail extension that enables access to the "All Mail" virtual folder */

 /* Extension ID - this should match the value in mailimap_extension_types.h */
 #define MAILIMAP_EXTENSION_XALLMAIL 14

 enum {
   MAILIMAP_XALLMAIL_TYPE_MAILBOXID,
   MAILIMAP_XALLMAIL_TYPE_RESP_TEXT_CODE
 };

 LIBETPAN_EXPORT
 extern struct mailimap_extension_api mailimap_extension_xallmail;

 /* Checks if the X-ALL-MAIL capability is supported by the server */
 LIBETPAN_EXPORT
 int mailimap_has_xallmail(mailimap * session);

 /* Enables the X-ALL-MAIL capability on the server */
 LIBETPAN_EXPORT
 int mailimap_enable_xallmail(mailimap * session);

 /* Creates a fetch attribute for MAILBOXID */
 LIBETPAN_EXPORT
 struct mailimap_fetch_att * mailimap_fetch_att_new_mailboxid(void);

 /* Gets the MAILBOXID from the SELECT response */
 LIBETPAN_EXPORT
 int mailimap_get_mailboxid(mailimap * session, char ** p_mailboxid);

 #ifdef __cplusplus
 }
 #endif

 #endif
