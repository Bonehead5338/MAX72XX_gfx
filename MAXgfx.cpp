// 
// 
// 

#include "MAXgfx.h"

// unnamed namespace for static functions
namespace
{
	//function prototypes
	void ClearMatrix(uint8_t* input, bool invert = false);
	void OrMatrix(uint8_t* input1_result, const uint8_t* input2);
	void TransposeMatrix(uint8_t* input, int move_x, int move_y);
	void CopyMatrix(const uint8_t* input, uint8_t* output);
	bool MaskMatrix(uint8_t* input, uint8_t size_x, uint8_t size_y);
	bool AreSpritesOverlapped(const MAXSprite* Sprite1, const MAXSprite* Sprite2);

	//function definitions
	void ClearMatrix(uint8_t* input, bool invert)
	{
		//clear all values (or set all if invert is set
		for (uint8_t i = 0; i < MATRIX_DIM; i++)
			if (invert)
				*(input + i) = 0xFF;
			else
				*(input + i) = 0x00;
	}

	void OrMatrix(uint8_t* input1_result, const uint8_t* input2)
	{
		for (uint8_t i = 0; i < MATRIX_DIM; i++)
		{
			*(input1_result + i) |= *(input2 + i);
		}

	}

	void TransposeMatrix(uint8_t* input, int move_x, int move_y)
	{
		//transpose x
		for (uint8_t i = 0; i < MATRIX_DIM; i++)
		{
			if (abs(move_x) >= MATRIX_DIM)
				input[i] = 0x00;
			else
			{
				if (move_x > 0)
					*(input + i) >>= move_x;
				else if (move_x < 0)
					*(input + i) <<= abs(move_x);
			}
		}

		//transpose y
		if (move_y)
		{
			//temporary storage for transposed matrix
			uint8_t temp[8];

			for (uint8_t i = 0; i < MATRIX_DIM; i++)
			{
				if (abs(move_y) >= MATRIX_DIM)
					temp[i] = 0x00;
				else
				{
					if (i - move_y >= 0 && i - move_y < MATRIX_DIM)
					{
						*(temp + i) = *(input + i - move_y);
					}
					else *(temp + i) = 0x00;
				}
			}

			CopyMatrix(temp, input);
		}
	}

	void CopyMatrix(const uint8_t* input, uint8_t* output)
	{
		for (uint8_t i = 0; i < MATRIX_DIM; i++)
			*(output + i) = *(input + i);
	}

	bool MaskMatrix(uint8_t* input, uint8_t size_x, uint8_t size_y)
	{
		//check for valid mask values
		if (size_x >= MATRIX_DIM || size_y >= MATRIX_DIM)
			return false;

		//mask (clear) all outside defined dimensions
		for (uint8_t i = 0; i < MATRIX_DIM; i++)
		{
			if (i < size_y)
			{
				uint8_t mask = 0xFF << (MATRIX_DIM - size_x);
				*(input + i) &= mask;
			}
			else
				*(input + i) = 0x00;
		}
	}

	bool AreSpritesOverlapped(MAXSprite* Sprite1, MAXSprite* Sprite2, uint8_t* result = NULL)
	{
		bool found_overlap = false;
		
		uint8_t sprite1_data[8];
		uint8_t sprite2_data[8];

		CopyMatrix(Sprite1->getDisplayData(), sprite1_data);
		CopyMatrix(Sprite2->getDisplayData(), sprite2_data);

		for (uint8_t i = 0; i < MATRIX_DIM; i++)
		{
			uint8_t row_result = (sprite1_data[i] & sprite2_data[i]);
			if (row_result) found_overlap = true;
			
			//return overlapped areas if result is non-null
			if (result) result[i] = row_result;
		}

		return found_overlap;
	}
}


