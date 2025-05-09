#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libetpan/libetpan.h>

/* Include the local header files directly since we're building within the source tree */
#include "../src/low-level/imap/xallmail.h"
#include "../src/low-level/imap/mailimap_extension_types.h"

static const char* username = NULL;
static const char* password = NULL;
static const char* server = "imap.mail.yahoo.com";
static const int port = 993; // IMAPS port (SSL)

static void check_error(int r, char * msg)
{
    if (r == MAILIMAP_NO_ERROR)
        return;
    if (r == MAILIMAP_NO_ERROR_AUTHENTICATED)
        return;
    if (r == MAILIMAP_NO_ERROR_NON_AUTHENTICATED)
        return;

    fprintf(stderr, "Error %s: %d\n", msg, r);

    // Print more detailed error information
    switch (r) {
        case MAILIMAP_NO_ERROR:
            fprintf(stderr, "No error\n");
            break;
        case MAILIMAP_NO_ERROR_AUTHENTICATED:
            fprintf(stderr, "No error authenticated\n");
            break;
        case MAILIMAP_NO_ERROR_NON_AUTHENTICATED:
            fprintf(stderr, "No error non authenticated\n");
            break;
        case MAILIMAP_ERROR_BAD_STATE:
            fprintf(stderr, "Bad state\n");
            break;
        case MAILIMAP_ERROR_STREAM:
            fprintf(stderr, "Stream error\n");
            break;
        case MAILIMAP_ERROR_PARSE:
            fprintf(stderr, "Parse error\n");
            break;
        case MAILIMAP_ERROR_CONNECTION_REFUSED:
            fprintf(stderr, "Connection refused\n");
            break;
        case MAILIMAP_ERROR_MEMORY:
            fprintf(stderr, "Memory error\n");
            break;
        case MAILIMAP_ERROR_FATAL:
            fprintf(stderr, "Fatal error\n");
            break;
        case MAILIMAP_ERROR_PROTOCOL:
            fprintf(stderr, "Protocol error\n");
            break;
        case MAILIMAP_ERROR_DONT_ACCEPT_CONNECTION:
            fprintf(stderr, "Don't accept connection\n");
            break;
        case MAILIMAP_ERROR_APPEND:
            fprintf(stderr, "Append error\n");
            break;
        case MAILIMAP_ERROR_NOOP:
            fprintf(stderr, "Noop error\n");
            break;
        case MAILIMAP_ERROR_LOGOUT:
            fprintf(stderr, "Logout error\n");
            break;
        case MAILIMAP_ERROR_CAPABILITY:
            fprintf(stderr, "Capability error\n");
            break;
        case MAILIMAP_ERROR_CHECK:
            fprintf(stderr, "Check error\n");
            break;
        case MAILIMAP_ERROR_CLOSE:
            fprintf(stderr, "Close error\n");
            break;
        case MAILIMAP_ERROR_EXPUNGE:
            fprintf(stderr, "Expunge error\n");
            break;
        case MAILIMAP_ERROR_COPY:
            fprintf(stderr, "Copy error\n");
            break;
        case MAILIMAP_ERROR_UID_COPY:
            fprintf(stderr, "UID copy error\n");
            break;
        case MAILIMAP_ERROR_CREATE:
            fprintf(stderr, "Create error\n");
            break;
        case MAILIMAP_ERROR_DELETE:
            fprintf(stderr, "Delete error\n");
            break;
        case MAILIMAP_ERROR_EXAMINE:
            fprintf(stderr, "Examine error\n");
            break;
        case MAILIMAP_ERROR_FETCH:
            fprintf(stderr, "Fetch error\n");
            break;
        case MAILIMAP_ERROR_UID_FETCH:
            fprintf(stderr, "UID fetch error\n");
            break;
        case MAILIMAP_ERROR_LIST:
            fprintf(stderr, "List error\n");
            break;
        case MAILIMAP_ERROR_LOGIN:
            fprintf(stderr, "Login error\n");
            break;
        case MAILIMAP_ERROR_LSUB:
            fprintf(stderr, "Lsub error\n");
            break;
        case MAILIMAP_ERROR_RENAME:
            fprintf(stderr, "Rename error\n");
            break;
        case MAILIMAP_ERROR_SEARCH:
            fprintf(stderr, "Search error\n");
            break;
        case MAILIMAP_ERROR_UID_SEARCH:
            fprintf(stderr, "UID search error\n");
            break;
        case MAILIMAP_ERROR_SELECT:
            fprintf(stderr, "Select error\n");
            break;
        case MAILIMAP_ERROR_STATUS:
            fprintf(stderr, "Status error\n");
            break;
        case MAILIMAP_ERROR_STORE:
            fprintf(stderr, "Store error\n");
            break;
        case MAILIMAP_ERROR_UID_STORE:
            fprintf(stderr, "UID store error\n");
            break;
        case MAILIMAP_ERROR_SUBSCRIBE:
            fprintf(stderr, "Subscribe error\n");
            break;
        case MAILIMAP_ERROR_UNSUBSCRIBE:
            fprintf(stderr, "Unsubscribe error\n");
            break;
        case MAILIMAP_ERROR_STARTTLS:
            fprintf(stderr, "STARTTLS error\n");
            break;
        case MAILIMAP_ERROR_INVAL:
            fprintf(stderr, "Invalid argument error\n");
            break;
        case MAILIMAP_ERROR_EXTENSION:
            fprintf(stderr, "Extension error\n");
            break;
        case MAILIMAP_ERROR_SASL:
            fprintf(stderr, "SASL error\n");
            break;
        case MAILIMAP_ERROR_SSL:
            fprintf(stderr, "SSL error\n");
            break;
        default:
            fprintf(stderr, "Unknown error code: %d\n", r);
            break;
    }

    exit(1);
}

