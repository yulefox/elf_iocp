#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../libtree.h"

struct point_t {
	int x;
	int y;
	struct rbtree_node  node;
};

int cmp(const struct rbtree_node *n1, const struct rbtree_node *n2)
{
	struct point_t *p1 = (struct point_t*)rbtree_container_of(n1, struct point_t, node);
	struct point_t *p2 = (struct point_t*)rbtree_container_of(n2, struct point_t, node);
	return p1->x - p2->x;
}

int main(int argc, char *argv[])
{
	struct point_t *p;
	struct rbtree rb;
	struct rbtree_node *node;
	int ret;
	int i;
	ret = rbtree_init(&rb, cmp, 0);
	printf("ret = %d\n", ret);

	srand((unsigned int)time(NULL));
	for (i = 0;i < 10; i++) {
		p = (struct point_t*)malloc(sizeof(struct point_t));
		//p->x = rand() % 23 + 1;
		p->x = 10 - i;
		p->y = i + 1;
		printf("(%d, %d)\n", p->x, p->y);

		/*
		p->node.left = NULL;
		p->node.right = NULL;
		p->node.parent = NULL;
		*/

		rbtree_insert(&(p->node), &rb);
	}
	printf("-------------------------------------------------\n");

	/*
	for (node = rbtree_first(&rb);node != NULL; node = rbtree_next(node)) {
		p = (struct point_t*)rbtree_container_of(node, struct point_t, node);
		printf("(%d, %d)\n", p->x, p->y);
	}
	*/

	while((node = rbtree_first(&rb)) != NULL) {
		p = (struct point_t*)rbtree_container_of(node, struct point_t, node);
		printf("(%d, %d)\n", p->x, p->y);
		rbtree_remove(node, &rb);
		free(p);
	}

	system("pause");
	return 0;
}
