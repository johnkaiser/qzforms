extern void qzrandom64_init(void);
extern uint64_t qzrandom64(void);
extern uint64_t qzrandom64ch(char* buf);
extern void gen_random_key(char keybuf[], int key_length );
extern float qzprobability(void);

enum last_char_rule {last_is_null = 0, last_not_null = 1};
extern void qzrandomch(char* buf, uint8_t bytes, enum last_char_rule rule);