void MAXSprite::setConstrainedPosition(int position_x, int position_y, uint8_t position_constraints)
{
	//copy position to class members
	PositionX = position_x;
	PositionY = position_y;
	
	//constrain if required
	if (position_constraints & TopEdge)
		PositionX = (position_x < 0) ? 0 : position_x;
	if (position_constraints & BottomEdge)
		PositionX = (position_x > MATRIX_DIM - Height) ? MATRIX_DIM - Height : position_x;
	if (position_constraints & LeftEdge)
		PositionY = (position_y < 0) ? 0 : position_y;
	if (position_constraints & RightEdge)
		PositionY = (position_y > MATRIX_DIM - Width) ? MATRIX_DIM - Width : position_y;
}

void MAXSprite::detectEdges()
{
	//OnEdge
	OnEdgeDectionResults = 0;
	if (PositionY == 0) OnEdgeDectionResults |= TopEdge;
	if (PositionY + Height == MATRIX_DIM) OnEdgeDectionResults |= BottomEdge;
	if (PositionX == 0) OnEdgeDectionResults |= LeftEdge;
	if (PositionX + Width == MATRIX_DIM) OnEdgeDectionResults |= RightEdge;

	//OverEdge
	OverEdgeDetectionResults = 0;
	if (PositionY < 0) OverEdgeDetectionResults |= TopEdge;
	if (PositionY + Height > MATRIX_DIM) OverEdgeDetectionResults |= BottomEdge; 
	if (PositionX < 0) OverEdgeDetectionResults |= LeftEdge;
	if (PositionX + Width > MATRIX_DIM) OverEdgeDetectionResults |= RightEdge;
	
	//OutofBounds
	OutOfBoundsDetectionResults = 0; 
	if (PositionY + Height <= 0) OutOfBoundsDetectionResults |= TopEdge;
	if (PositionY >= MATRIX_DIM) OutOfBoundsDetectionResults |= BottomEdge;
	if (PositionX + Width <= 0) OutOfBoundsDetectionResults |= LeftEdge;
	if (PositionX >= MATRIX_DIM) OutOfBoundsDetectionResults |= RightEdge;
}

void MAXSprite::updateDisplayData()
{
	ClearMatrix(DisplayData);
	CopyMatrix(SpriteData, DisplayData);
	MaskMatrix(DisplayData, Width, Height);
	TransposeMatrix(DisplayData, PositionX, PositionY);
}

uint8_t MAXSprite::isTouchingSprite(MAXSprite& other)
{
	
	//test left/right edges
	if (PositionY + Height - 1 >= other.PositionY && PositionY < other.PositionY + other.Height)
	{
		if (PositionX + Width == other.PositionX)
			return RightEdge;
		else if (PositionX == other.PositionX + other.Width)
			return LeftEdge;
	}

	//test top/bottom edges
	if (PositionX + Width - 1 >= other.PositionX && PositionX < other.PositionX + other.Width)
	{
		if (PositionY + Height == other.PositionY)
			return TopEdge;
		else if (PositionY == other.PositionY + other.Height)
			return BottomEdge;
	}
	
	//test left corners
	if (other.PositionX + other.Width == PositionX)
	{
		if (other.PositionY + other.Height == PositionY)
			return (TopEdge | LeftEdge);
		else if (PositionY + Height == other.PositionY)
			return (BottomEdge | LeftEdge);
	}

	//test right corners
	if (PositionX + Width == other.PositionX)
	{
		if (other.PositionY + other.Height == PositionY)
			return (TopEdge | RightEdge);
		else if (PositionY + Height == other.PositionY)
			return (BottomEdge | RightEdge);
	}

	return 0;
}


MAXSprite::MAXSprite(const uint8_t* data, uint8_t width, uint8_t height, int position_x /*= 0*/, int position_y /*= 0*/, uint8_t position_constraints /*= NoEdges*/, bool show /*= true*/)
{
	initSprite(data, width, height, position_x, position_y, position_constraints, show);
}

