#define PACKET_SIZE 10

typedef struct
{
	unsigned int x;
	unsigned int y;
	unsigned int brushSize;
} COORDINATE_PAIR;

typedef struct
{
	COORDINATE_PAIR array[PACKET_SIZE];
	int length;
} PACKET;

typedef struct
{
	double r;
	double b;
	double g;
} INIT_PACKET;

