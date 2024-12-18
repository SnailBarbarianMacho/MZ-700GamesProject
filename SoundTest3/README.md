# SoundTest3 6音源 / 6 sound sources  <!-- omit in toc -->

* 1982年頃 ZX Spextrum では既に6重音が実現されてると聞いたので, MZ-700 でも挑戦しました

## 目次 <!-- omit in toc -->
- [原理](#原理)
- [機能](#機能)
- [実装サンプルについて](#実装サンプルについて)

## 原理
* SoundTest とほぼ同じです. PFM(パルス周波数変調) による波形を OR で合成<br>
  ![合成原理](SoundTest3.png)
* 音源が多いのでパルス幅が狭くなってます
* パルス幅の調整で音量調整を行ってます. パルス幅が狭いのでちょっと音量処理は汚いです

## 機能
* Lead x 2ch:
  * 同じ音を時間差をつけて再生しており, エコー感を出してます
  * 3種類のエンベロープ (減衰速度調整可能)
  * スラー, ポルタメント (速度調整可能)
* Chord x 2ch:
  * 3種類のエンベロープ (減衰速度は固定)
  * 音長やエンベロープを共用して, データ削減を図ってます
* Base x 1ch:
  * 3種類のエンベロープ (減衰速度は固定)
* Drum x 1ch:
  * 1bit PCM, 256 サンプル (約0.018秒) x 3種類
  * ドラム データは, テキストで作ります
  * 波形が '1' のほうが処理に時間がかかってるので, 連打するとテンポが遅れる可能性があります
* リピート (最大8回 x 4重ネスト)
* F1 及び F2 キーによる中断機能 (中断禁止にもできます)
* 低い音程パルス幅を広く再生します. 倍音を重ねても両方の音が聴こえる筈です
* 自己書換を多用して, SoundTest に比べて処理速度が上がりました. 処理ループは最短約 14.2KHz です

## 実装サンプルについて
* サンプル曲として Tim Follin 氏の 'Chronos' をカバーしました.<br>
  再生時間は約3分です
* リピートによるランレングス圧縮のおかげで, 音符量が約 3/4 になってます
* 終了したらモニター ROM に戻ります (たぶん)<br>
  実行アドレスは, 0x1200 なので, 再実行できます
