#include "public/public.h"
#include "game/user/user.h"
#include "vld/vld.h"

int
main(void)
{
    user_t *user;
    uid_t uid1, uid2;

    user_init();
    user_uid_gen(&uid1, "100");
    user_uid_gen(&uid2, "100");
    user_create(&user, uid1);
    user_insert(user);
    user_create(&user, uid2);
    if (user_insert(user) != 0) {
        user_destroy(&user); 
    }
    user_remove(uid1);
    user_remove(uid2);
    user_fini();
    return 0;
}
