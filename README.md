# KEEPCHK.X

常駐プログラム存在チェッカー for X680x0/Human68k

C言語で書かれた常駐プログラムが常駐しているかどうかをチェックするツールです。
バッチファイル内での分岐や、自作C言語アプリケーション内に組み込むなどして利用できます。

---

## Install

KEEPCHK.ZIP をダウンロードして展開し、KEEPCHK.X をパスの通ったディレクトリにコピーします。

---

## Usage

    KEEPCHK.X [-d] <eye-catch>

チェック対象となる常駐プログラムの先頭にあるアイキャッチ文字列を `eye-catch` として指定します。最大32文字まで。

    keepchk EX16mxdrv206

常駐していることが確認できた場合は終了コード1を、確認できなかった場合は終了コード0を返します。
終了コードが負の値の場合はエラーです。

バッチファイル内で分岐させるには、例えば以下のように記述します。

    keepchk EX16mxdrv206
    IF NOT ERRORLEVEL 1 GOTO SKIP
    ...
    
    :SKIP

文字列にコントロールコードなどが含まれる場合は、`eye-catch` の先頭に `0x` をつけると、2桁ごとの16進数として解釈します。

    keepchk 0x455831366d78647276323036

これは先の例と同じ意味になります。

オプション `-d` を指定した場合は、ダンプモードとなり、次の引数を実行ファイル名(.X)とみなして、常駐時の先頭部分の32バイトを表示します。

    keepchk -d hogehoge.x

---
