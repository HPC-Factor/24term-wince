24term - 80x24 vt100 terminal emulator for WindowsCE
         Copyright (c) 2000-2002 Zoroyoshi, SOFTCLUB, Japan
         http://www.softclub.jp/~zoro/ce/

2002-4-12 updated

■MIPS用バイナリセット 2002-4-12

WindowsCEハンドヘルド機上で80x24行表示を行うvt100ターミナルです。
TCP/IPでのtelnet、RS232C接続が可能です。
ほどほどの機能とほどほどのスピードです。

・このプログラムは下記BSDライセンスに従います。無保証です。
・WindowsCE2.0以降の英語版・日本語版、画面は640x240あるいは480x240、
  4色白黒か256色か6万色のMIPS HPC機で動作します。
・動作確認はMobilePro700、Sigmarion、Sigmarion2で行っております。

■改定履歴

1998-8-2  作成開始。
2000-2-29 最初のバージョン。JNetHackに同包。
2000-3-31 4色白黒に対応。JNetHackに同包。
2002-1-31 6万色に対応、複数接続、設定ダイアログ、スクロールバー追加。
          ソースをちょっとだけ見やすくして単独で公開。
2002-3-31 コピーペースト、Bold改善、SSL、RSライン、CE3.0デバッグ。
2002-4-12 BSD接続デバッグ。

■インストール方法

(1)添付のフォント k12x10.ttf を、CE上の \Windows\Fonts にコピーします。
   昔のJNetHackに添付していたk12x10.ttfは間違った漢字を含むので、
   同包している k12x10.ttfに入れ替えてください。

(2)プログラム 24term.exe を、CE上の \Windows にコピーします。
   実際にはどこのディレクトリでも動作しますが、JNetHackが24term.exeを
   参照するので \Windowsへ置いてください。

(3)必要により手動でスタートメニューにショートカットを作成してください。
   ショートカットに接続先と設定ファイル名を引数として渡せます。
      \Windows\24term.exe [接続先] [設定ファイル名]

   \ではじまらない引数は接続先と解釈されます。
   TCP/IPホスト名の場合はドメイン部も必要です。

   設定ファイルは拡張子.24のテキストファイル(SJIS)です。
   (設定ダイアログでメニューから保存できます。)
   引数に指定する場合は絶対パス、つまり、 \ 記号で開始します。
   (WindowsCEにはカレントディレクトリの概念がありません。)
   ファイル名に空白を含む場合は " "で囲ってください。

■接続方法

24termを実行すると接続先などを指定するダイアログが現れます。
   sun.softclub.jp              telnetポート
   10.11.12.13:80               アドレス、ポート番号指定
   :2000p                       2000番で待ち受け
   #1:9600:7e                   RS232Cポート
のように指定します。WindowsCEにはデフォルトドメインの概念がないので
必ずFQDN、つまりドメイン部も含んで指定します。

RS232Cポートの指定方法は
  #ポート番号かデバイス名:通信速度:78neo12hsi
です。それぞれ 7bit/8bit/NoneParity/EvenParity/OddParity/
1StopBit/2StopBits/HardwareFlow/SoftwareFlow/setIR(赤外線)
に対応しています。デフォルトは9600:8n1です。

■設定ダイアログ

メニューボタンタップかAlt短押しでメニューを表示させて、
Setup...で設定ダイアログが開きます。

文字色や背景絵などの設定が可能です。背景に使えるのは、
480x240ドット以上で、128色以下に減色した8bit/pixelの
ビットマップファイルのみです。481x240ドットの大きさがよい具合です。

スクロールバッファ行数の変更は次回接続した時に反映されます。
接続先のRS232CのパラメータはOKの時点で反映されます。

設定ダイアログ中にメニューから設定ファイルの読み書きが可能です。
設定ファイルの関連付けもメニューから行えます。
なお、接続先が指定されていない設定ファイルにオーバーライトした場合は、
ダイアログ中の接続先は設定ファイルには書き込まれません。

■補足事項

・初代Personaでほどほどのスピードを出すため、
  VC5.0+CEToolkitでSH向けにコンパイルした場合の実行速度を
  意識してコードを書いています。ので他の機種向けのは遅いです。
  しかしeMVC3.0のSHコンパイラはろくなコード吐かないですね。
・k12x10のサイズが以前より大きいですがデータ記憶用メモリの消費は同じです。
・添付フォントk12x10のライセンス等詳細は
  http://www.softclub.jp/~zoro/k12x10/を参照してください。
-------------------------------------------------------------------------
24term - 80x24 vt100 terminal emulator for WindowsCE
Copyright (c) 2000-2002 Zoroyoshi, SOFTCLUB, Japan
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
