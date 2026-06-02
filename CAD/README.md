# 概要
本リポジトリは、Acconeer社製 60GHz Radar Unit "XM125" 及び "IR Sensor Array",
Espressif社 ESP32 Module を 清涼菓子 "MINTIA"ケースに収まる仕立ての回路及び基板データです。

## ハードウェア構成
* [Espressif ESP32](https://www.espressif.com/ja-jp/products/modules)
* [Acconeer](https://acconeer.com/products/) XM125 (60GHz Radar)
* IR Sensor Array
    * panasonic社 [AMG88](https://industrial.panasonic.com/jp/products/pt/grid-eye/models/AMG8833)
    * Melexis社 [MLX90640](https://www.melexis.com/en/product/MLX90640/far-infrared-thermal-sensor-array)



# ディレクトリ構造
[KiCAD](https://www.kicad.org/) 用 Data 及び PCB作成用ガーバーデータ一式

```
|   README.md
|   IR.kicad_sch
|   Main.kicad_sch
|   Power.kicad_sch
|   Radar.kicad_sch
|   Serial.kicad_sch
|   Work.kicad_pcb
|   Work.kicad_prl
|   Work.kicad_pro
|   Work.kicad_sch
|
\---GB
        PZ-H036Z-R1-B_Cu.gbl
        PZ-H036Z-R1-B_Mask.gbs
        PZ-H036Z-R1-B_Paste.gbp
        PZ-H036Z-R1-B_Silkscreen.gbo
        PZ-H036Z-R1-drl.rpt
        PZ-H036Z-R1-drl_map.gbr
        PZ-H036Z-R1-Edge_Cuts.gm1
        PZ-H036Z-R1-F_Cu.gtl
        PZ-H036Z-R1-F_Mask.gts
        PZ-H036Z-R1-F_Paste.gtp
        PZ-H036Z-R1-F_Silkscreen.gto
        PZ-H036Z-R1.drl
```



# ライセンス
MIT License

Copyright (c) 2026 Heartland Data Co., Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

