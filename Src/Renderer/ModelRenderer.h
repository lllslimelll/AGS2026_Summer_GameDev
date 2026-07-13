#pragma once
#include <DxLib.h>
class Material;

// MV1モデルをオリジナルシェーダーで描画するクラス
class ModelRenderer
{
public:

    // modelH は Transform::modelId を参照渡しで持つ
    ModelRenderer(Material& material, int& modelH);
    ~ModelRenderer(void);

    void Draw(void);

private:

    Material& material_;
    int& modelH_;

    void SetToDevice(void);
    void Reset(void);
};