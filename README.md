# 數值控制相關專案

## macro_expression: 巨集算式解析及執行

檔案說明

MacroVariable.h/cpp :

定義巨集變數，包含局部變數(#1-#33)、共用變數(#100-#999)以及系統變數(#3000以上)

MacroOperator.h/cpp :

定義巨集運算子，包含算術運算子、關係運算子、邏輯運算子、條件式運算子等幾大類

MacroParser.h/cpp :

巨集單節剖析器，解析巨集字串並產生巨集運算子語法樹，供用戶碼執行巨集運算

ControllerParameter.h/cpp : 控制器參數

CoordinateSystem.h/cpp : 坐標系定義

NC_NumberDefinition.h/cpp : 整數、浮點數之值域範圍定義

StringConverter.h/cpp : 字串與整數、浮點數之雙向轉換器

## UnitTest: 對應專案的單元測試

使用VS內建的MS Test框架進行程式碼測試，以namespace分為主要兩大類以及底下的運算子分組測試

namespace MacroOperators: 直接建立各種運算子並進行核算測試

namespace MacroExpressions: 對巨集語法單節字串進行解析、產生巨集運算子構成的巨集運算式，並核算其結果是否正確