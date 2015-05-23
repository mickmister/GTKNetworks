#define PACKET_SIZE 10
#define NUMUSERS 10

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
	int colorIndex;
} PACKET;

typedef struct
{
	int colorIndex;
} INIT_PACKET;

double colors[NUMUSERS][3] = 
{
    {
        1, 0, 0
    },
    {
        0, 1, 0
    },
    {
        0, 0, 1
    },
    {
        1, 1, 0
    },
    {
        1, 0, 1
    },
    {
        0, 1, 1
    },
    {
        0.5, 0, 1
    },
    {
        1, 0.5, 0
    },
    {
        0.5, 1, 0
    },
    {
        0, 0, 0
    }
};

