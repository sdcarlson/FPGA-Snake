#include "PmodJSTK2.h"
#include "PmodOLEDrgb.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "stdio.h"
#include "stdlib.h"
#include "sleep.h"
#include "bitmap.h"
#include "xgpio.h" // Include the XGpio header file

#define WIDTH 12
#define HEIGHT 8

#ifdef __MICROBLAZE__
#define CPU_CLOCK_FREQ_HZ (XPAR_CPU_CORE_CLOCK_FREQ_HZ)
#else
#define CPU_CLOCK_FREQ_HZ (XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ)
#endif
PmodOLEDrgb oledrgb;
PmodJSTK2 joystick;
XGpio ssd; /* The Instance of the GPIO Driver */

u8 rgbUserFont[] = {
    0x00, 0x04, 0x02, 0x1F, 0x02, 0x04, 0x00, 0x00, // 0x00
    0x0E, 0x1F, 0x15, 0x1F, 0x17, 0x10, 0x1F, 0x0E, // 0x01
    0x00, 0x1F, 0x11, 0x00, 0x00, 0x11, 0x1F, 0x00, // 0x0
    0x00, 0x00, 0x3C, 0x3C, 0x3C, 0x3C, 0x00, 0x00, // 0x3
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  // 0x04
};                                                  // This table defines 5 user characters, although only one is used

const u32 pattern[14] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x98, 0xFF, 0xBF, 0x9C, 0xC6};
const u32 anode[8] = {0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F};

void DemoInitialize();
void DemoRun();
void DemoCleanup();
void EnableCaches();
void DisableCaches();

