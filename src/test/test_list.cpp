#include "public/public.h"
#include "public/list.h"
#include "vld/vld.h"

int print(void *val, ...)
{
   LOG_INFO("pub.test", "%s", (char *)val);
   return 0;
}

int main(void)
{
    struct list_t *l;
    char *a = "1", *b = "2", *c = "3";
    char *pa;

    log_init();
    list_create(&l);
    list_push_front(l, a);
    list_push_front(l, b);
    list_push_front(l, c);
    list_push_back(l, a);
    list_push_back(l, b);
    list_push_back(l, c);
    list_traverse(l, print);
    LOG_TRACE("pub.test", "------------");
    list_pop_back(l, (void **)&pa);
    list_pop_back(l, (void **)&pa);
    list_pop_front(l, (void **)&pa);
    list_traverse(l, print);
    LOG_TRACE("pub.test", "------------");
    list_destroy(l);
    log_fini();
    return 0;
}