void MAXSprite::initSprite(const uint8_t* data, uint8_t width, uint8_t height, int position_x /*= 0*/, int position_y /*= 0*/, uint8_t position_constraints /*= NoEdges*/, bool show /*= true*/)
{
	//copy data to internal storage
	CopyMatrix(data, SpriteData);

	//constrain dimensions to size of matrix
	Width = ConstrainToMatrixDimensions(width);
	Height = ConstrainToMatrixDimensions(height);

	//set position 
	PositionConstraints = position_constraints & 0x0F;

	//set position with new values
	setPosition(position_x, position_y);
	
	//set show/hide value as required
	Show = show;
}

void MAXSprite::setPosition(int position_x, int position_y)
{
	//set position using configured constraints
	setConstrainedPosition(position_x, position_y, PositionConstraints);

	//position has changed, update edge detection values
	detectEdges();
}

void MAXSprite::move(int distance_x, int distance_y)
{
	setPosition(PositionX + distance_x, PositionY + distance_y);	
}

void MAXSprite::setPositionConstraints(uint8_t constraints)
{
	PositionConstraints = constraints & 0xF;
}

const uint8_t* MAXSprite::getDisplayData()
{
	updateDisplayData();
	return DisplayData;
}

uint8_t MAXSprite::getDisplayRow(uint8_t row)
{
	updateDisplayData();

	if (row < MATRIX_DIM)
		return DisplayData[row];
	else
		return 0x00;
}

bool MAXgfx::addSprite(MAXSprite& sprite)
{
	for (uint8_t i = 0; i < SPRITE_LOCATION_CNT; i++)
	{
		//find first open (null) slot and store sprite
		if (!Sprites[i])
		{
			Sprites[i] = &sprite;
			return true;
		}
	}

	//didn't find an open slot
	return false;
}

bool MAXgfx::removeSprite(uint8_t location)
{
	//check for valid location
	if (location >= SPRITE_LOCATION_CNT) return false;

	//clear location
	Sprites[location] = NULL;
	
	//move all sprites to lower locations (fill the gap if it's there)
	for (uint8_t i = location + 1; i < SPRITE_LOCATION_CNT; i++)
	{
		//check for filled (non-null) higher slots and move them down by one to fill gap
		if (Sprites[i])
		{
			Sprites[i - 1] = Sprites[i];
			Sprites[i] = NULL;
		}
	}

	return true;
}

bool MAXgfx::replaceSprite(uint8_t location, MAXSprite sprite)
{
	//check for valid location
	if (location >= SPRITE_LOCATION_CNT) return false;

	//replace sprite at given location
	Sprites[location] = &sprite;

	return true;
}

void MAXgfx::updateDisplay()
{
	
	//clear display data
	ClearMatrix(DisplayData);

	//get all sprite data and OR together
	for (uint8_t i = 0; i < SPRITE_LOCATION_CNT; i++)
	{
		//check for filled (non-null) sprites and or with result
		if (Sprites[i])
		{
			if (Sprites[i]->isShown())
				OrMatrix(DisplayData, Sprites[i]->getDisplayData());
		}
	}

	//send to MAX72XX
	MAX.setMatrix(DisplayData);

}

MAXSprite_MultiFrame::MAXSprite_MultiFrame(uint8_t** data, uint8_t frame_count, uint8_t width, uint8_t height, int position_x, int position_y, bool position_constraints, bool show)
{
	//initialize base sprite class
	MAXSprite::initSprite((uint8_t*)data, width, height, position_x, position_y, position_constraints, show);

	FrameCount = frame_count;
	FrameData = (uint8_t*)data;
}

bool MAXSprite_MultiFrame::loadFrame(uint8_t frame)
{
	if (frame >= FrameCount)
		return false;

	//load frame data as base class sprite
	CopyMatrix((FrameData + (frame * MATRIX_DIM)), SpriteData);
	
	//update current frame
	CurrentFrame = frame;

	return true;
}

void MAXSprite_MultiFrame::nextFrame()
{
	if (reverse)
		reverseFrame();
	else
		forwardFrame();
}

