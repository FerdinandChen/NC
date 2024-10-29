# 數值控制相關專案

## macro_expression: 巨集算式解析及執行

此專案僅能夠解析並執行純巨集算式，不包含NC程式碼或巨集混合NC程式碼

檔案說明

MacroVariable.h/cpp :

定義巨集變數，包含局部變數(#1-#33)、共用變數(#100-#999)以及系統變數(#3000以上)

MacroOperator.h/cpp :

定義巨集運算子，包含算術運算子、關係運算子、邏輯運算子、條件式運算子等幾大類

MacroParserFactory.h/cpp :

運用Factory Method pattern的設計，來建立針對不同品牌控制器的巨集語言解譯器，目前僅支援Fanuc巨集語言。方便將來擴展支援其他控制器如Siemens、Heidenhain等等

FanucMacroParser.h/cpp :

支援Fanuc巨集語言的單節剖析器，解析巨集字串並產生巨集運算子的語法樹，供用戶碼執行巨集運算

ControllerParameter.h/cpp : 控制器參數

CoordinateSystem.h/cpp : 座標系定義

NC_NumberDefinition.h/cpp : 整數、浮點數之值域範圍定義

StringConverter.h/cpp : 字串與整數、浮點數之雙向轉換器

## UnitTest: 對應專案的單元測試

使用Visual Studio內建的MS Test框架進行程式碼測試，以namespace分為主要兩大類以及底下的運算子分組測試

namespace MacroOperators: 直接建立各種運算子並進行核算測試

namespace MacroExpressions: 對巨集語法單節字串進行解析、產生巨集運算子構成的巨集運算式，並核算其結果是否正確
