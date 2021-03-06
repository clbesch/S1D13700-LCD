#include "S1D13700.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "WProgram.h"

//============================================================================
const unsigned char SplashText_Screen[30][41]=
	{"                                        ",  // 0
	"                                        ",  // 1
	"                                        ",  // 2
	"                                        ",  // 3
	"                                        ",  // 4
	"                                        ",  // 5
	"                                        ",  // 6
	"                                        ",  // 7
	"                                        ",  // 8
	"                                        ",  // 9
	"                                        ",  // 10
	"                                        ",  // 11
	"                                        ",  // 12
	"                                        ",  // 13
	"                                        ",  // 14
	"                                        ",  // 15
	"                                        ",  // 16
	"                                        ",  // 17
	"                                        ",  // 18
	"                                        ",  // 19
	"                               Built-in ",  // 20
	"                               5x7 font ",  // 21
	"                               on a     ",  // 22
	"                               separate ",  // 23
	"                               text     ",  // 24
	"                               layer.   ",  // 25
	"                               Can be   ",  // 26
	"                               combined ",  // 27
	"                               using AND",  // 28
	"                               OR or XOR"}; // 29


void S1D13700::switchLayer(uint8_t layer) {
    switch (_current_layer) {
    case 1:
        _current_layer_mempos = 0;
        break;
    case 2:
        _current_layer_mempos = SAD2_ADDR;
        break;
    case 3:
        _current_layer_mempos = SAD3_ADDR;
        break;
    default:
        // throw 5; //this is an error throw... probably going to update how this works.
        break;
    }

    //if we make it this far, set the current layer.
    _current_layer = layer;
}

void S1D13700::clearTextLayer() {

    //position cursor at beginning of text laye
    setMemPosition(SAD1_1, SAD1_2);

    command(MEMWRITE);

    //clearing loop
    for(int i = 0; i < SAD1_LEN; i++) {

        //loop through and write a space character to every location in text layer
		write(0x00);
	}
}

void S1D13700::clearLayer2() {

    //position cursor at beginning of second display block
    setMemPosition(SAD2_1, SAD2_2);

    //clear loop
	for(int i = 0; i < SAD2_LEN; i++) {

        //write 0x00 to all locations in the second display buffer
		command(MEMWRITE);
		write(0x00);
	}
}

void S1D13700::clearLayer3() {

    //position cursor at beginning of second display block
    setMemPosition(SAD3_1, SAD3_2);

    //clear loop
	for(int i = 0; i < SAD2_LEN; i++) {
        //write 0x00 to all locations in the second display buffer
		command(MEMWRITE);
		write(0x00);
	}
}
//MY STUFF!!!!

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

S1D13700::S1D13700(uint8_t res, uint8_t a0, uint8_t rw, uint8_t enable, uint8_t cs,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	init(res, a0, rw, enable, cs, d0, d1, d2, d3, d4, d5, d6, d7);
}

//we're not using RW here, just writing.
S1D13700::S1D13700(uint8_t res, uint8_t a0, uint8_t enable, uint8_t cs,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
	init(res, a0, 255, enable, cs, d0, d1, d2, d3, d4, d5, d6, d7);
}

void S1D13700::init(uint8_t res, uint8_t a0, uint8_t rw, uint8_t enable, uint8_t cs,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
    _cursorPos.x = 0;
    _cursorPos.y = 0;

	//tackle all pin assignments
	_res_pin = res;
	_a0_pin = a0;
	_rw_pin = rw;
	_cs_pin = cs;

	_enable_pin = enable;

	_data_pins[0] = d0;
	_data_pins[1] = d1;
	_data_pins[2] = d2;
	_data_pins[3] = d3; 
	_data_pins[4] = d4;
	_data_pins[5] = d5;
	_data_pins[6] = d6;
	_data_pins[7] = d7; 

	pinMode(_res_pin, OUTPUT);
	pinMode(_a0_pin, OUTPUT);
	// pinMode(_cs_pin, OUTPUT);

    // we can save 1 pin by not using RW. Indicate by passing 255 instead of pin#
	if (_rw_pin != 255) { 
		pinMode(_rw_pin, OUTPUT);
	}
	pinMode(_enable_pin, OUTPUT);
}

