#include <stdio.h>
#include "rte_ring.h"
// #include <inttypes.h>
int main(int argc, char *argv[])
{
	printf("%s\n", argv[0]);
	
	struct rte_ring * r = rte_ring_create( 1024,  0);
	printf("sizeof(struct rte_ring)=%lu\n", sizeof(struct rte_ring));
	printf("sizeof(r->prod)=%lu\n", sizeof(r->prod));
	printf("sizeof(r->cons)=%lu\n", sizeof(r->cons));
	printf(" %p:->:%p\n",r ,r+1);
	printf(" %lx:->:%lx\n",(unsigned long)r ,(unsigned long)(r+1));
	printf(" %p\n", &(r->prod));
	printf(" %p\n *********************\n", &(r->cons)); 
	int **obj_table = (int **)malloc(sizeof(int*) * 8);
	for(int i = 0; i < 8; ++i)
	{
		obj_table[i] = (int *)malloc(sizeof(int));
		obj_table[i][0] = i+1;
	}
	rte_ring_mp_enqueue_burst(r, (void *const *)(obj_table), 8, 0);
	int **obj_table1 = (int **)malloc(sizeof(int*) * 8);
	for(int i = 0; i < 8; ++i)
	{
		obj_table1[i] = (int *)malloc(sizeof(int));
	}
	rte_ring_dump(r);

	rte_ring_mc_dequeue_burst(r, (void **)(obj_table1), 8, 0);
	for(int i = 0; i < 8; ++i)
	{
		printf("%d\n", obj_table1[i][0]);
	}

	char * bufs[3] = { "123", "156", "189" };
	rte_ring_mp_enqueue_burst(r, (void *const *)(bufs), 3, 0);
	char * bufs2[3] = {0};
	rte_ring_mc_dequeue_burst(r, (void **)(bufs2), 3, 0);
	for(int i = 0; i < 3; ++i)
	{
		printf("%s\n", bufs2[i]);
	}
	rte_ring_dump(r);
	rte_ring_destory(r);
	
	return 0;
}
