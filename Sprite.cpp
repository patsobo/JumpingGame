#include "pch.h"
#include "Sprite.h"

using namespace DirectX;
using namespace std;

Sprite::Sprite(ID3D11ShaderResourceView *m_Texture, XMFLOAT2 size, XMFLOAT2 position, Windows::Foundation::Rect* movementBounds, float scale, float Speed) :
m_Texture(m_Texture), size(size), position(position), movementBounds(movementBounds), rows(1), columns(1), framesPerSecond(1), Velocity(XMFLOAT2(0, 0))
{
	this->position = position;
	this->movementBounds = movementBounds;
	this->m_Texture = m_Texture;
	this->scale = scale;
	this->Speed = Speed;
	this->initialPosition = position;
	this->spritesheet = Spritesheet(size);
	this->color = Colors::White;

	animationState = 0;
	currentFrame = 0;
	totalFrames = rows*columns;
	BoundingBox = CreateBoundingBoxFromPosition(Sprite::position);
	shouldChangeAnimation = false;
	timeSinceLastFrame = 0;
	rotation = 0.0f;
	spriteEffects = SpriteEffects_None;
}

Sprite::Sprite(ID3D11ShaderResourceView *m_Texture, XMFLOAT2 size, XMFLOAT2 position, Windows::Foundation::Rect* movementBounds, XMVECTORF32* color, float scale, float Speed) :
m_Texture(m_Texture), size(size), position(position), movementBounds(movementBounds), rows(1), columns(1), framesPerSecond(1), Velocity(XMFLOAT2(0, 0))
{
	this->position = position;
	this->movementBounds = movementBounds;
	this->m_Texture = m_Texture;
	this->scale = scale;
	this->Speed = Speed;
	this->initialPosition = position;
	this->spritesheet = Spritesheet(size);
	this->color = *color;

	animationState = 0;
	currentFrame = 0;
	totalFrames = rows*columns;
	BoundingBox = CreateBoundingBoxFromPosition(Sprite::position);
	shouldChangeAnimation = false;
	timeSinceLastFrame = 0;
	rotation = 0.0f;
	spriteEffects = SpriteEffects_None;
}

//Sprite::Sprite(ID3D11ShaderResourceView *m_Texture, XMFLOAT2 size, XMFLOAT2 position, Windows::Foundation::Rect* movementBounds, float scale) :
//m_Texture(m_Texture), size(size), position(position), movementBounds(movementBounds), rows(1), columns(1), framesPerSecond(1)
//{
//	this->position = position;
//	this->movementBounds = movementBounds;
//	this->m_Texture = m_Texture;
//	this->size = size;
//	this->scale = scale;
//
//	for (int i = 0; i < 5; i++)
//	{
//		this->dividers[i] = 0;
//	}
//
//	animationState = 0;
//	currentFrame = 0;
//	totalFrames = rows*columns;
//	BoundingBox = CreateBoundingBoxFromPosition(Sprite::position);
//	shouldChangeAnimation = false;
//	timeSinceLastFrame = 0;
//}

Sprite::Sprite(ID3D11ShaderResourceView *m_Texture, XMFLOAT2 size, XMFLOAT2 position, Windows::Foundation::Rect* movementBounds, int rows, int columns,
	double framesPerSecond, int dividers[])
{
	this->position = position;
	this->initialPosition = position;
	this->movementBounds = movementBounds;
	this->rows = rows;
	this->columns = columns;
	this->framesPerSecond = framesPerSecond;
	this->spritesheet = Spritesheet(size);
	scale = 1;
	this->color = Colors::White;

	this->m_Texture = m_Texture;
	this->size = size;

	animationState = 0;
	currentFrame = 0;
	totalFrames = rows*columns;
	BoundingBox = CreateBoundingBoxFromPosition(this->position);
	shouldChangeAnimation = false;
	timeSinceLastFrame = 0;
	rotation = 0.0f;
	spriteEffects = SpriteEffects_None;
}

Sprite::Sprite(Spritesheet* spritesheet, ID3D11ShaderResourceView *m_Texture, XMFLOAT2 position,
	Windows::Foundation::Rect* movementBounds, float Speed, double framesPerSecond, float scale)
{
	this->position = position;
	this->initialPosition = position;
	this->movementBounds = movementBounds;
	this->framesPerSecond = framesPerSecond;
	this->spritesheet = *spritesheet;
	this->Speed = Speed;
	this->m_Texture = m_Texture;
	this->color = Colors::White;

	currentFrame = 0;
	this->scale = scale;
	rotation = 0.0f;
	BoundingBox = CreateBoundingBoxFromPosition(this->position);
	timeSinceLastFrame = 0;
	spriteEffects = SpriteEffects_None;
}

void Sprite::Draw(SpriteBatch* spriteBatch)
{
	double imageWidth = spritesheet.currentSize.x;
	double imageHeight = spritesheet.currentSize.y;

	RECT* sourceRectangle = new RECT;
	sourceRectangle->left = spritesheet.currentPosition.x;
	sourceRectangle->top = spritesheet.currentPosition.y;
	sourceRectangle->right = spritesheet.currentPosition.x + spritesheet.currentSize.x;
	sourceRectangle->bottom = spritesheet.currentPosition.y + spritesheet.currentSize.y;

	spriteBatch->Draw(m_Texture, XMFLOAT2(position.x, position.y),
		sourceRectangle, color, rotation, XMFLOAT2(0.0f, 0.0f),
		XMFLOAT2(getWidth() / imageWidth, getHeight() / imageHeight),
		spriteEffects, 0.0f);
}