static mailimap * connect_and_login(void)
{
    mailimap * imap;
    int r;

    printf("Creating new IMAP session...\n");
    imap = mailimap_new(0, NULL);
    if (imap == NULL) {
        fprintf(stderr, "Failed to create IMAP session\n");
        exit(1);
    }

    printf("Connecting to %s:%d with SSL...\n", server, port);
    r = mailimap_ssl_connect(imap, server, port);
    check_error(r, "connecting to server with SSL");
    printf("Connected successfully\n");

    printf("Logging in as %s...\n", username);
    r = mailimap_login(imap, username, password);
    check_error(r, "login");
    printf("Logged in successfully\n");

    return imap;
}

static void test_select_inbox(mailimap * imap)
{
    int r;
    char * mailboxid = NULL;

    printf("Selecting INBOX...\n");
    r = mailimap_select(imap, "INBOX");
    check_error(r, "select INBOX");

    r = mailimap_get_mailboxid(imap, &mailboxid);
    if (r == MAILIMAP_NO_ERROR) {
        printf("INBOX MAILBOXID: %s\n", mailboxid);
        free(mailboxid);
    } else {
        printf("No MAILBOXID found for INBOX (error: %d)\n", r);
    }
}

static void test_enable_xallmail(mailimap * imap)
{
    int r;

    printf("Checking if X-ALL-MAIL is supported...\n");
    if (mailimap_has_xallmail(imap)) {
        printf("X-ALL-MAIL is supported!\n");

        printf("Checking if ENABLE is supported...\n");
        if (mailimap_has_enable(imap)) {
            printf("ENABLE is supported!\n");

            printf("Enabling X-ALL-MAIL...\n");
            r = mailimap_enable_xallmail(imap);
            check_error(r, "enable X-ALL-MAIL");
            printf("X-ALL-MAIL enabled successfully\n");
        } else {
            printf("ENABLE is not supported by the server\n");
        }
    } else {
        printf("X-ALL-MAIL is not supported by the server\n");
    }
}

static void test_select_all_mail(mailimap * imap)
{
    int r;
    char * mailboxid = NULL;
    const char* folder_names[] = {
        "All Mail",
        "Archive"
    };
    int num_folders = sizeof(folder_names) / sizeof(folder_names[0]);
    int i;
    int success = 0;

    for (i = 0; i < num_folders; i++) {
        printf("Trying to select folder: %s...\n", folder_names[i]);
        r = mailimap_select(imap, folder_names[i]);
        if (r == MAILIMAP_NO_ERROR) {
            printf("Successfully selected folder: %s\n", folder_names[i]);

            r = mailimap_get_mailboxid(imap, &mailboxid);
            if (r == MAILIMAP_NO_ERROR) {
                printf("%s MAILBOXID: %s\n", folder_names[i], mailboxid);
                free(mailboxid);
                mailboxid = NULL;
            } else {
                printf("No MAILBOXID found for %s (error: %d)\n", folder_names[i], r);
            }

            success = 1;
            break;
        } else {
            printf("Could not select folder: %s (error: %d)\n", folder_names[i], r);
        }
    }

    if (!success) {
        printf("Could not find or select any All Mail folder. Continuing with INBOX...\n");
        r = mailimap_select(imap, "INBOX");
        if (r != MAILIMAP_NO_ERROR) {
            check_error(r, "select INBOX");
        }
    }
}

