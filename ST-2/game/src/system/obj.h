/**
 * オブジェクト
 * - 名前空間 OBJ_ または obj
 * - オブジェクトの管理
 *
 * @author Snail Barbarian Macho (NWK)
 */

#ifndef OBJ_H_INCLUDED
#define OBJ_H_INCLUDED

#include "../../../../src-common/common.h"

// ---------------------------------------------------------------- Obj 構造体
/** Obj 幾何学情報 構造体 */
// ---- この 10 バイトはアセンブラで使うので変更禁止!
typedef struct s_ObjGeo {
    s16 sx, x;              // X 速度と速度(s7.8). ※アセンブラで計算しやすいように, 速度が先です
    u8  w;                  // 衝突幅(u7. 0なら衝突判定なし)
    s16 sy, y;              // Y 速度と速度(s7.8). ※アセンブラで計算しやすいように, 速度が先です
    u8  h;                  // 衝突高さ(u8)
} ObjGeo;
typedef struct s_ObjGeo8 {
    u8  sxl;                // X 速度小数部
    s8  sxh;                // X 速度整数部
    u8  xl;                 // X 座標小数部
    s8  xh;                 // X 座標整数部
    u8  w;                  // 衝突幅
    u8  syl;                // Y 速度小数部
    s8  syh;                // Y 速度整数部
    u8  yl;                 // Y 座標小数部
    s8  yh;                 // Y 座標整数部
    u8  h;                  // 衝突高さ
} ObjGeo8;

/** Obj Work 構造体 */
typedef struct s_ObjWorkPlayer {
    u8  attractInput;       // アトラクト モードでの入力
} ObjWorkPlayer;
typedef struct s_ObjWorkPlayerBullet {
    u8  text1, text2;       // 弾の TEXT
    u8  atb1,  atb2;        // 弾の ATB
} ObjWorkPlayerBullet;
typedef struct s_ObjWorkItem {
    u8  subLevel;           // アイテムのサブ レベル
} ObjWorkItem;
typedef struct s_ObjWorkEnemy {
    u16 score;              // 加算されるスコア
    u8  itemSubLevel;       // 吐き出される アイテムのサブ レベル 1～7
    u8  nrItems;            // 吐き出される最大アイテム数 1～10くらい
    u8  ct;                 // 汎用カウンタ
    u8  animCt;             // アニメ用カウンタ
    u8  dir;                // 現在の方向 [0, 8]
                            // 6  7  8
                            //  ＼|／
                            // 5--0--1
                            //  ／|＼
                            // 4  3  2
    u8  bulletMode;         // 弾モード(Enemy4以降)
} ObjWorkEnemy;
typedef struct s_ObjWorkScore {
    u16 score;              // 表示されるスコア
} ObjWorkScore;

/** Obj 構造体 */
typedef struct s_Obj // 構造体の大きさは 2 のべき乗であること!
{
    // ※は, create 時に自動初期化
    // ---- 位置, 速度, 寸法
    union u_Geo {
        ObjGeo  geo;
        ObjGeo8 geo8;
    } uGeo;

    // ---- 関数
    // 1 フレームに 1 回呼ばれる関数です. false を返すと消えます. nullable.
    bool (*mainFunc)(struct s_Obj* const);
    // 描画時に呼ばれる関数です. nullable.
    //    drawAddr は, x, y から計算されるアドレスです
    void (*drawFunc)(struct s_Obj* const, u8* drawAddr);

    // ---- フラグ, 体力, 攻撃力, ステップ, カウンタ
    bool    bHit;           // 衝突検出
    u16     fitness;        // 体力
    u8      offence;        // 攻撃力 ヒットすると体力を減らしていく
    u8      step;           // 汎用ステップ
    u8      ct;             // 汎用カウンタ

    // ---- システム管理
    struct s_Obj*  pNext;
    struct s_Obj*  pPrev;

    // ---- 各オブジェクト特有のデータ
    union u_ObjWork {
        u8                  dummy1[8];
        ObjWorkPlayer       player;
        ObjWorkPlayerBullet playerBullet;
        ObjWorkItem         item;
        ObjWorkEnemy        enemy;
        ObjWorkScore        score;
    } uObjWork;
} Obj;