void S1D13700::begin() {

    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	delayMicroseconds(50000);

    // Now we pull both A0 and R/W low to begin commands
	digitalWrite(_a0_pin, LOW);
	digitalWrite(_enable_pin, LOW);

	if (_rw_pin != 255) { 
		digitalWrite(_rw_pin, LOW);
	}

	write8bits(0x00); // not sure if I should do this in "reset" or not

	//now that we have all data lines at 0 and all control lines properly set, reset.
	reset();

//SYSTEM_SET:
	command(SYSTEM_SET); //system set
	write(0x30); // first register, single panel drive
	write(0x87); //FX - Horizontal character size register (8 pixels)
	write(0x07); //FY - Vertical character size register (8 pixels)
	write(0x27); //C/R - character bytes per row
	write(0x42); //TC/R - total character bytes per row (CHECK CALCULATIONS)
	write(0xEF); //L/F - frame height in lines
	write(0x28); //APL - horizontal address range
	write(0x00); //APH - also horizontal address range!

	// Scroll setup, of course:
	command(SCROLL);
	write(SAD1_2); //screen block 1 start address
	write(SAD1_1); // screen block 1 start address
	write(SL1); // screen block 1 size
	write(SAD2_2);
	write(SAD2_1);
	write(SL2);
	write(SAD3_2);
	write(SAD3_1);
	write(SAD4_2); //these are zero, as we're not using a 4th display area
	write(SAD4_1);

    //HDOT_SCR:
	command(HDOT_SCR);
	write(0x00); //NO SCROLL

    //OVERLAY:
	command(OVLAY);
	write(0x1C);	// SB1 = Graphics
					// SB3 = Text
					// Exculsive-OR
					
	command(DISP_OFF);
	write(0x56); // P1 -- flash cursor!!! yes, fucking FINALLY
    // write(0x54); //cursor off

    //wipe all layers in use
    clearTextLayer();
    clearLayer2();
    clearLayer3();
    
    setMemPosition(SAD1_1, SAD1_2);

    //CSR_FORM:
	command(CSRFORM);
	write(0x04);
	write(0x86);
	
	//turn the display on!
	command(DISP_ON);
	write(0x56); // P1 -- flash cursor!!! yes, fucking FINALLY
	
	//set the cursor direction to "right"
	command(CSRDIR_RIGHT);

    switchLayer(1); //switch so drawing occurs on second layer.
}


/*********** drawing commands ********/

//sets the memory position to the 
void S1D13700::setMemPosition(uint8_t hiByte, uint8_t lowByte) {
    command(CSRW);
	write(lowByte);
	write(hiByte);
}

//cursor goes from 0 to 39 (x), 0 to 29 in text layer (y)
void S1D13700::setCursor(uint8_t xCursor, uint8_t yCursor) {

    //remember that the y position is the line, and the x position is the cursor spot.
    _cursorPos.x = xCursor;
    _cursorPos.y = yCursor;

    uint16_t memPos = _current_layer_mempos + xCursor + (yCursor * 40); //40 bytes across the screen... fix this    

    uint8_t hiNib = (uint8_t)HINIBBLE(memPos);
    uint8_t loNib = (uint8_t)LONIBBLE(memPos);

    setMemPosition(hiNib, loNib);            
}

uint8_t S1D13700::readByte(uint8_t xPos, uint8_t yPos) {
    if ((xPos >= LCDWIDTH / 8) || (yPos >= LCDHEIGHT))
        return 0x00;

    //set the cursor to the proper character position"
    setCursor(xPos, yPos); //the x should be divided by bits per character... y's good as "lines"

    command(MEMREAD);
    return read();
}

// the most basic function, set a single pixel.
// (later, add support for a white or black pen.)
void S1D13700::setPixel(uint16_t xPos, uint16_t yPos, uint8_t color) {
    if ((xPos >= LCDWIDTH) || (yPos >= LCDHEIGHT))
        return;

    //set the cursor to the proper character position"
    setCursor(xPos >> 3, yPos); //the x should be divided by bits per character... y's good as "lines"

    uint8_t xBit = 0x80 >> (xPos % 8); //x position along the character of the bit to be flipped.

    //this functionality can be found in readByte as well, but we want to skip the cursor set. Optimize later!
    command(MEMREAD);
    uint8_t pix = read();

    setCursor(xPos >> 3, yPos); //the x should be divided by bits per character... y's good as "lines"

    //operation
    pix |= xBit;
    
    command(MEMWRITE);
    write(pix);
}

