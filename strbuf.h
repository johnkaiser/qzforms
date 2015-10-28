


struct strbuf {
    struct strbuf * next;
    struct strbuf * prev;
    struct strbuf * tmp;
    int length;
    int bufsize;
    int n;
    char* endnull;
    char str[];
};

/*
 * new_strbuf
 *
 *  Create a new strbuf that is the larger of that necessary
 *  to hold newdata and minsize.
 *  newdata may be null, in which case minsize is required.
 *  minsize may be zero, in which case newdata is required.
 */

extern struct strbuf* new_strbuf(char* newdata, int minsize);

/* 
 * strbuf_from_file
 *
 *  Read the named file into a strbuf, returning
 *  the entire file in a single strbuf.
 */

extern struct strbuf* strbuf_from_file(char* filename);

/*
 * strbuf_embiggen
 *
 *  Enlarge a strbuf, either in place or by moving to a larger
 *  memory block.
 *  The returned pointer may be the same or different from the 
 *  one given in sbuf.
 *  Do not embiggen a strbuf that is in a list, it may break.
 */

extern struct strbuf * strbuf_embiggen( struct strbuf* sbuf, int newbufsize );

/*
 * strbuf_split
 *
 *  Split one big strbuf into a list of strbufs
 *  delimited by ch.
 *  inbuf will need to be freed by the calling program.
 */

extern struct strbuf* strbuf_split(struct strbuf* inbuf, char ch);

/*
 * strbuf_insert_before
 *
 *  Insert newstrbuf in front of the element chain.
 *  If chain is the first element in a list then it becomes the 
 *  new head.  The calling program must know and deal with this.
 */

extern void strbuf_insert_before(struct strbuf* chain, struct strbuf* newstrbuf);

/*
 * strbuf_insert_after
 *
 *  Insert newstrbuf after the node chain.
 */

extern void strbuf_insert_after(struct strbuf* chain, struct strbuf* newstrbuf);

/*
 * strbuf_remove
 *
 *  Remove the node remove, linking the chain of the surrounding nodes,
 *  and setting the removed link pointers to null.
 */

extern struct strbuf* strbuf_remove( struct strbuf* list, struct strbuf* remove );

/*
 * strbuf_append
 *
 * Add a node to the end of list.
 */

extern void strbuf_append( struct strbuf* list, struct strbuf* abuf );

/*
 * strbuf_cmp
 *
 *  Call strcmp on two strbuf->str
 */

extern int strbuf_cmp(const struct strbuf* s1, const struct strbuf* s2);

/*
 * insert_one
 *
 *  Insert one strbuf into a chain of strbuf's in sorted order.
 */
 
extern struct strbuf* insert_one(struct strbuf* sorted, struct strbuf* newitem);

/*
 * strbuf_sort
 *
 *  Sort a chain of strbufs.
 *  The sort is just an insertion sort.
 *  It works fine for short lists, < 100 good, < 1000 OK.
 *  Not intended for longer lists.
 */

extern struct strbuf* strbuf_sort(struct strbuf* unsorted);


/*
 * strbuf_free_chain
 *
 * Free every node in a strbuf chain.
 */
void strbuf_free_chain(struct strbuf* given_chain);
