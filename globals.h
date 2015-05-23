

#define PACKET_SIZE 10
#define NUMUSERS 10
#define CHECK_SIZE 10
#define SPACING 2
#define BRUSH_SIZE_MIN 10
#define BRUSH_SIZE_MAX 20
#define DRAWING_AREA_SIZE 400
#define BUFFER_SIZE_MAX 100

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

double colors[NUMUSERS][3];