// bresenham's algorithm - thx wikpedia
void S1D13700::drawLine(uint16_t x0, uint8_t y0, uint16_t x1, uint8_t y1, uint8_t color) {
    uint8_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }

    uint8_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int8_t err = dx / 2;
    int8_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;}

    for (; x0<x1; x0++) {
        if (steep) {
            setPixel(y0, x0, color);
        } else {
            setPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

// filled rectangle
void S1D13700::fillRect(uint16_t x, uint8_t y, uint16_t w, uint8_t h, 
                      uint8_t color) {

    // stupidest version - just pixels - but fast with internal buffer!
    for (uint16_t i=x; i<x+w; i++) {
        for (uint8_t j=y; j<y+h; j++) {
            setPixel(i, j, color);
        }
    }
}

// draw a rectangle
void S1D13700::drawRect(uint16_t x, uint8_t y, uint16_t w, uint8_t h, 
                      uint8_t color) {
    // stupidest version - just pixels - but fast with internal buffer!
    for (uint16_t i=x; i<x+w; i++) {
        setPixel(i, y, color);
        setPixel(i, y+h-1, color);
    }
    for (uint8_t i=y; i<y+h; i++) {
        setPixel(x, i, color);
        setPixel(x+w-1, i, color);
    } 
}

// draw a circle outline
void S1D13700::drawCircle(uint16_t x0, uint8_t y0, uint8_t r, 
                        uint8_t color) {
    int8_t f = 1 - r;
    int8_t ddF_x = 1;
    int8_t ddF_y = -2 * r;
    int8_t x = 0;
    int8_t y = r;

    setPixel(x0, y0+r, color);
    setPixel(x0, y0-r, color);
    setPixel(x0+r, y0, color);
    setPixel(x0-r, y0, color);

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
  
        setPixel(x0 + x, y0 + y, color);
        setPixel(x0 - x, y0 + y, color);
        setPixel(x0 + x, y0 - y, color);
        setPixel(x0 - x, y0 - y, color);
    
        setPixel(x0 + y, y0 + x, color);
        setPixel(x0 - y, y0 + x, color);
        setPixel(x0 + y, y0 - x, color);
        setPixel(x0 - y, y0 - x, color);
    
    }
}

void S1D13700::fillCircle(uint16_t x0, uint8_t y0, uint8_t r, 
                        uint8_t color) {
    int8_t f = 1 - r;
    int8_t ddF_x = 1;
    int8_t ddF_y = -2 * r;
    int8_t x = 0;
    int8_t y = r;

    for (uint16_t i= y0 - r; i<=y0+r; i++) {
        setPixel(x0, i, color);
    }

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
  
        for (uint16_t i = y0 - y; i <= y0 + y; i++) {
            setPixel(x0 + x, i, color);
            setPixel(x0 - x, i, color);
        } 
        for (uint16_t i = y0 - x; i <= y0 + x; i++) {
            setPixel(x0 + y, i, color);
            setPixel(x0 - y, i, color);
        }    
    }
}


/*********** mid level commands, for sending data/cmds */

inline void S1D13700::command(uint8_t value) {
	send(value, HIGH);
}

inline void S1D13700::write(uint8_t value) {
	send(value, LOW);
}

uint8_t S1D13700::read() {

    
    uint8_t ret = 0x00;
    //a0 pin goes high to read.
    digitalWrite(_a0_pin, HIGH);
    
    //set rw pin high to read
    digitalWrite(_rw_pin, HIGH);

    digitalWrite(_enable_pin, HIGH);
    delayMicroseconds(10);

    //go through pin array, 
    for (int i = 0; i < 8; i++) {
        pinMode(_data_pins[i], INPUT);
        ret |= (digitalRead(_data_pins[i]) << i);
    }

    digitalWrite(_enable_pin, LOW);
    delayMicroseconds(10);

    return ret;
}

/************ low level data pushing commands **********/


// write either command or data, with automatic 4/8-bit selection
void S1D13700::send(uint8_t value, uint8_t mode) {
	digitalWrite(_a0_pin, mode);

// if there is a RW pin indicated, set it low to Write
	if (_rw_pin != 255) { 
		digitalWrite(_rw_pin, LOW);
	}

	write8bits(value);  
}

void S1D13700::reset() {
	digitalWrite(_res_pin, LOW);
	delayMicroseconds(5000);
	digitalWrite(_res_pin, HIGH);
}

void S1D13700::pulseEnable(void) {
	digitalWrite(_enable_pin, LOW);
	delayMicroseconds(1);
	digitalWrite(_enable_pin, HIGH);
	delayMicroseconds(1);    // enable pulse must be >450ns
	digitalWrite(_enable_pin, LOW);
	delayMicroseconds(10);   // commands need > 37us to settle
}

void S1D13700::write8bits(uint8_t value) {
	for (int i = 0; i < 8; i++) {
		pinMode(_data_pins[i], OUTPUT);
		digitalWrite(_data_pins[i], (value >> i) & 0x01);
	}

	pulseEnable();
}

