extern void qzrandom_init(void);
extern uint64_t qzrandom64(void);
extern void gen_random_key(char keybuf[], int key_length );

enum last_char_rule {last_is_null = 0, last_not_null = 1};
extern void qzrandomch(char* buf, uint8_t bytes, enum last_char_rule rule);