void Sprite::Update(float timeTotal, float timeDelta)
{
	UpdateAnimation(timeTotal, timeDelta);
	UpdatePosition(timeTotal, timeDelta);
}

void Sprite::UpdateAnimation(float timeTotal, float timeDelta)
{
	timeSinceLastFrame += timeDelta;

	// Continue animating as normal
	if (timeSinceLastFrame > SecondsBetweenFrames())
	{
		currentFrame++;
		timeSinceLastFrame = 0;
	}

	// If you reach the end of your animation, loop that animation again
	if (currentFrame >= spritesheet.numOfFrames[spritesheet.currentAnimation])
	{
		currentFrame = 0;
		animationPlayedOnce = true;
	}
	//else currentFrame = 0;

	// Update the source rectangle
	spritesheet.Update(spritesheet.currentAnimation, currentFrame);
}

void Sprite::UpdatePosition(float timeTotal, float timeDelta)
{
	XMFLOAT2 newPosition;
	newPosition.x = position.x + ((Velocity.x * timeDelta) * Speed);
	newPosition.y = position.y + ((Velocity.y * timeDelta) * Speed);

	if (Blocked(newPosition))
	{
		BoundingBox = CreateBoundingBoxFromPosition(position);
		return;
	}
	BoundingBox = CreateBoundingBoxFromPosition(newPosition);
	position = newPosition;
}

void Sprite::reset()
{
	position = initialPosition;
	Velocity = XMFLOAT2(0, 0);
}

double Sprite::SecondsBetweenFrames() { return 1.0 / framesPerSecond; }

bool Sprite::Blocked(XMFLOAT2 newPosition)
{
	BoundingBox = CreateBoundingBoxFromPosition(newPosition);

	Windows::Foundation::Point topLeft(BoundingBox->X, BoundingBox->Y);
	Windows::Foundation::Point topRight(BoundingBox->X + getWidth(), BoundingBox->Y);
	Windows::Foundation::Point bottomLeft(BoundingBox->X, BoundingBox->Y + getHeight());
	Windows::Foundation::Point bottomRight(BoundingBox->X + getWidth(), BoundingBox->Y + getHeight());

	return !((movementBounds->Contains(topLeft)) && (movementBounds->Contains(topRight)) && (movementBounds->Contains(bottomLeft)) &&
		(movementBounds->Contains(bottomRight)));
}

Windows::Foundation::Rect* Sprite::CreateBoundingBoxFromPosition(XMFLOAT2 position)
{
	return new Windows::Foundation::Rect(position.x, position.y, getWidth(), getHeight());
}

bool Sprite::CollidesWith(Sprite* that)
{
	if (this->BoundingBox->IntersectsWith(*that->getBoundingBox()))
		return true;
	return false;
}

void Sprite::NormalizeVelocity()
{
	if (Velocity.x == 0 && Velocity.y == 0)
		return;
	float magnitude = sqrt(pow(Velocity.x, 2) + pow(Velocity.y, 2));
	Velocity.x /= magnitude;
	Velocity.y /= magnitude;
}

void Sprite::adjustPosition()
{
	XMFLOAT2 adjustVelocity = XMFLOAT2(-1 * Velocity.x, -1 * Velocity.y);
	position.x += adjustVelocity.x * POS_ADJUST;
	position.y += adjustVelocity.y * POS_ADJUST;
	BoundingBox = CreateBoundingBoxFromPosition(position);
}

void Sprite::setOpacity(float opacity) {
	if (opacity > 1 || opacity < 0)
		return;
	XMVECTORF32 newColor = { color.f[0], color.f[1], color.f[2], opacity };
	color = newColor;
}

float Sprite::getWidth() { return spritesheet.currentSize.x * scale; }
float Sprite::getHeight() { return spritesheet.currentSize.y * scale; }
XMFLOAT2 Sprite::getVelocity() { return Velocity; }
void Sprite::setAnimation(int newAnim)
{
	spritesheet.currentAnimation = newAnim;
	currentFrame = 0;
	animationPlayedOnce = false;
	// Needed to be called here because otherwise the 1st frame of the old animation
	// will play before switching to the new animation.
	// Also, the base sprite update needs to be called first in child updates
	spritesheet.Update(spritesheet.currentAnimation, currentFrame);
}
int Sprite::getAnimation() { return spritesheet.currentAnimation; }
void Sprite::setFrame(int newFrame)
{
	if (newFrame < spritesheet.numOfFrames[spritesheet.currentAnimation] && newFrame >= 0)
		currentFrame = newFrame;
}
int Sprite::getNumFrames()
{
	return spritesheet.numOfFrames[spritesheet.currentAnimation];
}
void Sprite::setVelocity(XMFLOAT2 newVelocity)
{
	Velocity = newVelocity;
	NormalizeVelocity();
}
Windows::Foundation::Rect* Sprite::getBoundingBox() { return BoundingBox; }
void Sprite::setFPS(float newFPS) {
	framesPerSecond = newFPS;
}
void Sprite::setSpeed(float newSpeed) {
	Speed = newSpeed;
}
float Sprite::getFPS() {
	return framesPerSecond;
}