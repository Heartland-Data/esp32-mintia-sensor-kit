# 公開前セルフチェックレポート

**実施日**: 2026-05-19  
**対象ディレクトリ**: `SRC/HLDC/XM125_IR_MINTIA`  
**チェック実施者**: GitHub Copilot による自動チェック

---

## チェック結果サマリー

| フェーズ           | ツール               | 結果 | 要対応    |
| ------------------ | -------------------- | ---- | --------- |
| 1 シークレット     | Gitleaks v8.30.1     | ✅   | なし      |
| 2 社内ネットワーク | semgrep v1.162.0     | ✅   | なし      |
| 3 ライセンス       | REUSE lint v6.2.0    | ⚠️   | あり      |
| 4 個人情報 (PII)   | Presidio (全件誤検出) | ✅   | なし      |
| 5 未発表製品情報   | grep + 目視          | ⚠️   | 要確認    |

---

## 実施手順（再現手順）

### フェーズ 0: セットアップ

```powershell
cd C:\PROJ\radar\00-git\RadarSensor\SRC\HLDC\XM125_IR_MINTIA

# ツールバージョン確認
gitleaks version
# → 8.30.1

.\.venv\Scripts\semgrep --version
# → 1.162.0

.\.venv\Scripts\reuse --version
# → reuse, version 6.2.0
```

`.venv/`、`gitleaks-report.json`、`semgrep.yml`、`pii_check_report.txt` はすべて `.gitignore` 済みで確認。

---

### フェーズ 1: シークレットチェック

```powershell
cd C:\PROJ\radar\00-git\RadarSensor\SRC\HLDC\XM125_IR_MINTIA
gitleaks detect --source . --report-format json --report-path .\gitleaks-report.json

# 出力要点:
# 2:03PM INF Unknown SCM platform. host=192.168.10.16   ← .git/config の remote URL (ファインディングではない)
# 2:03PM INF 203 commits scanned.
# 2:03PM INF scanned ~14432646 bytes (14.43 MB) in 5.28s
# 2:03PM INF no leaks found
# Exit code: 0
```

**結果**: 0件。`host=192.168.10.16` は `.git/config` の remote URL から読まれた値であり、JSON レポートは空（ファインディングなし）。

---

### フェーズ 2: 社内ネットワーク情報チェック

```powershell
Copy-Item "C:\Users\hldc0066\.agents\skills\pre-publish-selfcheck\scripts\semgrep.yml" ".\semgrep.yml" -Force
$env:PYTHONUTF8 = "1"
.\.venv\Scripts\semgrep --config .\semgrep.yml . --exclude semgrep.yml

# 出力要点:
# Scanning 71 files (only git-tracked) with 14 Code rules:
# Findings: 0 (0 blocking)
# Rules run: 14
# Targets scanned: 71
# Ran 14 rules on 71 files: 0 findings.
# Exit code: 0
```

**結果**: 14ルール × 71ファイル、検出0件。

---

### フェーズ 3: ライセンスチェック

```powershell
$env:PYTHONUTF8 = "1"
$output = .\.venv\Scripts\reuse lint 2>&1
$output | Where-Object { $_ -match "XM125_IR_MINTIA|^#|MISSING|compliant|Summary" }

# 出力要点:
# # MISSING LICENSES: 'Apache-2.0' found in 90+ files, 'MIT' found in 4 files
# # MISSING COPYRIGHT AND LICENSING INFORMATION: 4 files
# Exit code: 1
```

**MISSING LICENSES の原因（構造的問題）:**  
REUSE は `.git` ディレクトリを辿ってプロジェクトルートを `RadarSensor/` と判断する。
`LICENSES/Apache-2.0.txt` および `LICENSES/MIT.txt` は `XM125_IR_MINTIA/LICENSES/` に存在するが、
REUSE は `RadarSensor/LICENSES/` を探すため「未発見」と報告する。

なお、各ソースファイル（`.c`, `.h`, `.cpp`, `.hpp`）にはすべてインライン SPDX ヘッダーが付与済み。

**MISSING COPYRIGHT AND LICENSING INFORMATION（4ファイル）:**  
以下のファイルに SPDX 情報がなく、REUSE.toml にも含まれていない:

| ファイル                           | 対応状況             |
| ---------------------------------- | -------------------- |
| `.devcontainer/devcontainer.json`  | SPDX なし / 未登録   |
| `.devcontainer/Dockerfile`         | SPDX なし / 未登録   |
| `README.md`                        | SPDX なし / 未登録   |
| `NOTICE`                           | SPDX なし / 未登録   |

---

### フェーズ 4: 個人情報チェック

```powershell
$env:PYTHONUTF8 = "1"
$skillScripts = "C:\Users\hldc0066\.agents\skills\pre-publish-selfcheck\scripts"
& "$skillScripts\run_pii_check.ps1"

# 出力要点:
# スキャン完了: 97ファイル処理, 11587ファイルスキップ
# 検出された問題: 185件 (2ファイル)
# - マイナンバー (JP_MY_NUMBER): 85件
# - 電話番号 (JP_PHONE_NUMBER): 85件
# - 氏名 (JP_PERSON_NAME): 15件
# Exit code: 0
# sdkconfig: サイズ超過エラー (80723 bytes > 49149 bytes上限) → .gitignore 済みにつき問題なし
```

**検出ファイルと誤検出判定:**

