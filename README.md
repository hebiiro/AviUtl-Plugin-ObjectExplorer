﻿# AviUtl プラグイン - オブジェクトエクスプローラ

エクスプローラを表示してエイリアス、オブジェクト、素材ファイルを使いやすくします。
[最新バージョンをダウンロード](../../releases/latest/)

## 導入方法

以下のファイルとフォルダを AviUtl の Plugins フォルダに入れてください。
* ObjectExplorer.auf
* ObjectExplorer (フォルダ)
* セカンドエクスプローラ.auf (複数のウィンドウが必要な場合)
* セカンドエクスプローラ (フォルダ) (複数のウィンドウが必要な場合)

## 使用方法

### オブジェクトエクスプローラを表示する

1. 「表示」→「オブジェクトエクスプローラの表示」を選択します。
2. オブジェクトエクスプローラが表示されます。
3. オブジェクトエクスプローラは通常のエクスプローラとほぼ同じように使用できます。

### ツリービューの表示/非表示を切り替える

1. 上部のボタンを右クリックして「ツリービューを表示/非表示」を選択します。

### ボイスを有効にする

1. [WavPlayer](../../../WavPlayer/) を導入します。
2. 上部のボタンを右クリックして「声で知らせる」を選択してチェックを付けます。

### ボイスを無効にする

1. 上部のボタンを右クリックして「声で知らせる」を選択してチェックを外します。

### コモンダイアログを使用しない

1. 上部のボタンを右クリックして「コモンダイアログを使用する」を選択してチェックを外します。

### ウィンドウを増やす

1. ObjectExplorer.auf と ObjectExplorer フォルダのコピーを作成する。
2. 識別しやすい任意の名前 (例えばサードエクスプローラなど) にリネームする。

### お気に入りフォルダを登録する

1. 登録したいフォルダに移動します。
2. 上部のボタンを右クリックして「お気に入りに追加」を選択します。

### お気に入りフォルダに移動する

1. 右上のドロップダウンボタンを押します。
2. ドロップダウンボタンリストの中から移動したいフォルダを選択します。

### アイテムに関連したフォルダに移動する

1. タイムラインで動画ファイルなどのアイテムを選択します。
2. オブジェクトエクスプローラで「取得」ボタンを押します。

### 拡張編集ウィンドウでエイリアスファイルを作成する

1. タイムラインでアイテムを右クリックして「エイリアスを作成」を選択します。
2. 拡張編集独自のダイアログではなく通常のファイル選択ダイアログが表示されます。
このとき最初に表示されるフォルダはオブジェクトエクスプローラで現在表示しているフォルダになります。
3. 通常のファイルのようにエイリアスファイルを保存します。

### 拡張編集ウィンドウでエイリアスファイルを読み込む

1. オブジェクトエクスプローラからタイムライン上にエイリアスファイルをドラッグアンドドロップします。
2. ファイルが読み込まれ、タイムライン上にアイテムが生成されます。(拡張編集の通常の動作)

### 設定ダイアログでエイリアスファイルを作成する

1. 設定ダイアログで右クリックして「設定の保存」→「現在の設定でエイリアスを作成する」を選択します。
2. 拡張編集ウィンドウのときと同じ手順でエイリアスファイルを保存します。

### 設定ダイアログでエイリアスファイルを読み込む

1. オブジェクトエクスプローラから設定ダイアログにエイリアスファイルをドラッグアンドドロップします。
2. エイリアスファイルに含まれている複数のフィルタが選択アイテムに付与されます。

## 設定方法

ObjectExplorerSettings.xml をテキストエディタで編集します。

## 更新履歴

* 3.4.0 - 2023/01/07 音を外部プロセスで鳴らすように変更
* 3.3.0 - 2022/08/31 コモンダイアログを使用するか選べるように修正
* 3.2.1 - 2022/08/02 ドキュメントの誤りを修正
* 3.2.0 - 2022/07/28 エクスプローラを複数表示できるように修正
* 3.1.0 - 2022/07/26 ツリービューの表示/非表示を切り替えられるように修正
* 3.0.1 - 2022/07/10 表示設定が永続化するように修正
* 3.0.0 - 2022/06/26 外部プロセスのダイアログを子ウィンドウに変更
* 2.2.0 - 2022/05/30 DarkenWindow の仕様変更に対応
* 2.1.0 - 2022/05/13 DarkenWindow.aul が存在する場合は読み込むように修正
* 2.0.2 - 2022/05/12 特定のオブジェクト名のときエイリアスの保存ができない問題を修正
* 2.0.1 - 2022/05/05 設定ダイアログのドラッグアンドドロップでデフォルト処理が実行されない問題を修正
* 2.0.0 - 2022/05/05 リファクタリング
* 1.0.0 - 2022/04/05 初版

## 動作確認

* (必須) AviUtl 1.10 & 拡張編集 0.92 http://spring-fragrance.mints.ne.jp/aviutl/
* (共存確認) patch.aul r42 https://scrapbox.io/ePi5131/patch.aul

## クレジット

* Microsoft Research Detours Package https://github.com/microsoft/Detours
* aviutl_exedit_sdk https://github.com/ePi5131/aviutl_exedit_sdk
* Common Library https://github.com/hebiiro/Common-Library
* VOICEVOX https://voicevox.hiroshiba.jp/
* VOICEVOX:青山龍星

## 作成者情報
 
* 作成者 - 蛇色 (へびいろ)
* GitHub - https://github.com/hebiiro
* Twitter - https://twitter.com/io_hebiiro

## 免責事項

この作成物および同梱物を使用したことによって生じたすべての障害・損害・不具合等に関しては、私と私の関係者および私の所属するいかなる団体・組織とも、一切の責任を負いません。各自の責任においてご使用ください。