enum Direction
{
    STOP = 0,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

struct SnakeGame
{
    int gameOver;
    int score;
    int snakeX, snakeY;
    int fruitX, fruitY;
    int tailX[100], tailY[100];
    int tailLength;
    enum Direction dir;
};

void setup(struct SnakeGame *game)
{
    game->gameOver = 0;
    game->dir = STOP;
    game->snakeX = WIDTH / 2;
    game->snakeY = HEIGHT / 2;
    game->fruitX = rand() % WIDTH;
    game->fruitY = rand() % HEIGHT;
    game->score = 0;
    game->tailLength = 0;
}

int logic(struct SnakeGame *game)
{
	int scored = 0;
    int prevX = game->tailX[0];
    int prevY = game->tailY[0];
    int prev2X, prev2Y;

    game->tailX[0] = game->snakeX;
    game->tailY[0] = game->snakeY;

    if (game->snakeX == game->fruitX && game->snakeY == game->fruitY)
    {
    	scored = 1;
        game->score += 1;
        game->tailLength++;
        game->fruitX = rand() % WIDTH;
        game->fruitY = rand() % HEIGHT;
        int is_tail = 1;
        while (is_tail) {
            is_tail = 0;
            int fruitx = game->fruitX;
            int fruity = game->fruitY;

            for (int i = 1; i < game->tailLength; i++) {
                int tailx = game->tailX[i];
                int taily = game->tailY[i];

                if (tailx == fruitx && taily == fruity) {
                    is_tail = 1;
                    game->fruitX = rand() % WIDTH;
                    game->fruitY = rand() % HEIGHT;
                    break; // Exit the loop early since we found a match
                }
            }
        }
    }

    for (int i = 1; i < game->tailLength; i++)
    {
        prev2X = game->tailX[i];
        prev2Y = game->tailY[i];
        game->tailX[i] = prevX;
        game->tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    switch (game->dir)
    {
    case LEFT:
        game->snakeX--;
        break;
    case RIGHT:
        game->snakeX++;
        break;
    case UP:
        game->snakeY--;
        break;
    case DOWN:
        game->snakeY++;
        break;
    }

    if (game->snakeX < 0)
        game->snakeX = WIDTH - 1;
    if (game->snakeX >= WIDTH)
        game->snakeX = 0;
    if (game->snakeY < 0)
        game->snakeY = HEIGHT - 1;
    if (game->snakeY >= HEIGHT)
        game->snakeY = 0;

    for (int i = 0; i < game->tailLength; i++)
    {
        if (game->tailX[i] == game->snakeX && game->tailY[i] == game->snakeY)
            game->gameOver = 1;
    }
    return scored;
}

void DemoInitialize()
{
    EnableCaches();

    // Initialize the joystick device
    JSTK2_begin(
        &joystick,
        XPAR_PMODJSTK2_0_AXI_LITE_SPI_BASEADDR,
        XPAR_PMODJSTK2_0_AXI_LITE_GPIO_BASEADDR);

    // Set inversion register to invert only the Y axis
    JSTK2_setInversion(&joystick, 0, 1);

    OLEDrgb_begin(&oledrgb, XPAR_PMODOLEDRGB_0_AXI_LITE_GPIO_BASEADDR,
                  XPAR_PMODOLEDRGB_0_AXI_LITE_SPI_BASEADDR);

    int status = XGpio_Initialize(&ssd, XPAR_GPIO_0_DEVICE_ID); // Initialize GPIO
    if (status != XST_SUCCESS)
    {
        return;
    }

    XGpio_SetDataDirection(&ssd, 1, 0x00); // Set the direction for all signals to be outputs
}

void DisplayNumber(int digit1, int digit2);

void DemoRun()
{

    char ch;

    // Define the user definable characters
    for (ch = 0; ch < 5; ch++)
    {
        OLEDrgb_DefUserChar(&oledrgb, ch, &rgbUserFont[ch * 8]);
    }

    struct SnakeGame game;
    setup(&game); // Initialize the snake game
    JSTK2_Position position;
    JSTK2_DataPacket rawdata;

    int delay, counter = 0, digit1 = 1, digit2 = 3, limit = 2000;

    if (game.dir == STOP) {
                       	   OLEDrgb_SetCursor(&oledrgb, 1, 2);
                       	    OLEDrgb_SetFontColor(&oledrgb, OLEDrgb_BuildRGB(255, 0, 0)); // Blue font
                       	    OLEDrgb_PutString(&oledrgb, "Move JSTK");
                        	   OLEDrgb_SetCursor(&oledrgb, 1, 1);
    }

    while (!game.gameOver)
    {

    	digit1 = game.score % 10;
    	digit2 = game.score / 10;
    	XGpio_DiscreteWrite(&ssd, 1, anode[0]);
        XGpio_DiscreteWrite(&ssd, 2, pattern[digit1]);
        for (delay = 0; delay < 1000; delay++)
            ; // Delay for stability

        // Display second digit
        XGpio_DiscreteWrite(&ssd, 1, anode[1]);
        XGpio_DiscreteWrite(&ssd, 2, pattern[digit2]);
        for (delay = 0; delay < 1000; delay++);

        if (counter == limit)
        {

            OLEDrgb_Clear(&oledrgb);


            if (game.dir == STOP) {
                               	   OLEDrgb_SetCursor(&oledrgb, 1, 2);
                               	    OLEDrgb_SetFontColor(&oledrgb, OLEDrgb_BuildRGB(255, 0, 0)); // Blue font
                               	    OLEDrgb_PutString(&oledrgb, "Move JSTK");
                                	   OLEDrgb_SetCursor(&oledrgb, 1, 3);
              }
            // Get joystick x and y coordinate values
            position = JSTK2_getPosition(&joystick);
            // Get button states
            rawdata = JSTK2_getDataPacket(&joystick);

            xil_printf(
                "X:%d\tY:%d snake: %d,%d\r\n",
                game.fruitY, game.fruitX,
                game.snakeX, game.snakeY);

            // Set led from btns and axis
            if (rawdata.Jstk != 0 || rawdata.Trigger != 0)
            {
                JSTK2_setLedRGB(&joystick, 0, 255, 0);
            }
            else
            {
                JSTK2_setLedRGB(&joystick, position.XData, 0, position.YData);
            }

            if (position.YData > 200 && game.dir != LEFT)
            {
                game.dir = RIGHT;
            }
            else if (position.YData < 128 - (200 - 128) && game.dir != RIGHT)
            {
                game.dir = LEFT;
            }

            if (position.XData > 200 && game.dir != UP)
            {
                game.dir = DOWN;
            }
            else if (position.XData < 128 - (200 - 128) && game.dir != DOWN)
            {
                game.dir = UP;
            }

            int scored = logic(&game);
            if (scored) {
            	limit -= 17;
            }
            OLEDrgb_SetFontColor(&oledrgb, OLEDrgb_BuildRGB(0, 0, 255));
            OLEDrgb_SetCursor(&oledrgb, game.fruitX, game.fruitY);
            OLEDrgb_PutChar(&oledrgb, 1);

            OLEDrgb_SetFontColor(&oledrgb, OLEDrgb_BuildRGB(0, 255, 0));

            OLEDrgb_SetCursor(&oledrgb, game.snakeX, game.snakeY);
            OLEDrgb_PutChar(&oledrgb, 4);

            for (int k = 0; k < game.tailLength; k++)
            {
                OLEDrgb_SetCursor(&oledrgb, game.tailX[k], game.tailY[k]);
                OLEDrgb_PutChar(&oledrgb, 3);
            }
            counter = 0;
        } else {
        	counter++;
        }
    }

    OLEDrgb_Clear(&oledrgb);

    OLEDrgb_SetCursor(&oledrgb, 1, 4);
    OLEDrgb_SetFontColor(&oledrgb, OLEDrgb_BuildRGB(255, 0, 0));
    OLEDrgb_PutString(&oledrgb, "Game over!");

    DisplayNumber(digit1, digit2);

}

void DemoCleanup()
{
    DisableCaches();
}

void EnableCaches()
{
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_DCacheEnable();
#endif
#endif
}

void DisableCaches()
{
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_DCACHE
    Xil_DCacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_ICACHE
    Xil_ICacheDisable();
#endif
#endif
}


// Function to display two-digit number of anode display
void DisplayNumber(int digit1, int digit2)
{
    int delay;
    for (int i = 0; i < 100000000000000; i++)
    {
        // Display first digit
        XGpio_DiscreteWrite(&ssd, 1, anode[0]);
        XGpio_DiscreteWrite(&ssd, 2, pattern[digit1]);
        for (delay = 0; delay < 1000; delay++)
            ; // Delay for stability

        // Display second digit
        XGpio_DiscreteWrite(&ssd, 1, anode[1]);
        XGpio_DiscreteWrite(&ssd, 2, pattern[digit2]);
    }
}

int main()
{
    DemoInitialize();
    DemoRun();
    DemoCleanup();
}
