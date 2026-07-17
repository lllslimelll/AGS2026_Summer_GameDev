#pragma once
#include "Vector3.h"
class Quaternion;

// 4x4 行列
struct Matrix4x4
{
    float m[4][4];

    // ---------------------------------------------------------------
    // コンストラクタ
    // ---------------------------------------------------------------

    // 単位行列で初期化
    Matrix4x4(void);

    // 要素を直接指定
    Matrix4x4(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33);

    // ---------------------------------------------------------------
    // 生成
    // ---------------------------------------------------------------

    // 単位行列
    static Matrix4x4 Identity(void);

    // 平行移動行列
    static Matrix4x4 Translation(const Vector3& pos);

    // スケール行列
    static Matrix4x4 Scale(const Vector3& scl);

    // クォータニオンから回転行列を生成
    static Matrix4x4 Rotation(const Quaternion& q);

    // 位置・回転・スケールを合成した Transform 行列を生成
    static Matrix4x4 Compose(
        const Vector3& pos,
        const Quaternion& rot,
        const Vector3& scl);

    // ---------------------------------------------------------------
    // 演算
    // ---------------------------------------------------------------

    // 行列の乗算
    Matrix4x4 operator*(const Matrix4x4& other) const;

    // ---------------------------------------------------------------
    // 変換
    // ---------------------------------------------------------------

    // 点を変換する（平行移動あり）
    // Unreal の FMatrix::TransformPosition に相当
    Vector3 TransformPosition(const Vector3& pos) const;

    // ベクトルを変換する（平行移動なし）
    // Unreal の FMatrix::TransformVector に相当
    Vector3 TransformVector(const Vector3& vec) const;

    // ---------------------------------------------------------------
    // 分解
    // ---------------------------------------------------------------

    // 行列から位置を取得
    Vector3 GetPosition(void) const;

    // 行列からスケールを取得
    Vector3 GetScale(void) const;

    // 行列からクォータニオンを取得
    Quaternion GetRotation(void) const;

    // ---------------------------------------------------------------
    // DxLib との変換
    // ---------------------------------------------------------------
    // DxLib の MATRIX に変換（MV1SetMatrix などに渡すとき）
    // Core 層は DxLib を知らないが
    // Transform など境界層でのみ使用する
    struct DxLibMatrix { float m[4][4]; }; // MATRIX 相当
};