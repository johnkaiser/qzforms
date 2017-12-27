extern void qzrandom64_init(void);
extern uint64_t qzrandom64(void);
extern uint64_t qzrandom64ch(char* buf);
extern void gen_random_key(char keybuf[], int key_length );
extern float qzprobability(void);

struct rand128 {
    uint64_t rnbr[2];
};

extern struct rand128 qzrandom128ch(char* buf);