/** Obj 構造体のオフセット. アセンブラで使う */
#define OBJ_OFFSET_GEO_SX       0
#define OBJ_OFFSET_GEO_X        2
#define OBJ_OFFSET_GEO_W        4
#define OBJ_OFFSET_GEO_SY       5
#define OBJ_OFFSET_GEO_Y        7
#define OBJ_OFFSET_GEO_H        9
#define OBJ_OFFSET_GEO8_SXL     0
#define OBJ_OFFSET_GEO8_SXH     1
#define OBJ_OFFSET_GEO8_XL      2
#define OBJ_OFFSET_GEO8_XH      3
#define OBJ_OFFSET_GEO8_W       4
#define OBJ_OFFSET_GEO8_SYL     5
#define OBJ_OFFSET_GEO8_SYH     6
#define OBJ_OFFSET_GEO8_YL      7
#define OBJ_OFFSET_GEO8_YH      8
#define OBJ_OFFSET_GEO8_H       9
#define OBJ_OFFSET_MAIN_FUNC    10
#define OBJ_OFFSET_DRAW_FUNC    12
#define OBJ_OFFSET_B_HIT        14
#define OBJ_OFFSET_FITNESS      15
#define OBJ_OFFSET_OFFENCE      17
#define OBJ_OFFSET_STEP         18
#define OBJ_OFFSET_CT           19
#define OBJ_OFFSET_P_NEXT       20
#define OBJ_OFFSET_P_PREV       22
#define OBJ_OFFSET_WORK_PLAYER_BULLET_TEXT1  24
#define OBJ_OFFSET_WORK_PLAYER_BULLET_TEXT2  25
#define OBJ_OFFSET_WORK_PLAYER_BULLET_ATB1   26
#define OBJ_OFFSET_WORK_PLAYER_BULLET_ATB2   27
#define OBJ_OFFSET_WORK_ITEM_SUB_LEVEL       24

// ---------------------------------------------------------------- システム
/**
 * Obj 管理システムの初期化
 */
void objInit() __z88dk_fastcall;
/** 敵弾を全て初期化します */
void objInitEnemyBullet();
/** アイテムを全て初期化します */
void objInitItem();
/**
 * Obj 管理システム メイン
 * - 1 フレームに 1 回呼びます
 */
void objMain() __z88dk_fastcall;

// ---------------------------------------------------------------- 生成
/**
 * Obj を生成します - プレーヤー
 * @param initFunc 初期化関数. nullable
 * @param mainFunc 1 フレームに 1 回呼ばれる関数. false を返すと消えます
 * @param drawFunc 表示時に呼ばれる関数. nullable
 * @param pParent  この Obj を生成した親. nullable
 * @return 生成に成功したら ポインタを返すので, 各自初期化(座標等)をしてください.
 *   メモリ不足で失敗したら, nullptr を返します
 */
Obj* objCreatePlayer(
    void (*initFunc)(Obj* const, Obj* const),
    bool (*mainFunc)(Obj* const),
    void (*drawFunc)(Obj* const, u8* drawAddr),
    Obj* const pParent);
/**
 * Obj を生成します - プレーヤー弾
 * パラメータ, 戻値については, @see objCreatePlayer
 */
Obj* objCreatePlayerBullet(
    void (*initFunc)(Obj* const, Obj* const),
    bool (*mainFunc)(Obj* const),
    void (*drawFunc)(Obj* const, u8* drawAddr),
    Obj* const pParent);
/**
 * Obj を生成します - 敵
 * パラメータ, 戻値については, @see objCreatePlayer
 */
Obj* objCreateEnemy(
    void (*initFunc)(Obj* const, Obj* const),
    bool (*mainFunc)(Obj* const),
    void (*drawFunc)(Obj* const, u8* drawAddr),
    Obj* const pParent);
/**
 * Obj を生成します - 敵弾
 * パラメータ, 戻値については, @see objCreatePlayer
 */
Obj* objCreateEnemyBullet(
    void (*initFunc)(Obj* const, Obj* const),
    bool (*mainFunc)(Obj* const),
    void (*drawFunc)(Obj* const, u8* drawAddr),
    Obj* const pParent);
/**
 * Obj を生成します - アイテム
 * パラメータ, 戻値については, @see objCreatePlayer
 */
Obj* objCreateItem(
    void (*initFunc)(Obj* const, Obj* const),
    bool (*mainFunc)(Obj* const),
    void (*drawFunc)(Obj* const, u8* drawAddr),
    Obj* const pParent);
/**
 * Obj を生成します - その他
 * パラメータ, 戻値については, @see objCreatePlayer
 */
Obj* objCreateEtc(
    void (*initFunc)(Obj* const, Obj* const),
    bool (*mainFunc)(Obj* const),
    void (*drawFunc)(Obj* const, u8* drawAddr),
    Obj* const pParent);

// ---------------------------------------------------------------- 初期化マクロ
/** 初期化マクロ. Obj の initFunc 内で呼んでください. 初期化忘れ防止になります
 * _x, _y, _sx, _sy は s7.8 です
 * _w, _h, は u8 です
 */
#define OBJ_INIT(_pObj, _x, _y, _w, _h, _sx, _sy) do {\
    _pObj->uGeo.geo.x = (_x);       \
    _pObj->uGeo.geo.y = (_y);       \
    _pObj->uGeo.geo.w = (_w);       \
    _pObj->uGeo.geo.h = (_h);       \
    _pObj->uGeo.geo.sx = (_sx);     \
    _pObj->uGeo.geo.sy = (_sy);     \
} while (false)

// ---------------------------------------------------------------- ユーティリティ
/** 使用中のプレーヤー オブジェクトを返します. nullptr もありえます */
Obj* objGetInUsePlayer();
/** 使用中の敵オブジェクトを返します. nullptr もありえます */
Obj* objGetUserdEnemy();


#endif // OBJ_H_INCLUDED
