#pragma once
#include <DxLib.h>
#include "../Core/Vector2.h"

class Material;

// 2D矩形ポリゴンにピクセルシェーダーをかけて描画するクラス
class PixelRenderer
{
public:

    explicit PixelRenderer(Material& material);

    void MakeSquareVertex(Vector2 size, Vector2 pos);
    void Draw(void);

private:

    static constexpr int NUM_VERTEX = 4;
    static constexpr int NUM_VERTEX_INDEX = 6;
    static constexpr int NUM_POLYGON = 2;

    Material& material_;

    Vector2          pos_;
    Vector2          size_;
    VERTEX2DSHADER   vertexs_[NUM_VERTEX];
    WORD             indexes_[NUM_VERTEX_INDEX];

    void MakeVertexIndex(void);
    void SetToDevice(void);
    void Reset(void);
};