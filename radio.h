// Copyright (c) 2014, Robert Harris.

typedef enum { B_FALSE, B_TRUE } boolean_t;
typedef enum { T_NULL, T_TEMPERATURE, T_PRESSURE, T_VOLTAGE } type_t;

struct payload {
	type_t	p_type;
	float		p_value;
	unsigned int	p_seqno;
};

#define	P_SET_ADDRESS	0

#define	ADDR0		0xb7
#define	ADDR1		0xb7
#define	ADDR2		0xb7
#define	ADDR3		0xb7

#define	RADIO_MAGIC	0xcafeba00
