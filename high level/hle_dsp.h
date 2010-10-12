enum {
	DSPUCODE_HARDROM,
	DSPUCODE_LOADER,
	DSPUCODE_ZWW /* wind waker US */
};

extern int DSPucode;

int is_msg_queue_empty(void);
u32 peek_msg_queue(void);
void pop_msg_queue(void);
void write_msg_queue(u32 msg);

void dsphle_init(void);
void ucode_loader_parse(u32 message);

void ucode_zww_init(void);
void ucode_zww_parse(u32 mesg);
void ucode_zww_processcmd(u32 * data,int size);