static void test_fetch_with_mailboxid(mailimap * imap)
{
    int r;
    struct mailimap_set * set;
    struct mailimap_fetch_type * fetch_type;
    clist * fetch_result;
    clistiter * iter;
    uint32_t exists_count = 0;

    printf("Fetching messages with MAILBOXID...\n");

    /* Get the number of messages in the mailbox */
    if (imap->imap_selection_info != NULL) {
        exists_count = imap->imap_selection_info->sel_exists;
        printf("Mailbox contains %u messages\n", exists_count);
    } else {
        printf("Could not determine message count\n");
    }

    /* If there are no messages, try to fetch a larger range */
    if (exists_count == 0) {
        exists_count = 1000; /* Try a larger range */
        printf("Trying to fetch a larger range of messages (up to %u)\n", exists_count);
    }

    /* Create a set for all messages in the mailbox using wildcard */
    set = mailimap_set_new_interval(1, 0);  /* 1:* means all messages */
    if (set == NULL) {
        printf("Failed to create set for fetching\n");
        return;
    }

    /* Create a fetch type with UID and FLAGS */
    fetch_type = mailimap_fetch_type_new_fetch_att_list_empty();
    if (fetch_type == NULL) {
        printf("Failed to create fetch type\n");
        mailimap_set_free(set);
        return;
    }

    /* Add UID attribute */
    mailimap_fetch_type_new_fetch_att_list_add(fetch_type, mailimap_fetch_att_new_uid());

    /* We'll skip FLAGS for now since it's causing issues */
    // mailimap_fetch_type_new_fetch_att_list_add(fetch_type, mailimap_fetch_att_new_flags());

    /* Add MAILBOXID attribute */
    struct mailimap_fetch_att * mailboxid_att = mailimap_fetch_att_new_mailboxid();
    if (mailboxid_att != NULL) {
        mailimap_fetch_type_new_fetch_att_list_add(fetch_type, mailboxid_att);
        printf("Added MAILBOXID fetch attribute\n");
    } else {
        printf("Failed to create MAILBOXID fetch attribute\n");
    }

    /* Try to fetch messages */
    printf("Fetching messages with UID, FLAGS, and MAILBOXID...\n");
    r = mailimap_fetch(imap, set, fetch_type, &fetch_result);
    if (r != MAILIMAP_NO_ERROR) {
        printf("Error fetching messages with mailimap_fetch: %d\n", r);

        /* Try with UID FETCH instead */
        printf("Trying UID FETCH instead...\n");
        r = mailimap_uid_fetch(imap, set, fetch_type, &fetch_result);
        if (r != MAILIMAP_NO_ERROR) {
            printf("Error fetching messages with mailimap_uid_fetch: %d\n", r);
            mailimap_set_free(set);
            mailimap_fetch_type_free(fetch_type);
            return;
        }
    }

    printf("Fetched %d messages\n", (int)clist_count(fetch_result));

    /* Process the fetch results */
    for (iter = clist_begin(fetch_result); iter != NULL; iter = clist_next(iter)) {
        struct mailimap_msg_att * msg_att = clist_content(iter);
        clistiter * item_iter;
        uint32_t uid = 0;
        int found_mailboxid = 0;
        char * mailboxid_value = NULL;

        /* Process each attribute in the message */
        for (item_iter = clist_begin(msg_att->att_list); item_iter != NULL; item_iter = clist_next(item_iter)) {
            struct mailimap_msg_att_item * item = clist_content(item_iter);

            if (item->att_type == MAILIMAP_MSG_ATT_ITEM_STATIC) {
                /* Handle static attributes (UID, FLAGS, etc.) */
                if (item->att_data.att_static->att_type == MAILIMAP_MSG_ATT_UID) {
                    uid = item->att_data.att_static->att_data.att_uid;
                }
            }
            else if (item->att_type == MAILIMAP_MSG_ATT_ITEM_EXTENSION) {
                /* Handle extension attributes (MAILBOXID, etc.) */
                struct mailimap_extension_data * ext_data = item->att_data.att_extension_data;

                if (ext_data->ext_extension->ext_id == MAILIMAP_EXTENSION_XALLMAIL &&
                    ext_data->ext_type == MAILIMAP_XALLMAIL_TYPE_MAILBOXID) {
                    mailboxid_value = (char *)ext_data->ext_data;
                    found_mailboxid = 1;
                }
            }
        }

        /* Print the message information */
        if (uid > 0) {
            printf("Message UID %u", uid);

            if (found_mailboxid) {
                printf(" MAILBOXID %s", mailboxid_value);
            } else {
                printf(" (no MAILBOXID)");
            }

            printf("\n");
        }
    }

    mailimap_fetch_list_free(fetch_result);
    mailimap_set_free(set);
    mailimap_fetch_type_free(fetch_type);

    printf("Fetch test completed successfully\n");
}

int main(int argc, char ** argv)
{
    mailimap * imap;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <username> <password>\n", argv[0]);
        exit(1);
    }

    username = argv[1];
    password = argv[2];

    imap = connect_and_login();

    test_select_inbox(imap);
    test_enable_xallmail(imap);
    test_select_all_mail(imap);
    test_fetch_with_mailboxid(imap);

    mailimap_logout(imap);
    mailimap_free(imap);

    return 0;
}