| ファイル                                         | 検出内容                              | 判定         |
| ------------------------------------------------ | ------------------------------------- | ------------ |
| `main/src/service/bt_communication_service.cpp`  | 行192: `0000 0000 0000 0000`（BT PIN）| 誤検出 ✅    |
| `main/src/service/bt_communication_service.cpp`  | 行66: `期化処理 開始`（漢字2文字）    | 誤検出 ✅    |
| `pii_check_report.txt`                           | レポート自己検出                      | 誤検出 ✅（.gitignore済み） |

**結論**: 実際の PII（個人情報）の検出はゼロ。

---

### フェーズ 5: 未発表製品情報チェック

#### 5-1. TODO/FIXME スキャン

```powershell
Get-ChildItem -Recurse -Include "*.c","*.cpp","*.h","*.hpp","*.md","*.py" |
  Select-String -Pattern "(TODO|FIXME|HACK|XXX|NOTE):" |
  Where-Object { $_.Path -notmatch "\\build\\" -and $_.Path -notmatch "\\\.venv\\" }

# 検出結果:
# data_parser_service.cpp:173: // TODO: 現状は無条件で成功を送付する
# data_parser_service.cpp:182: // TODO: 現状は無条件で成功を送付する
# task_sensor.cpp:122:         // TODO: サービス側にパラメータ設定処理を追加予定
# mlx90640_i2c_driver.c:199:  // Note: In ESP32-IDF, changing frequency at runtime requires reconfiguration (サードパーティ)
# i2c_master.h:119:            Note: This is not strong enough to pullup buses...  (ESP-IDF ヘッダー、サードパーティ)
```

#### 5-2. カスタムキーワードスキャン

`.pr-keywords-blacklist.txt` が存在しないためスキップ。  
プロジェクト固有の社外秘キーワードがある場合は手動で作成・実行すること。

#### 5-3. 目視確認結果

| #   | 確認対象                         | 結果                                                                 |
| --- | -------------------------------- | -------------------------------------------------------------------- |
| A   | 製品名・基板名                   | `MINTIA`、`esp32-mintia-sensor-kit` はQiita公開記事で既出。問題なし  |
| B   | 社内ライブラリ・コンポーネント名 | `dt_plus` は自社コンポーネントだが外部秘の情報なし（要最終確認）     |
| C   | ブランチ名・コミットメッセージ   | ⚠️ ブランチ名 `chore/mintia-fw-prepublish-cleanup-59279` に内部チケット番号 `59279` の可能性あり |
| D   | テスト用マクロ・スクリプト       | `test/teraterm_macro/*.ttl` は汎用マクロ。接続先情報なし。問題なし   |
| E   | 画像・スクリーンショット         | `docs/images/` の3枚は実物写真。未発表UIなし。問題なし               |

---

## チェック時に生成されたファイル

| ファイル               | 内容                      | .gitignore 済み |
| ---------------------- | ------------------------- | --------------- |
| `gitleaks-report.json` | Gitleaks スキャン結果     | ✅              |
| `semgrep.yml`          | semgrep カスタムルール    | ✅              |
| `pii_check_report.txt` | Presidio PII スキャン結果 | ✅              |
| `.venv/`               | Python 仮想環境           | ✅              |

---

## 詳細結果

### フェーズ 3 詳細: REUSE 構造的問題

**原因**: REUSE ツールは `.git` を起点にプロジェクトルートを `RadarSensor/` と判定するが、
`LICENSES/` ディレクトリは `XM125_IR_MINTIA/LICENSES/` に配置されているため発見できない。

**補足事項**:  
- 全ソースファイルにはインライン SPDX ヘッダー（`SPDX-FileCopyrightText` / `SPDX-License-Identifier`）が付与済み
- `LICENSES/Apache-2.0.txt` および `LICENSES/MIT.txt` の実体ファイルは存在する
- `NOTICE` ファイルにサードパーティコンポーネントの帰属情報が記載済み

### フェーズ 5 詳細: TODO コメント

| ファイル                      | 行  | コメント                                   | 分類           |
| ----------------------------- | --- | ------------------------------------------ | -------------- |
| `main/src/service/data_parser_service.cpp` | 173 | `TODO: 現状は無条件で成功を送付する`  | 自社コード     |
| `main/src/service/data_parser_service.cpp` | 182 | `TODO: 現状は無条件で成功を送付する`  | 自社コード     |
| `main/src/task/task_sensor.cpp`            | 122 | `TODO: サービス側にパラメータ設定処理を追加予定` | 自社コード |
| `components/mlx90640/mlx90640_i2c_driver.c` | 199 | `Note: In ESP32-IDF, changing frequency...` | サードパーティ（無害）|

---

## 推奨対策

| 優先度 | 対象                                       | 対処内容                                                                                     |
| ------ | ------------------------------------------ | -------------------------------------------------------------------------------------------- |
| 高     | REUSE 構造的問題                           | `LICENSES/` を git ルート (`RadarSensor/LICENSES/`) にコピーする、または公開ブランチ作成時に別リポジトリとして切り出す |
| 中     | `.devcontainer/devcontainer.json`・`Dockerfile` | REUSE.toml に `[[annotations]]` エントリを追加して著作権情報を付与する              |
| 中     | `README.md`・`NOTICE`                      | 同上、または各ファイル先頭にコメントで SPDX ヘッダーを追加する                              |
| 低     | TODO コメント 3件                          | `data_parser_service.cpp` と `task_sensor.cpp` の未実装項目を公開前に残す場合はコメントを整理する |
| 低     | ブランチ名の内部チケット番号               | 公開時はブランチ名に内部番号を含まないよう運用ルールを検討（コードには影響なし）            |
| 低     | `.pr-keywords-blacklist.txt`               | プロジェクト固有の社外秘キーワードがある場合は作成して再スキャンすること                    |
