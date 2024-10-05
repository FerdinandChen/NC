# 數值控制相關專案

## macro_expression: 巨集算式解析及執行

檔案說明

MacroVariable.h/cpp :

定義巨集變數，包含局部變數(#1-#33)、共用變數(#100-#999)以及系統變數(#3000以上)

MacroOperator.h/cpp :

定義巨集運算子，包含算術運算子、關係運算子、邏輯運算子、條件式運算子等幾大類

MacroParser.h/cpp :

巨集單節剖析器，解析巨集字串並產生巨集運算子語法樹，供用戶碼執行巨集運算

ControllerParameter.h/cpp :

控制器參數

CoordinateSystem.h/cpp :

坐標系定義

NC_NumberDefinition.h/cpp :

整數、浮點數之值域範圍定義

StringConverter.h/cpp :

字串與整數、浮點數之雙向轉換器

## UnitTest: 對應專案的單元測試

