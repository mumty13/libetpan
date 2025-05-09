#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libetpan/libetpan.h>
#include <libetpan/capability-parse.h>

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

static void test_capabilities(mailimap * imap)
{
    int r;
    struct mailimap_capability_data * cap_data;
    clistiter * iter;
    struct mailimap_capability * cap;
    
    printf("Fetching capabilities...\n");
    r = mailimap_capability(imap, &cap_data);
    check_error(r, "capability");
    
    printf("Server capabilities:\n");
    for (iter = clist_begin(cap_data->cap_list); iter != NULL; iter = clist_next(iter)) {
        cap = clist_content(iter);
        
        if (cap->cap_type == MAILIMAP_CAPABILITY_NAME) {
            printf("- %s\n", cap->cap_data.cap_name);
        }
        else if (cap->cap_type == MAILIMAP_CAPABILITY_AUTH_TYPE) {
            printf("- AUTH=%s\n", cap->cap_data.cap_auth_type);
        }
    }
    
    /* Check for MESSAGELIMIT capability */
    if (mailimap_has_capability(imap, "MESSAGELIMIT", 1)) {
        char * value = mailimap_get_capability_value(imap, "MESSAGELIMIT");
        if (value != NULL) {
            printf("\nFound MESSAGELIMIT capability with value: %s\n", value);
            free(value);
        } else {
            printf("\nFound MESSAGELIMIT capability but couldn't extract value\n");
        }
    } else {
        printf("\nMESSAGELIMIT capability not found\n");
    }
    
    /* Check for APPENDLIMIT capability */
    if (mailimap_has_capability(imap, "APPENDLIMIT", 1)) {
        char * value = mailimap_get_capability_value(imap, "APPENDLIMIT");
        if (value != NULL) {
            printf("Found APPENDLIMIT capability with value: %s\n", value);
            free(value);
        } else {
            printf("Found APPENDLIMIT capability but couldn't extract value\n");
        }
    } else {
        printf("APPENDLIMIT capability not found\n");
    }
    
    mailimap_capability_data_free(cap_data);
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
    test_capabilities(imap);

    mailimap_logout(imap);
    mailimap_free(imap);

    return 0;
}
