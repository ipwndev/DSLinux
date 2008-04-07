Noiz2sa  readme.txt
for Windows98/2000/XP
ver. 0.51
(C) Kenta Cho

アブストラクトシューティング、Noiz2sa。


○ インストール方法

noiz2sa0_51.zipを適当なフォルダに展開してください。
その後、'noiz2sa.exe'を実行してください。


○ 遊び方

キーボードかジョイスティックでステージを選んでください。

 - 移動         矢印キー / ジョイステック
 - ショット     [Z]      / トリガ1, トリガ4
 - スローダウン [X]      / トリガ2, トリガ3
 - ポーズ       [P]

ショットキーでゲームを開始します。

自機を操作して、弾幕を避けてください。
敵本体に接触しても自機は破壊されません。
スローダウンキーを押している間、自機が遅くなります。

緑の星はボーナスアイテムです。
連続して取ることで、アイテムの得点（左上に表示）が上昇します。

自機がすべて破壊されると、ゲームオーバーです。
自機は200,000点および500,000点ごとに1機増えます。

以下のコマンドラインオプションが指定できます。
 -nosound       音を出力しません。
 -window        ウィンドウモードで起動します。
 -reverse       ショットとスローダウンのキーを入れ替えます。
 -brightness n  画面の明るさを設定します(n=0-256)。
 -accframe      別のフレームレート制御方法を利用します。
                （フレームレートが一定しないなどの問題がある場合に
                  利用してください。）


○ オリジナル弾幕の追加

オリジナルの弾幕を書いて、Noiz2saに追加することができます。
'noiz2sa'ディレクトリ内に、'zako', 'middle', 'boss'の3つの
ディレクトリがあり、この中に弾幕パターンファイルが配置されます。

弾幕パターンファイルはBulletMLで書かれています。
BulletMLについては、以下のページを参照ください。

BulletML
http://www.asahi-net.or.jp/~cs8k-cyu/bulletml/index.html 

'zako'ディレクトリは、雑魚用のデータ、
'middle'ディレクトリは、中型機用のデータ、
'boss'ディレクトリは、大型機用のデータです。

弾幕パターンファイルを記述する場合には、$rank変数で
弾幕の難易度を調整する必要があります。
$rank変数は、Noiz2sa内で、各シーンの難易度調整用に利用されます。


○ ご意見、ご感想

コメントなどは、cs8k-cyu@asahi-net.or.jp までお願いします。


○ 謝辞

BulletMLファイルのパースにlibBulletMLを利用しています。
 libBulletML
 http://user.ecc.u-tokyo.ac.jp/~s31552/wp/libbulletml/

画面の出力にはSimple DirectMedia Layerを利用しています。
 Simple DirectMedia Layer
 http://www.libsdl.org/

BGMとSEの出力にSDL_mixerとOgg Vorbis CODECを利用しています。
 SDL_mixer 1.2
 http://www.libsdl.org/projects/SDL_mixer/
 Vorbis.com
 http://www.vorbis.com/


○ ヒストリ

2003  8/10  ver. 0.51
            libBulletMLのアップデート。
2003  2/12  ver. 0.5
            -accframeオプションの追加。
            弾幕追加。
2003  1/ 3  ver. 0.42
            弾幕修正。
2003  1/ 3  ver. 0.41
            弾幕調整。
2002 12/31  ver. 0.4
            ENDLESS INSANE追加。
            弾幕追加。
2002 11/23  ver. 0.32
            無敵時間の調整。
2002 11/ 9  ver. 0.31
            自機の移動可能範囲の修正。
            -brightnessオプションの追加。
2002 11/ 3  ver. 0.3
            初公開版。


○ ライセンス

Noiz2saはBSDスタイルライセンスのもと配布されます。

License
-------

Copyright 2002 Kenta Cho. All rights reserved. 

Redistribution and use in source and binary forms, 
with or without modification, are permitted provided that 
the following conditions are met: 

 1. Redistributions of source code must retain the above copyright notice, 
    this list of conditions and the following disclaimer. 

 2. Redistributions in binary form must reproduce the above copyright notice, 
    this list of conditions and the following disclaimer in the documentation 
    and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
