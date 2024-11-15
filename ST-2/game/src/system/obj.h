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
    u8  attract_input;      // アトラクト モードでの入力
} ObjWorkPlayer;
typedef struct s_ObjWorkPlayerBullet {
    u8  text1, text2;       // 弾の TEXT
    u8  atb1,  atb2;        // 弾の ATB
} ObjWorkPlayerBullet;
typedef struct s_ObjWorkItem {
    u8  sub_level;          // アイテムのサブ レベル
} ObjWorkItem;
typedef struct s_ObjWorkEnemy {
    u16 score;              // 加算されるスコア
    u8  item_sub_level;     // 吐き出される アイテムのサブ レベル 1～7
    u8  nr_items;           // 吐き出される最大アイテム数 1～10くらい
    u8  ct;                 // 汎用カウンタ
    u8  anim_ct;            // アニメ用カウンタ
    u8  dir;                // 現在の方向 [0, 8]
                            // 6  7  8
                            //  ＼|／
                            // 5--0--1
                            //  ／|＼
                            // 4  3  2
    u8  bullet_mode;        // 弾モード(Enemy4以降)
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
    } u_geo;

    // ---- 関数
    // 1 フレームに 1 回呼ばれる関数です. false を返すと消えます. nullable.
    bool (*main_func)(struct s_Obj* const);
    // 描画時に呼ばれる関数です. nullable.
    //    draw_addr は, x, y から計算されるアドレスです
    void (*draw_func)(struct s_Obj* const, u8* draw_addr);

    // ---- フラグ, 体力, 攻撃力, シーン, カウンタ
    bool    b_hit;          // 衝突検出
    u16     fitness;        // 体力
    u8      offence;        // 攻撃力 ヒットすると体力を減らしていく
    u8      step;           // 汎用シーン
    u8      ct;             // 汎用カウンタ

    // ---- システム管理
    struct s_Obj*  p_next;
    struct s_Obj*  p_prev;

    // ---- 各オブジェクト特有のデータ
    union u_ObjWork {
        u8                  dummy1[8];
        ObjWorkPlayer       player;
        ObjWorkPlayerBullet player_bullet;
        ObjWorkItem         item;
        ObjWorkEnemy        enemy;
        ObjWorkScore        score;
    } u_obj_work;
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

// ---------------------------------------------------------------- private 変数. 直接触らない
extern Obj* p_obj_in_use_player_;
extern Obj* p_obj_in_use_playerBullet_;
extern Obj* p_obj_in_use_enemy_;
extern Obj* p_obj_in_use_enemy_bullet_;
extern Obj* p_obj_in_use_item_;
extern Obj* p_obj_in_use_etc_;

// ---------------------------------------------------------------- システム
/**
 * Obj 管理システムの初期化
 */
void objInit(void) __z88dk_fastcall;
/** 敵弾を全て初期化します */
void objInitEnemyBullet(void);
/** アイテムを全て初期化します */
void objInitItem(void);
/**
 * Obj 管理システム メイン
 * - 1 フレームに 1 回呼びます
 */
void objMain(void) __z88dk_fastcall;

// ---------------------------------------------------------------- 生成
/**
 * Obj を生成します - プレーヤー
 * @param init_func 初期化関数. nullable
 * @param main_func 1 フレームに 1 回呼ばれる関数. false を返すと消えます
 * @param draw_func 表示時に呼ばれる関数. nullable
 * @param p_parent  この Obj を生成した親. nullable
 * @return 生成に成功したら ポインタを返すので, 各自初期化(座標等)をしてください.
 *   メモリ不足で失敗したら, nullptr を返します
 */
Obj* objCreatePlayer(
    void (*init_func)(Obj* const, Obj* const),
    bool (*main_func)(Obj* const),
    void (*draw_func)(Obj* const, u8* draw_addr),
    Obj* const p_parent);
/**
 * Obj を生成します - プレーヤー弾
 * パラメータ, 戻値については, @see objCreatePlayer
 */
Obj* objCreatePlayerBullet(
    void (*init_func)(Obj* const, Obj* const),
    bool (*main_func)(Obj* const),
    void (*draw_func)(Obj* const, u8* draw_addr),
    Obj* const p_parent);
/**
 * Obj を生成します - 敵
 * パラメータ, 戻値については, @see objCreatePlayer
 */
Obj* objCreateEnemy(
    void (*init_func)(Obj* const, Obj* const),
    bool (*main_func)(Obj* const),
    void (*draw_func)(Obj* const, u8* draw_addr),
    Obj* const p_parent);
/**
 * Obj を生成します - 敵弾
 * パラメータ, 戻値については, @see objCreatePlayer
 */
Obj* objCreateEnemyBullet(
    void (*init_func)(Obj* const, Obj* const),
    bool (*main_func)(Obj* const),
    void (*draw_func)(Obj* const, u8* draw_addr),
    Obj* const p_parent);
/**
 * Obj を生成します - アイテム
 * パラメータ, 戻値については, @see objCreatePlayer
 */
Obj* objCreateItem(
    void (*init_func)(Obj* const, Obj* const),
    bool (*main_func)(Obj* const),
    void (*draw_func)(Obj* const, u8* draw_addr),
    Obj* const p_parent);
/**
 * Obj を生成します - その他
 * パラメータ, 戻値については, @see objCreatePlayer
 */
Obj* objCreateEtc(
    void (*init_func)(Obj* const, Obj* const),
    bool (*main_func)(Obj* const),
    void (*draw_Func)(Obj* const, u8* draw_addr),
    Obj* const p_parent);

// ---------------------------------------------------------------- 初期化マクロ
/** 初期化マクロ. Obj の init_func 内で呼んでください. 初期化忘れ防止になります
 * _x, _y, _sx, _sy は s7.8 です
 * _w, _h, は u8 です
 */
#define OBJ_INIT(_p_obj, _x, _y, _w, _h, _sx, _sy) do {\
    _p_obj->u_geo.geo.x = (_x);       \
    _p_obj->u_geo.geo.y = (_y);       \
    _p_obj->u_geo.geo.w = (_w);       \
    _p_obj->u_geo.geo.h = (_h);       \
    _p_obj->u_geo.geo.sx = (_sx);     \
    _p_obj->u_geo.geo.sy = (_sy);     \
} while (false)

// ---------------------------------------------------------------- ユーティリティ
/** 使用中のプレーヤー オブジェクトを返します. nullptr もありえます */
inline Obj* objGetInUsePlayer(void) { return p_obj_in_use_player_; }
/** 使用中の敵オブジェクトを返します. nullptr もありえます */
inline Obj* objGetUserdEnemy(void) { return p_obj_in_use_enemy_; }


#endif // OBJ_H_INCLUDED