void MAXSprite_MultiFrame::forwardFrame()
{
	//increment Current Frame, wrap back round to zero at FrameCount
	CurrentFrame = (CurrentFrame + 1) % FrameCount;
	loadFrame(CurrentFrame);
}

void MAXSprite_MultiFrame::reverseFrame()
{
	//decrement Current Frame, wrap back round to FrameCount - 1 below zero
	CurrentFrame = CurrentFrame > 0 ? CurrentFrame - 1 : FrameCount - 1;
	loadFrame(CurrentFrame);
}

void MAXSprite_StraightLine::initStraightLine(uint8_t length, uint8_t thickness, bool vertical /*= false*/, int position_x /*= 0*/, int position_y /*= 0*/, uint8_t position_constraints /*= NoEdges*/, bool show /*= true*/)
{
	//limit length and thickness to size of matrix
	uint8_t line_length = ConstrainToMatrixDimensions(length);
	uint8_t line_thickness = ConstrainToMatrixDimensions(thickness);

	//width and height of line
	uint8_t width = vertical ? line_thickness : line_length;
	uint8_t height = vertical ? line_length : line_thickness;
	
	//generate line sprite
	uint8_t line_data[8];
	for (uint8_t i = 0; i < MATRIX_DIM; i++)
	{
		line_data[i] = i < height ? 0xFF << (MATRIX_DIM - width) : 0x00;
	}

	//store local data
	Length = line_length;
	Thickness = line_thickness;
	Vertical = vertical;

	//initialize base class
	MAXSprite::initSprite(line_data, width, height, position_x, position_y, position_constraints, show);
}

MAXSprite_StraightLine::MAXSprite_StraightLine(uint8_t length, uint8_t thickness, bool vertical /*= false*/, int position_x /*= 0*/, int position_y /*= 0*/, uint8_t position_constraints /*= NoEdges*/, bool show /*= true*/)
{
	initStraightLine(length, thickness, vertical, position_x, position_y, position_constraints, show);	
}

MAXSprite_Rectangle::MAXSprite_Rectangle(uint8_t width, uint8_t height, uint8_t border_thickness /*= 1*/, bool filled /*= false*/, int position_x /*= 0*/, int position_y /*= 0*/, uint8_t position_constraints /*= NoEdges*/, bool show /*= true*/)
{
	initRectangle(width, height, border_thickness, filled, position_x, position_y, position_constraints, show);
}

void MAXSprite_Rectangle::initRectangle(uint8_t width, uint8_t height, uint8_t border_thickness /*= 1*/, bool filled /*= false*/, int position_x /*= 0*/, int position_y /*= 0*/, uint8_t position_constraints /*= NoEdges*/, bool show /*= true*/)
{
	//check if border thickness is valid (use maximum valid value if too thick, border thickness of 1 for smallest dim of 1)
	uint8_t smallest_dim = min(width, height);
	uint8_t max_border_thickness = smallest_dim > 1 ? smallest_dim / 2 : 1;
	BorderThickness = (border_thickness > max_border_thickness) ? max_border_thickness : border_thickness;

	//generate square sprite
	uint8_t rectangle_data[8];

	for (uint8_t i = 0; i < MATRIX_DIM; i++)
	{
		if (i < BorderThickness)
			rectangle_data[i] = 0xFF << (MATRIX_DIM - width);
		else if (i < height - BorderThickness)
			rectangle_data[i] = filled ? 0xFF << (MATRIX_DIM - width) : (0xFF << (MATRIX_DIM - BorderThickness)) | (((0xFF >> (MATRIX_DIM - BorderThickness)) << (MATRIX_DIM - width)));
		else if (i < height)
			rectangle_data[i] = 0xFF << (MATRIX_DIM - width);
		else
			rectangle_data[i] = 0x00;
	}

	//initialize base class
	MAXSprite::initSprite(rectangle_data, width, height, position_x, position_y, position_constraints, show);
}
