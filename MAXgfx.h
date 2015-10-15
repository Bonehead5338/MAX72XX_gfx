// MAX72XX_gfx.h

#ifndef _MAX72XX_GFX_h
#define _MAX72XX_GFX_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <MAX72XX.h>

#define SPRITE_LOCATIONS 8
#define SPRITE_LOCATION_0 0x01
#define SPRITE_LOCATION_1 0x02
#define SPRITE_LOCATION_2 0x04
#define SPRITE_LOCATION_3 0x08
#define SPRITE_LOCATION_4 0x10
#define SPRITE_LOCATION_5 0x20
#define SPRITE_LOCATION_6 0x40
#define SPRITE_LOCATION_7 0x80

//TODO Make the sprite class a base class only to hide the init function. Subclasses will have initSubClass (eg initRectangle)
class MAXSprite
{

public:
	enum enumEdges : uint8_t {
		TopEdge = 0x01,
		BottomEdge = 0x02,
		VerticalEdges = 0x03,
		LeftEdge = 0x04,
		RightEdge = 0x08,
		HorizontalEdges = 0x0C,
		AllEdges = 0x0F
	};

protected:

	//sprite data (max = 8x8)
	uint8_t SpriteData[8];

	//Width and height of sprite
	uint8_t Width = 0;
	uint8_t Height = 0;

	//pointer to base sprite
	uint8_t DisplayData[MATRIX_DIM];

	//sprite position
	int PositionX;
	int PositionY;

	//position is constrained to edges;
	uint8_t PositionConstraints;

	//sprite is to be displayed
	bool Show;

	uint8_t OnEdge;
	uint8_t OverEdge;
	uint8_t OutOfBounds;

	static uint8_t ConstrainToMatrixDimensions(uint8_t dimension) { return dimension < MATRIX_DIM ? dimension : MATRIX_DIM; }
	void setConstrainedPosition(int position_x, int position_y, uint8_t edgeconstraints);
	void detectEdges();

public:

	MAXSprite() {};
	MAXSprite(const uint8_t* const data, uint8_t width, uint8_t height, int position_x = 0, int position_y = 0, uint8_t pos_constraints = 0, bool show = true); 
	void initSprite(const uint8_t* data, uint8_t width, uint8_t height, int position_x = 0, int position_y = 0, uint8_t pos_constraints = 0, bool show = true);
	
	void setPosition(int position_x, int position_y);
	void move(int distance_x, int distance_y);

	void setPositionConstraints(uint8_t constraints);

	//show/hide public methods
	void show() { Show = true; }
	void hide() { Show = false; }
	bool isShown() { return Show; }
	bool isHidden() { return !Show; }

	//edge detection methods
	bool onEdge(enumEdges edge) { return OnEdge & edge; }
	uint8_t onEdge() { return OnEdge; }
	bool overEdge(enumEdges edge) { return OverEdge & OverEdge; }
	uint8_t overEdge() { return OverEdge; }
	bool outOfBounds(enumEdges edge) { return OutOfBounds & edge; }
	uint8_t outOfBounds() { return OutOfBounds; }

	//position and dimension getters
	int getPositionX() { return PositionX; }
	int getPositionY() { return PositionY; }
	uint8_t getWidth() { return Width; }
	uint8_t getHeight() { return Height; }

	const uint8_t* getDisplayData();
};

/** A fixed-width sprite with multiple frames */
class MAXSprite_MultiFrame : public MAXSprite
{
protected:
	//number of frames in animation
	uint8_t FrameCount;
	uint8_t* FrameData;
	uint8_t CurrentFrame = 0;

	bool reverse = false;

	void forwardFrame();
	void reverseFrame();

public:

	MAXSprite_MultiFrame(uint8_t** data, uint8_t frame_count, uint8_t width, uint8_t height, int position_x = 0, int position_y = 0, bool constrain_pos = false, bool show = true);

	bool loadFrame(uint8_t index);
	void nextFrame();

	//direction public methods
	void setDirForward() { reverse = false; }
	void setDirReverse() { reverse = true; }
	void toggleDirection(){ reverse = !reverse; }
	bool isReversed() { return reverse; }
};

class MAXSprite_Rectangle : public MAXSprite
{
protected:
	uint8_t BorderThickness;
	bool Filled;
public:
	//constructor
	MAXSprite_Rectangle() {}
	MAXSprite_Rectangle(uint8_t width, uint8_t height, uint8_t border_thickness = 1, bool filled = false, int position_x = 0, int position_y = 0, uint8_t position_constraints = 0, bool show = true);
	void initRectangle(uint8_t width, uint8_t height, uint8_t border_thickness = 1, bool filled = false, int position_x = 0, int position_y = 0, uint8_t position_constraints = 0, bool show = true);

};

class MAXSprite_StraightLine : public MAXSprite
{
protected:

public:
	//constructor
	MAXSprite_StraightLine() {};
	MAXSprite_StraightLine(uint8_t length, uint8_t thickness, bool vertical = false, int position_x = 0, int position_y = 0, uint8_t constrain_pos = false, bool show = true);
	void initStraightLine(uint8_t length, uint8_t thickness, bool vertical = false, int position_x = 0, int position_y = 0, uint8_t constrain_pos = false, bool show = true);

};

class MAXgfx
{

protected:

	MAX72XX MAX;
	
	MAXSprite* Sprites[SPRITE_LOCATIONS];
	uint8_t DisplayData[MATRIX_DIM];

public:

	MAXgfx(int LOAD_PIN) : Sprites(), MAX(LOAD_PIN) {}

	void init() { MAX.init(); };


	//void init(MAX72XX max72xx); 

	bool addSprite(MAXSprite& sprite);
	bool removeSprite(uint8_t location);
	bool replaceSprite(uint8_t location, MAXSprite sprite);

	void updateDisplay();

	MAXSprite* getSprite(uint8_t location);
	void getSpriteCopy(uint8_t location, MAXSprite* sprite);

	void setSpritePosition(uint8_t index , int position_x, int position_y);
	void moveSprite(int move_x, int move_y);

	//wrapper functions for MAX7221 class
	void setIntensity(uint8_t intensity) { MAX.setIntensity(intensity); }
	void setShutDownMode(bool shutdown) { MAX.setShutDownMode(shutdown); }
	void setTestMode(bool test_mode) { MAX.setTestMode(test_mode); }
	uint8_t getIntensity() { return MAX.getIntensity(); }
	bool getShutdownMode() { return MAX.getShutdownMode(); }
	bool getDisplayTestMode() { return MAX.getDisplayTestMode(); }
};


#endif
