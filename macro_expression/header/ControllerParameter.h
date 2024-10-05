#pragma once

#include <float.h>
#include "CoordinateSystem.h"

class ModalParameter {
public:
	ModalParameter();
	~ModalParameter() {}
	//模式移動指令
	unsigned short motion_command;
	//工作平面(G17/G18/G19)
	unsigned short working_plane;
	//絕對值或增量值模式(G90/G91)
	unsigned short coordinate_value_type;
	//每分或每轉進給(G94/G95)
	unsigned short feed_rate_type;
	//公制或英制單位(G21/20)
	unsigned short system_unit;
	//刀具半徑補正模式
	unsigned short tool_radius_compensation;
	//刀具長度補正模式
	unsigned short tool_length_compensation;   
	//孔加工循環模式(標準G80-G89/特殊G73,G74,G76)
	unsigned short canned_cycle_mode;
	//孔加工循環退刀平面(G98/G99)
	unsigned short canned_cycle_retract_plane;
	//比例模式(G50/G51)
	unsigned short scale_mode;
	//巨集模式(G65/G66/G67)
	unsigned short macro_mode;
	//主軸轉速模式(G96/G97)
	unsigned short spindle_speed_mode;
	//工作座標系(G54-G59)
	unsigned short working_coordinate_system;
	//轉角過渡模式(G61-G64)
	unsigned short corner_mode;                
	//座標系旋轉(G68/G69)
	unsigned short coordinate_system_rotation; 
	//B指令碼
	int B_code;
	//刀具半徑補正號碼
	int D_code;
	//E指令碼
	int E_code;
	//進給率
	int F_code;
	//刀具長度補正號碼
	int H_code;
	//輔助指令
	int M_code;
	//程式序號
	int sequence_number;
	//程式號碼
	int program_number;
	//主軸轉數
	int S_code;
	//刀具號碼
	int T_code;
	//額外工作座標系號碼
	int P_code;
};

enum CycleMode {
	//循環啟動
	cycle_start,
	//循環停止
	cycle_stop,
	//循環結束
	cycle_end };

enum OperationMode {
	//記憶體模式
	memory_mode,
	//紙帶(DNC)模式
	tape_mode };

class OperationParameter {
public:
	OperationParameter();
	~OperationParameter() {}
	//模擬啟動flag
	bool simulation_on;
	//選擇性跳過程式單節
	bool optional_skip;
	//選擇性停止
	bool optional_stop;
	//是否單節停止
	bool single_block_stop;
	//是否自動重新執行
	bool auto_restart;
	//目前控制器狀態
	CycleMode cycle_mode;
	//目前程式執行模式
	OperationMode operation_mode;
};

//刀長補正號碼容量
constexpr unsigned short TOOL_LENGTH_REGISTER_MAX = 81;

class SystemParameter {
public:
	SystemParameter();
	~SystemParameter() {}
	//單節停止無效
	bool SuppressSingleBlockStop() const {
		return suppress_single_block_stop_wait_auxiliary_function & 1; }
	void UpdatePreviewProgramPosition(Coordinate& position) {
		preview_program_position = position; }
	//最近一個指令為移動指令
	bool last_command_motion;
	//序號固定快取
	bool sequence_cache_fixed;
	//序號變動快取
	bool sequence_cache_variable;
	//序號搜尋記錄快取
	bool sequence_cache_history;
	//副程式指定起始序號
	bool sub_program_begin_sequence;
	//副程式號碼採用八位數格式
	bool sub_program_number_P8;
	//最近一次復歸之參考點號碼
	char last_reference_position;
	//單節停止無效以及是否等候輔助指令完成
	unsigned short suppress_single_block_stop_wait_auxiliary_function;
	//孔加工循環搪刀偏移方向
	int boring_shift_direction;
	//程式圓弧半徑
	double program_radius;
	//X軸快速進給率
	double rapid_feed_rate_X;
	//Y軸快速進給率
	double rapid_feed_rate_Y;
	//Z軸快速進給率
	double rapid_feed_rate_Z;
	//啄鑽間隙距離
	double peck_drilling_clearance;
	//啄鑽退刀距離
	double peck_drilling_retraction;
	//刀具長度補正表格
	std::map<unsigned short, double> tool_length_offset_table;
	//程式中繼點座標
	Coordinate intermediate_position;
	//主要參考原點位置
	Coordinate reference_position_1st;
	//第二參考點位置
	Coordinate reference_position_2nd;
	//第三參考點位置
	Coordinate reference_position_3rd;
	//第四參考點位置
	Coordinate reference_position_4th;
	//預讀程式終點座標
	Coordinate preview_program_position;
	//目前模式參數
	ModalParameter current_modal_parameter;
	//預讀模式參數
	ModalParameter preview_modal_parameter;
	//加工操作參數
	OperationParameter operation_parameter;
	//機械座標系
	Coordinate machine_coordinate;
	//工作座標系(G54-G59)
	WorkingCoordinateSystem working_coordinate_system;
	//程式座標系
	ProgramCoordinateSystem program_coordinate_system;
};
