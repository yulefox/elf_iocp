#include <windows.h>
#include <stdlib.h>

#include "public/public.h"
#include "vld/vld.h"

#include "game/container/group/container.h"

typedef struct command_t {
    char *str;
    int argc;
    void (*func)(container_t *, const uid_t &);
} command_t;

void test_add(container_t *c, const uid_t &id)
{
    int ret;
    printf("begin to add uid(%d) to container(uid=%d).\n",
           id, container_get_uid(c));

    ret = container_add(c, id);

    printf("Add uid(%d) to container(uid=%d) %s...\n",
           id, container_get_uid(c), ret == 0 ? "OK" : "Failed");
}

void test_remove(container_t *c, const uid_t &id)
{
    int ret;
    printf("begin to remove uid(%d) from container(uid=%d).\n",
           id, container_get_uid(c));
    ret = container_remove(c, id);
    printf("Remove uid(%d) from container(uid=%d) %s...\n",
           id, container_get_uid(c), ret == 1 ? "OK" : "Failed");
}

void test_find(container_t *c, const uid_t &id)
{
    int ret;
    printf("begin to find uid(%d) in container(uid=%d).\n",
           id, container_get_uid(c));
    ret = container_find((const container_t *)c, id);
    printf("Find uid(%d) in container(uid=%d) %s...\n",
           id, container_get_uid(c), ret == 1 ? "OK" : "Failed");
}

void test_getall(container_t *c, const uid_t &id)
{
    std::vector<uid_t> uids;
    size_t i;
    int ret;

    ret = container_getall(c, uids);
    if (ret < 0) {
        printf("Get all uid from container(uid=%d) failed...\n",
               container_get_uid(c));
        return;
    }
    printf("Get all uid from container(uid=%d) ok...\n",
           container_get_uid(c));
    for (i = 0; i < uids.size(); i++) {
        printf("uid=%d\n", uids[i]);
    }
}

#define MAXN_CMD 4
command_t cmdlist[MAXN_CMD] = { {"add", 1, test_add},
    {"remove", 1, test_remove},
    {"find", 1, test_find},
    {"getall", 0, test_getall}
};

int str_split(char *str, char token, char *argv[])
{
    int argc = 0;
    if (str == NULL || str[0] == '\0') {
        return 0;
    }

    char *p = str;
    char *q = NULL;

    while ((q = strchr(p, token))) {
        *q = '\0';
        argv[argc] = (char *)pool_alloc(strlen(p) + 1);
        memset(argv[argc], 0, strlen(p) + 1);
        strcpy(argv[argc], p);
        ++argc;
        p = q + 1;
    }
    if (p[0] != '\0') {
        argv[argc] = (char *)pool_alloc(strlen(p) + 1);
        memset(argv[argc], 0, strlen(p) + 1);
        strcpy(argv[argc], p);
        ++argc;
    }
    return argc;
}

int main(void)
{
    public_init();
    container_t *c;
    uid_t uid;
    char str[1024];
    char *argv[1024];
    int quit;
    int argc;
    int i;

    srand((unsigned int)time(NULL));
    c = container_create(NULL);
    if (c == NULL) {
        LOG_INFO("test.group", "Create Group Container Failed...");
        goto end;
    }

    uid = uid_t(rand() % 0xffff);
    container_set_uid(c, uid);
    LOG_INFO("test.group", "Create a group container(uid=%d)", uid);

    quit = 0;
    while (quit == 0) {
        printf(">");
        gets(str);
        argc = str_split(str, ' ', argv);
        if (argc == 0) {
            continue;
        }

        if (!strcmp(argv[0], "quit") ||
                !strcmp(argv[0], "exit")) {
            printf("Bye...\n");
            quit = 1;
        } else if (!strcmp(argv[0], "help")) {
            for (i = 0; i < MAXN_CMD; i++) {
                printf("%s uid\n", cmdlist[i].str);
            }
        } else {
            int flag =  0;
            for (i = 0; i < MAXN_CMD; i++) {
                if (!strcmp(argv[0], cmdlist[i].str)) {
                    flag = 1;
                    if (argc - 1 == cmdlist[i].argc) {
                        switch (cmdlist[i].argc) {
                        case 0:
                            cmdlist[i].func(c, uid_t(0));
                            break;
                        case 1:
                            cmdlist[i].func(c, uid_t(atoi(argv[1])));
                            break;
                        default:
                            break;
                        }
                    } else {
                        printf("Invalid format, type \'help\' to get the usage.\n");
                    }
                }
            }
            if (flag == 0) {
                printf("Wrong command, type \'help\' to get the command list.\n");
            }
        }
        for (i = 0; i < argc; i++) {
            pool_free(argv[i]);
        }
    }

end:
    container_destroy(c);
    public_fini();
    return 0;
}
