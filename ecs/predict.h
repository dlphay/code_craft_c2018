#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "lib_io.h"



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "math.h"
#include "stdlib.h"
#include "ctime"

// ascii 到数字转换
#define CONV_ASCII_TO_INT  48	

// 噪声增益系数
#define NOISE_W  1.5

// 噪声偏置系数
#define NOISE_B  0.01

#define MAX_SERVER_NUM  300

//定义间隔一星期(7天)为一个样本周期
//#define CYC_SAMPLE  9





// 虚拟机
struct Flavor {
	int flavor_name;
	int cpu_core_num;
	int mem_size;
};

// 输入文件控制
struct Inputcontrol {
	int flavorMaxnum;
	// 年-月-日期描述:
	// 例如: 9月18日   startTime = 5*9 * 31 + 18;

	// 默认是[31(1) , 28(2) , 31(3) , 30(4) , 31(5) , 30(6) , 31(7) , 31(8) , 30(9) , 31(10) , 30(11) , 31(12) ] = 365天！
	int startTime;
	int endTime;
	int intervalTime; // 天数
					  // 输入文件行数
	int inputfileLinenum;
	//cpu or mem
	int cpuOrmem;

};

// 月份
struct Month {

	// 默认是[31(1) , 28(2) , 31(3) , 30(4) , 31(5) , 30(6) , 31(7) , 31(8) , 30(9) , 31(10) , 30(11) , 31(12) ] = 365天！
	int month_name;
	int month_day_num;
	int month_day_total_num;
};



// 训练文件控制
struct Traincontrol {
	// 月-日期描述:
	// 例如: 9月18日   startTime = 9 * 31 + 18;
	int startTime;
	int endTime;

	//样本个数！
	int sample_num;

	//样本以1天为单位抽取！！为去噪做准备
	int sample_num_noise;

	// 训练文件行数
	int trainfileLinenum;

};


// 服务器
struct Server {
	int cpu_core_num;
	int mem_size;    // GB
	int harddisk_size;  //GB
};



// 个体基因信息
struct GeneInfoUnit {
	int gene[16];  // 最大的虚拟机种类
	int period;
	int result_save_gene[16 * 2000]; //用来保存放置方案（样本在已有的放置方案之下的表现） 默认服务器最大是2000个！！！
	int result_serve_num; //需要的服务器数量
	double value_fit;  // 适应度（公式评分）
};

void month_init();
int poww(int power_num);
int poww_two(int power_num);
int round_my(double x);
int input_a_int_num(char *result_file_local, int  result_point, int input_num);


void read_flavor_class_num(char ** const inputfileBuff);
void do_input_file(char ** const inputfileBuff, Flavor *inputFlavor);
void read_time_diff(char ** const trainfileBuff, int train_file_line);

int do_train_file_valid_div(char ** const trainfileBuff, int *valid_div_data);
void do_train_file(char ** const trainfileBuff, Flavor *inputFlavor, int *trainfileFlavordata, int *valid_div_data);
void get_trainfileFlavordata_diff(int *input, int *ouput, int temp);

void do_noise_happyday(int *trainfileFlavordata, int *day_data,int tatal_daynum);
void do_noise_avgfilter(int *trainfileFlavordata);
void do_noise_midfilter(int *trainfileFlavordata, Flavor *inputFlavor);
void print_data(int *trainfileFlavordata, int hangshu);

int *predict_run_RLMS(int *traindata, int num_vm, int num_traindata, int preidct_sample_day_num);
int *predict_run_Wmean(int *traindata, int num_vm, int num_traindata, int preidct_sample_day_num);
int *predict_run_RLS(int *traindata, int num_vm, int num_traindata);
double *predict_get_ori_data(int *traindata, int num_vm, int num_traindata, int preidct_sample_day_num);
double *predict_run_liner(int *traindata, int num_vm, int num_traindata, int preidct_sample_day_num);
int putVM_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor_data, int *result_save);
int putVM_greedy_AAA(int *require_vm_data, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor_data, int *result_save);
int putVM_greedy_without_seq(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save);
void putVM_seq(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save);
void putVM_seq_vmsize(int *require_vm,  Server resource_pm, int opt_target, Flavor *inputFlavor, long *vm_size_data,double *ratio_vm_data, double *ratio_vm_diff_data, int *vm_put_priority,int Flag_put_priority);
int putVM_greedy_AAA_vmsize(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int *vm_put_priority);

void putVM_seq_ratio_guided(int *require_vm, Server resource_pm, int opt_target, Flavor *inputFlavor,  double *ratio_vm_data, int *vm_put_priority, int Flag_put_priority);
int putVM_greedy_ratio_guided(int *require_vm, int server_num_pre, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int *vm_put_priority);
int putVM_dynamicpro_ratio_guided(int *require_vm, int num_of_total_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int *vm_put_priority);


int putVM_liner(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save);
int putVM_dynamicpro(int *require_vm, int num_of_total_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save);


int putVM_correct(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int num_py);
int putVM_correct_vmsize(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int num_py);

double get_score(int *result_predict_real, int *result_predict_temp, Flavor *inputFlavor_temp, int cpuormem, int serve_num);

int putVM_compet_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save);
int putVM_compet_dynamicpro(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save);

int putVM_score_compet_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save);
int putVM_score_compet_dynamicpro(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save);

int gene_dynamicpro(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save);
int gene_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save);

int putVM_score_SAA_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save);
int putVM_directTraversal(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save);

void print_resource(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int num_py);

double *normal_result(double *data, double *data_a, int data_num);
void mat_mult(double *mult_out, double *mat1, int mat1_height, int mat1_width, double *mat2, int mat2_height, int mat2_width);
void write_output_to_result(char *result_file_local, int *vm_predict, int vm_require_num, int *result_py_save, int py_require_num, Flavor *inputFlavor, int vm_class_num);



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename);






#endif
