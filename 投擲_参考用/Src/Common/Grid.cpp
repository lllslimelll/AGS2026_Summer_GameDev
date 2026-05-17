#include <DxLib.h>
#include "Grid.h"

Grid::Grid(void)
{
}

Grid::~Grid(void)
{
}

void Grid::Init(void)
{
}

void Grid::Update(void)
{
}

void Grid::Draw(void)
{
	for (int z = -10; z < 10; z++)
	{
		VECTOR sPos = { -1000.0f, 0.0f, static_cast<float>(z * 100)};
		VECTOR ePos = { 1000.0f, 0.0f,  static_cast<float>(z * 100)};
		DrawLine3D(sPos, ePos, 0xff0000);
		DrawSphere3D(ePos, 20.0f, 10, 0xff0000, 0xff0000, true);
	}

	for (int x = -10; x < 10; x++)
	{
		VECTOR sPos = { static_cast<float>(x * 100), 0.0f, -1000 };
		VECTOR ePos = { static_cast<float>(x * 100), 0.0f, 1000 };
		DrawLine3D(sPos, ePos, 0x0000ff);
		DrawSphere3D(ePos, 20.0f, 10, 0x0000ff, 0x0000ff, true);
	}
}

void Grid::Release(void)
{
}
