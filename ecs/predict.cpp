#include "predict.h"
#include <stdio.h>













///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/******************************
全部变量、函数定义部分
*******************************/


Inputcontrol inputcontrol;
Traincontrol traincontrol;
Server inputServer;

// 周期！！！
int CYC_SAMPLE = 9;

// 噪声去除的周期 默认为1
int CYC_SAMPLE_NOISE = 1;

// 默认是[31(1) , 28(2) , 31(3) , 30(4) , 31(5) , 30(6) , 31(7) , 31(8) , 30(9) , 31(10) , 30(11) , 31(12) ] = 365天！
Month month[13];

int month_data[13] = { 0,31 , 28 , 31 , 30 , 31 , 30 , 31, 31 , 30 , 31 , 30 , 31 };

int HAPPYDAY_TOTALNUM = 4;
// 定义节假日  元旦1.1 1.2 1.3  五一5.1 5.2 5.3 十一10.1 10.2 10.3  双11 11.11
int happyday[4] = { 1,   121,   274,   315 };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 月份的初始化
void month_init()
{
	month[0].month_name = 0;
	month[0].month_day_num = 0;
	month[0].month_day_total_num = 0;


	for (int i = 1; i< 13; i++)
	{
		month[i].month_name = i;
		month[i].month_day_num = month_data[i];
		month[i].month_day_total_num = 0;
		for (int j = 1; j <= i; j++)   month[i].month_day_total_num += month_data[j - 1];

	}

}

// 10的power_num次幂
int poww(int power_num) {
	int power_out = 1;
	for (int i = 0; i < power_num; i++) {
		power_out = power_out * 10;
	}
	return power_out;
}

// 2的power_num次幂
int poww_two(int power_num) {
	int power_out = 1;
	for (int i = 0; i < power_num; i++) {
		power_out = power_out * 2;
	}
	return power_out;
}

/*函数功能:double类型四舍五入输出int类型*/
int round_my(double x)
{
	int x_out;
	if (x > 0)
		x_out = (int)(x + 0.5);
	else
		if (x < 0)
			x_out = (int)(x - 0.5);
	if (x == 0)  x_out = 0;

	return x_out;
}

/*
int round_my(double x) {
int x_int, x_out;
double x_zheng;
x_int = (int)x;
x_zheng = (double)x_int;
if (x - x_zheng < 0.5) {
x_out = x_int;
}
else {
x_out = x_int + 1;
}
return x_out;
}
*/


/*
往字符数组写入一个int数字
result_file_local:被写入数组
result_point:从数组的第几位开始输入
input_num:输入数字
函数返回为输入数字后result_point的位置
*/
int input_a_int_num(char *result_file_local, int  result_point, int input_num) {
	int weishu = 0; //整形数据的位数
	int num_readytowrite = input_num; //copy一个input_num
	while (num_readytowrite != 0) {
		num_readytowrite /= 10;
		weishu++;
	}
	if (input_num == 0) { //0也是1位数
		weishu = 1;
	}
	int weishu_copy = weishu;//用以递减次幂
	for (int i = result_point; i < result_point + weishu; i++) {
		result_file_local[i] = '0' + input_num / poww(weishu_copy - 1);
		input_num = input_num - (input_num / poww(weishu_copy - 1)) * poww(weishu_copy - 1);
		weishu_copy--;
	}
	result_point += weishu;
	return result_point;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 读入虚拟机种类数量
void read_flavor_class_num(char ** const inputfileBuff)
{
	//第三行的处理
	///!!!!!!!!!!!!!!!
	if (inputfileBuff[2][1] < '0' || inputfileBuff[2][1] > '9')
		inputcontrol.flavorMaxnum = ((inputfileBuff[2][0] - CONV_ASCII_TO_INT));
	else
		inputcontrol.flavorMaxnum = ((inputfileBuff[2][0] - CONV_ASCII_TO_INT) * 10 + (inputfileBuff[2][1] - CONV_ASCII_TO_INT));
	// inputcontrol.flavorMaxnum = 5;
	printf("inputcontrol.flavorMaxnum : %d \n", inputcontrol.flavorMaxnum);
}


// 输入文件的处理
void do_input_file(char ** const inputfileBuff, Flavor *inputFlavor)
{
	//读硬件服务器资源
	int kong1 = 0;  //空格1
	int kong2 = 0;  //空格2
	int kong3 = 0;  //末尾
	for (int i = 0; i < 12; i++)
	{
		if (inputfileBuff[0][i] == ' ' && kong1 != 0)  kong2 = i;
		if (inputfileBuff[0][i] == ' ' && kong2 == 0)  kong1 = i;
		if (inputfileBuff[0][i] == '\n') {
			kong3 = i;
			break;
		}
	}
	if (kong1 == 1)
	{
		inputServer.cpu_core_num = (inputfileBuff[0][0] - CONV_ASCII_TO_INT);
	}
	else if (kong1 == 2)
	{
		inputServer.cpu_core_num = (inputfileBuff[0][0] - CONV_ASCII_TO_INT) * 10 + (inputfileBuff[0][1] - CONV_ASCII_TO_INT);
	}
	inputServer.mem_size = 0;
	for (int i = kong2 - 1; i>kong1; i--)
	{
		inputServer.mem_size += (inputfileBuff[0][i] - CONV_ASCII_TO_INT)*poww((kong2 - 1) - i);

	}
	inputServer.harddisk_size = 0;
	for (int i = kong3 - 1; i>kong2; i--)
	{
		inputServer.harddisk_size += (inputfileBuff[0][i] - CONV_ASCII_TO_INT)*poww((kong3 - 1) - i);
	}



	//读取各个虚拟机名称与性能
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
		kong1 = 0;  //空格1
		kong2 = 0;  //空格2
		kong3 = 0;  //末尾
		for (int j = 0; j < 19; j++) {
			if (inputfileBuff[3 + i][j] == ' ' && kong1 != 0)  kong2 = j;
			if (inputfileBuff[3 + i][j] == ' ' && kong2 == 0)  kong1 = j;
			if (inputfileBuff[3 + i][j] == '\n') {
				kong3 = j - 1;
				break;
			}
		}
		if (kong1 == 7) {
			inputFlavor[i].flavor_name = (inputfileBuff[3 + i][6] - CONV_ASCII_TO_INT);
		}
		else if (kong1 == 8) {
			inputFlavor[i].flavor_name = (inputfileBuff[3 + i][6] - CONV_ASCII_TO_INT) * 10 + (inputfileBuff[3 + i][7] - CONV_ASCII_TO_INT);
		}

		inputFlavor[i].cpu_core_num = 0;
		for (int j = kong2 - 1; j>kong1; j--) {
			inputFlavor[i].cpu_core_num += (inputfileBuff[3 + i][j] - CONV_ASCII_TO_INT)*poww((kong2 - 1) - j);
		}

		inputFlavor[i].mem_size = 0;
		long long mem_size_temp = 0;
		for (int j = kong3 - 1; j>kong2; j--) {
			mem_size_temp += (inputfileBuff[3 + i][j] - CONV_ASCII_TO_INT)*poww((kong3 - 1) - j);
		}
		// 从 MB 转换成为 GB



		//printf("kong1 :%d \n", kong1);
		//printf("kong2 :%d \n", kong2);
		//printf("kong3 :%d \n", kong3);

		//printf("mem_size_temp :%d \n", mem_size_temp);
		mem_size_temp = mem_size_temp >> 10;
		inputFlavor[i].mem_size = (int)mem_size_temp;
		//printf(" %d", inputFlavor[i].mem_size);

	}

	//读取要求预测的时间段

	if (inputfileBuff[inputcontrol.inputfileLinenum - 1][4] == 45 && inputfileBuff[inputcontrol.inputfileLinenum - 1][7] == 45)
	{
		inputcontrol.endTime = ((inputfileBuff[inputcontrol.inputfileLinenum - 1][2] - CONV_ASCII_TO_INT) * 10 + (inputfileBuff[inputcontrol.inputfileLinenum - 1][3] - CONV_ASCII_TO_INT)) * 365 +
			month[(inputfileBuff[inputcontrol.inputfileLinenum - 1][5] - CONV_ASCII_TO_INT) * 10 + (inputfileBuff[inputcontrol.inputfileLinenum - 1][6] - CONV_ASCII_TO_INT)].month_day_total_num +
			(inputfileBuff[inputcontrol.inputfileLinenum - 1][8] - CONV_ASCII_TO_INT) * 10 + (inputfileBuff[inputcontrol.inputfileLinenum - 1][9] - CONV_ASCII_TO_INT);
	}
	if (inputfileBuff[inputcontrol.inputfileLinenum - 2][4] == 45 && inputfileBuff[inputcontrol.inputfileLinenum - 2][7] == 45)
	{
		inputcontrol.startTime = ((inputfileBuff[inputcontrol.inputfileLinenum - 2][2] - CONV_ASCII_TO_INT) * 10 + (inputfileBuff[inputcontrol.inputfileLinenum - 2][3] - CONV_ASCII_TO_INT)) * 365 +
			month[(inputfileBuff[inputcontrol.inputfileLinenum - 2][5] - CONV_ASCII_TO_INT) * 10 + (inputfileBuff[inputcontrol.inputfileLinenum - 2][6] - CONV_ASCII_TO_INT)].month_day_total_num +
			(inputfileBuff[inputcontrol.inputfileLinenum - 2][8] - CONV_ASCII_TO_INT) * 10 + (inputfileBuff[inputcontrol.inputfileLinenum - 2][9] - CONV_ASCII_TO_INT);
	}
	printf("inputcontrol.startTime : %d   ", inputcontrol.startTime, inputcontrol.startTime / 365);
	printf("inputcontrol.endTime : %d   \n", inputcontrol.endTime);

	inputcontrol.intervalTime = inputcontrol.endTime - inputcontrol.startTime + 1;


	// 读取优化资源目标:cpu or mem ,cpu为0,memory为1
	if (inputfileBuff[inputcontrol.inputfileLinenum - 4][0] == 'C')  inputcontrol.cpuOrmem = 0;
	if (inputfileBuff[inputcontrol.inputfileLinenum - 4][0] == 'M')  inputcontrol.cpuOrmem = 1;

	printf("inputServer : %d   %d   %d\n", inputServer.cpu_core_num, inputServer.mem_size, inputServer.harddisk_size);

	for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
	{
		printf("inputFlavor%d : %d   %d   %d\n", i, inputFlavor[i].flavor_name, inputFlavor[i].cpu_core_num, inputFlavor[i].mem_size);
	}




	//printf("inputcontrol.inputfileLinenum :%d \n", inputcontrol.inputfileLinenum);

	//printf("xxxx :%d \n", (int)(' '));
	//printf("xxxx :%d \n", (int)('\n'));

	//printf("xxxx :%d \n", (int)('\n'));
}




void read_time_diff(char ** const trainfileBuff, int train_file_line)
{
	for (int j = 0; j < 50; j++)
		if ((trainfileBuff[0][j] == 45) && (trainfileBuff[0][j + 3] == 45) && (trainfileBuff[0][j + 9] == 58))
		{
			traincontrol.startTime = ((trainfileBuff[0][j - 2] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[0][j - 1] - CONV_ASCII_TO_INT)) * 365 +
				month[(trainfileBuff[0][j + 1] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[0][j + 2] - CONV_ASCII_TO_INT)].month_day_total_num +
				((trainfileBuff[0][j + 4] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[0][j + 5] - CONV_ASCII_TO_INT));
			break;
		}
	for (int j = 0; j < 50; j++)
		if ((trainfileBuff[train_file_line - 1][j] == 45) && (trainfileBuff[train_file_line - 1][j + 3] == 45) && (trainfileBuff[train_file_line - 1][j + 9] == 58))
		{
			traincontrol.endTime = ((trainfileBuff[train_file_line - 1][j - 2] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[train_file_line - 1][j - 1] - CONV_ASCII_TO_INT)) * 365 +
				month[(trainfileBuff[train_file_line - 1][j + 1] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[train_file_line - 1][j + 2] - CONV_ASCII_TO_INT)].month_day_total_num +
				(trainfileBuff[train_file_line - 1][j + 4] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[train_file_line - 1][j + 5] - CONV_ASCII_TO_INT);
			break;
		}
	// cout << traincontrol.startTime << endl;
	// cout << traincontrol.endTime << endl;
	printf("traincontrol.startTime: %d   ", traincontrol.startTime);
	printf("traincontrol.endTime: %d \n", traincontrol.endTime);
}

// 对训练文件数据进行有效的分割！！！ 时间段如下：（13，19]
int do_train_file_valid_div(char ** const trainfileBuff, int *valid_div_data)
{
	int sample_num_count = 0;
	valid_div_data[0] = traincontrol.endTime;
	int temptime = traincontrol.endTime;
	int temptime_count = 0;
	for (int i = (traincontrol.trainfileLinenum - 2); i >= 0; i--)  //正在处理第i行！！！
	{
		// 寻找时间段
		for (int j = 0, flag_shijianduan = 0; flag_shijianduan == 0; j++)
		{
			// 先找见时间并确认时间  "  - :  - "
			if ((trainfileBuff[i][j] == 45) && (trainfileBuff[i][j + 3] == 45) && (trainfileBuff[i][j + 9] == 58))
			{
				flag_shijianduan = 1;  // 找见了时间段标志位
				if ((((trainfileBuff[i][j - 2] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j - 1] - CONV_ASCII_TO_INT)) * 365 +
					month[(trainfileBuff[i][j + 1] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j + 2] - CONV_ASCII_TO_INT)].month_day_total_num +
					(trainfileBuff[i][j + 4] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j + 5] - CONV_ASCII_TO_INT)) != temptime)
				{   // 与当前的时间不一样，证明这一行是新的时间！！！进行赋值，并算作一次。
					temptime = (((trainfileBuff[i][j - 2] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j - 1] - CONV_ASCII_TO_INT)) * 365 +
						month[(trainfileBuff[i][j + 1] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j + 2] - CONV_ASCII_TO_INT)].month_day_total_num +
						(trainfileBuff[i][j + 4] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j + 5] - CONV_ASCII_TO_INT));
					temptime_count++;
				}
				if (temptime_count == CYC_SAMPLE_NOISE)
				{
					temptime_count = 0;
					sample_num_count++;
					valid_div_data[sample_num_count] = temptime;
				}


			}

		}

	}


	return sample_num_count;
}


void do_train_file(char ** const trainfileBuff, Flavor *inputFlavor, int *trainfileFlavordata, int *valid_div_data)
{

	// 计算间隔
	//  = inputcontrol.startTime - CYC_SAMPLE;

	// 当前时间
	int temptime = 0;


	// 分割时间段
	for (int count_div = 0; count_div < traincontrol.sample_num_noise;)
	{
		//int a = 0;
		for (int i = (traincontrol.trainfileLinenum - 1); i >= 0; i--)
		{
			if (count_div >= traincontrol.sample_num_noise) break;
			//printf("\n第%d行在处理...", i);
			// 寻找时间段
			for (int j = 0, flag_shijianduan = 0; flag_shijianduan == 0; j++)
			{
				if (count_div >= traincontrol.sample_num_noise) break;
				// 先找见时间并确认时间  "  - :  - "
				if ((trainfileBuff[i][j] == 45) && (trainfileBuff[i][j + 3] == 45) && (trainfileBuff[i][j + 9] == 58))
				{
					flag_shijianduan = 1;  // 找见了时间段标志位
					if (count_div >= traincontrol.sample_num_noise) break;

					temptime = (((trainfileBuff[i][j - 2] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j - 1] - CONV_ASCII_TO_INT)) * 365 +
						month[(trainfileBuff[i][j + 1] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j + 2] - CONV_ASCII_TO_INT)].month_day_total_num +
						(trainfileBuff[i][j + 4] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j + 5] - CONV_ASCII_TO_INT));
					// 左边界 与 右边界 约束
					// [13,20) : 13 14 15 16 17 18 19
					if (temptime > valid_div_data[count_div + 1] && temptime <= valid_div_data[count_div])
					{
						// 约束成功 开始统计

						// 约束flavor规格
						for (int j_fla = 0, flag_fla = 0; flag_fla == 0; j_fla++)
						{
							if (count_div >= traincontrol.sample_num_noise) break;
							// 寻找到 "fla"作为查询条件
							if ((trainfileBuff[i][j_fla] == 102) && (trainfileBuff[i][j_fla + 1] == 108) && (trainfileBuff[i][j_fla + 2] == 97))
							{
								flag_fla = 1;
								// 确认是flavor后面是一位数字还是两位数字
								if (trainfileBuff[i][j_fla + 7] > 47 && trainfileBuff[i][j_fla + 7] < 58)
								{
									//两位数
									for (int kkk = 0; kkk < inputcontrol.flavorMaxnum; kkk++)
									{
										// 匹配乱序的虚拟机型号!!!
										if (((trainfileBuff[i][j_fla + 6] - CONV_ASCII_TO_INT) * 10 + trainfileBuff[i][j_fla + 7] - CONV_ASCII_TO_INT) == inputFlavor[kkk].flavor_name)
											trainfileFlavordata[count_div * inputcontrol.flavorMaxnum + kkk]++;
									}


								}
								else
								{
									for (int kkk = 0; kkk < inputcontrol.flavorMaxnum; kkk++)
									{
										// 匹配乱序的虚拟机型号!!!
										if ((trainfileBuff[i][j_fla + 6] - CONV_ASCII_TO_INT) == inputFlavor[kkk].flavor_name)
										{
											trainfileFlavordata[count_div * inputcontrol.flavorMaxnum + kkk]++;
											//printf("数组中第%d个 -- %d", (count_div * inputcontrol.flavorMaxnum + kkk), trainfileFlavordata[count_div * inputcontrol.flavorMaxnum + kkk]);
										}

									}

								}

								// 查询到就跳出循环,防止数组越界!!!
								// break;
							}
							// 剩下的数据就不需要了!!!

						}
					}
					else
					{  // 约束失败 增加一个周期(更换时间段)
						count_div++;
						// 剩下的数据就不需要了!!!
						if (count_div >= traincontrol.sample_num_noise) break;
						// 本段将必定约束成功 并完成统计!! 本部分代码在跳段阶段执行且只会执行1次
						if (temptime > valid_div_data[count_div + 1] && count_div < traincontrol.sample_num_noise && temptime <= valid_div_data[count_div])
						{
							// 约束成功 开始统计
							// 约束flavor规格
							for (int j_fla = 0, flag_fla = 0; flag_fla == 0; j_fla++)
							{
								// 寻找到 "fla"作为查询条件
								if ((trainfileBuff[i][j_fla] == 102) && (trainfileBuff[i][j_fla + 1] == 108) && (trainfileBuff[i][j_fla + 2] == 97))
								{
									flag_fla = 1;

									// 确认是flavor后面是一位数字还是两位数字
									if (trainfileBuff[i][j_fla + 7] > 47 && trainfileBuff[i][j_fla + 7] < 58)
									{
										//两位数
										for (int kkk = 0; kkk < inputcontrol.flavorMaxnum; kkk++)
										{
											// 匹配乱序的虚拟机型号!!!
											if (((trainfileBuff[i][j_fla + 6] - CONV_ASCII_TO_INT) * 10 + trainfileBuff[i][j_fla + 7] - CONV_ASCII_TO_INT) == inputFlavor[kkk].flavor_name)
												trainfileFlavordata[count_div * inputcontrol.flavorMaxnum + kkk]++;
										}
									}
									else
									{
										for (int kkk = 0; kkk < inputcontrol.flavorMaxnum; kkk++)
										{
											// 匹配乱序的虚拟机型号!!!
											if ((trainfileBuff[i][j_fla + 6] - CONV_ASCII_TO_INT) == inputFlavor[kkk].flavor_name)
											{
												trainfileFlavordata[count_div * inputcontrol.flavorMaxnum + kkk]++;
												//printf("数组中第%d个 -- %d", (count_div * inputcontrol.flavorMaxnum + kkk), trainfileFlavordata[count_div * inputcontrol.flavorMaxnum + kkk]);
											}
										}
									}
									// 查询到就跳出循环,防止数组越界!!!
									// break;
								}
							}
						}
					}
					//  break;
				}
			}
		}
	}

	// 顺序组织数组顺序 作为样本
	int *temp_trainfileFlavordata = new int[inputcontrol.flavorMaxnum * traincontrol.sample_num_noise];
	for (int i = 0; i < traincontrol.sample_num_noise; i++) {
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++) {
			temp_trainfileFlavordata[i*inputcontrol.flavorMaxnum + j] = trainfileFlavordata[(traincontrol.sample_num_noise - 1 - i)*inputcontrol.flavorMaxnum + j];
		}
	}
	for (int i = 0; i < inputcontrol.flavorMaxnum * traincontrol.sample_num_noise; i++) {
		trainfileFlavordata[i] = temp_trainfileFlavordata[i];
	}

	//print_data(trainfileFlavordata);

}

// 差分计算部分。  temp为要处理的长度！！
void get_trainfileFlavordata_diff(int *input, int *ouput, int temp)
{
	//int temp = traincontrol.sample_num - 1;  //要处理的长度
	for (int i = 0; i < temp; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			ouput[j + i * inputcontrol.flavorMaxnum] = input[j + (i + 1) * inputcontrol.flavorMaxnum] - input[j + i * inputcontrol.flavorMaxnum];
		}
	}
}



// 均值滤波器
void do_noise_avgfilter(int *trainfileFlavordata)
{
	int temp = traincontrol.sample_num_noise;  //要处理的长度

	float shouwei_3[3] = { 0.33, 0.33, 0.33 };
	float moban_3[3] = { 0.50, 0.0, 0.50 };  // 长度为3
	float shouwei_5[5] = { 0.2, 0.2, 0.2, 0.2, 0.2 };
	float moban_5[5] = { 0.25, 0.25,0.0,0.25,0.25 };  // 长度为5

													  // 首尾处理！长度为3
	for (int i = 0; i < temp; i++)
	{
		// 首
		trainfileFlavordata[i] = (int)(trainfileFlavordata[i + 0 * inputcontrol.flavorMaxnum] * shouwei_3[0] +
			trainfileFlavordata[i + 1 * inputcontrol.flavorMaxnum] * shouwei_3[1] + trainfileFlavordata[i + 2 * inputcontrol.flavorMaxnum] * shouwei_3[2] + 0.5);
		// 尾
		trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum] = (int)(trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum] * shouwei_3[0] +
			trainfileFlavordata[i + (temp - 2) * inputcontrol.flavorMaxnum] * shouwei_3[1] + trainfileFlavordata[i + (temp - 3) * inputcontrol.flavorMaxnum] * shouwei_3[2] + 0.5);
	}
	// 中间部分
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
		for (int j = 1; j < (temp - 1); j++)
		{
			trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] = (int)(trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum] * moban_3[0] +
				trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum] * moban_3[2] + 0.5);
		}
	}
}

// 节假日去噪！！
void do_noise_happyday(int *trainfileFlavordata, int *day_data, int tatal_daynum)
{
	for (int i = 0; i < tatal_daynum; i++)
	{
		for (int j = 0; j < HAPPYDAY_TOTALNUM - 1; j++)
		{
			if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[j] && i == 0)  //节假日是第一天(不是双11)
			{
				for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++) //依次赋值！！！
				{
					trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i + 3)* inputcontrol.flavorMaxnum + aaa];
					trainfileFlavordata[(i + 1)* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i + 3)* inputcontrol.flavorMaxnum + aaa];
					trainfileFlavordata[(i + 2)* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i + 3)* inputcontrol.flavorMaxnum + aaa];
				}
				i += 2;
				break;
			}
			if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[j] && i > 0 && i<tatal_daynum - 3)  //节假日不是第一天(不是双11)
			{
				for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++) //依次赋值！！！
				{
					trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = (trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa] + trainfileFlavordata[(i + 3)* inputcontrol.flavorMaxnum + aaa]) / 2;
					trainfileFlavordata[(i + 1)* inputcontrol.flavorMaxnum + aaa] = (trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa] + trainfileFlavordata[(i + 3)* inputcontrol.flavorMaxnum + aaa]) / 2;
					trainfileFlavordata[(i + 2)* inputcontrol.flavorMaxnum + aaa] = (trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa] + trainfileFlavordata[(i + 3)* inputcontrol.flavorMaxnum + aaa]) / 2;
				}
				i += 2;
				break;
			}

			if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[j] && i > 0 && i == tatal_daynum - 3)  //节假日是倒数3天(不是双11)
			{
				for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++) //依次赋值！！！
				{
					trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa];
					trainfileFlavordata[(i + 1)* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa];
					trainfileFlavordata[(i + 2)* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa];
				}
				i += 2;
				break;
			}

			if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[j] && i > 0 && i == tatal_daynum - 2)  //节假日是最后2天(不是双11)
			{
				for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++) //依次赋值！！！
				{
					trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa];
					trainfileFlavordata[(i + 1)* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa];
				}
				i += 2;
				break;
			}

			if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[j] && i > 0 && i == tatal_daynum - 1)  //节假日是最后一天(不是双11)
			{
				for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++) //依次赋值！！！
				{
					trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa];
				}
				i += 2;
				break;
			}

		}
		if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[9] && i == 0)  //11.11是第一天，真尴尬
		{
			for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++)
			{
				trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i + 1)* inputcontrol.flavorMaxnum + aaa];
			}
		}
		if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[9] && i > 0 && i != (tatal_daynum - 1))  //11.11 不是第一天！！也不是最后一天
		{
			for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++)
			{
				trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = (trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa] + trainfileFlavordata[(i + 1)* inputcontrol.flavorMaxnum + aaa]) / 2;
			}
		}
		if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[9] && i > 0 && i == (tatal_daynum - 1))  //11.11 是最后一天
		{
			for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++)
			{
				trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa];
			}
		}

	}
}



// 中值滤波
void do_noise_midfilter(int *trainfileFlavordata, Flavor *inputFlavor)
{
	int temp = traincontrol.sample_num_noise;  //要处理的长度

											   // 去除头和尾
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
		// 首
		// printf("trainfileFlavordata[%d] : %d\n", i, trainfileFlavordata[i]);
		// printf("trainfileFlavordata[%d] : %d\n", i + 1 * inputcontrol.flavorMaxnum, trainfileFlavordata[i + 1 * inputcontrol.flavorMaxnum]);
		// printf("trainfileFlavordata[%d] : %d\n\n", i + 2 * inputcontrol.flavorMaxnum, trainfileFlavordata[i + 2 * inputcontrol.flavorMaxnum]);
		if (trainfileFlavordata[i] > trainfileFlavordata[i + 1 * inputcontrol.flavorMaxnum] &&
			trainfileFlavordata[i] > trainfileFlavordata[i + 2 * inputcontrol.flavorMaxnum] &&
			trainfileFlavordata[i] > (NOISE_B + trainfileFlavordata[i + 1 * inputcontrol.flavorMaxnum] * NOISE_W + trainfileFlavordata[i + 2 * inputcontrol.flavorMaxnum] * NOISE_W)
			&& (inputFlavor[i].flavor_name == 1 || inputFlavor[i].flavor_name == 4 || inputFlavor[i].flavor_name == 7 || inputFlavor[i].flavor_name == 10 || inputFlavor[i].flavor_name == 13 ||
				inputFlavor[i].flavor_name == 3 || inputFlavor[i].flavor_name == 6 || inputFlavor[i].flavor_name == 9 || inputFlavor[i].flavor_name == 12 || inputFlavor[i].flavor_name == 15
				|| inputFlavor[i].flavor_name == 2 || inputFlavor[i].flavor_name == 5 || inputFlavor[i].flavor_name == 8 || inputFlavor[i].flavor_name == 11 || inputFlavor[i].flavor_name == 14
				))
		{
			printf("冷门类型中的高峰  首部位置[ %d ]   去除之前 : [ %d %d %d ]\n", i, trainfileFlavordata[i], trainfileFlavordata[i + 1 * inputcontrol.flavorMaxnum], trainfileFlavordata[i + 2 * inputcontrol.flavorMaxnum]);
			trainfileFlavordata[i] = (int)(trainfileFlavordata[i + 1 * inputcontrol.flavorMaxnum] + trainfileFlavordata[i + 2 * inputcontrol.flavorMaxnum] + 0.5);
			//trainfileFlavordata[i] = (int)(trainfileFlavordata[i] / 2 + 1);
			printf("冷门类型中的高峰  首部位置[ %d ]   去除之后 : [ %d %d %d ]\n", i, trainfileFlavordata[i], trainfileFlavordata[i + 1 * inputcontrol.flavorMaxnum], trainfileFlavordata[i + 2 * inputcontrol.flavorMaxnum]);
		}



		// 尾
		// printf("trainfileFlavordata[%d] : %d\n", i + (temp - 1) * inputcontrol.flavorMaxnum, trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum]);
		// printf("trainfileFlavordata[%d] : %d\n", i + (temp - 2) * inputcontrol.flavorMaxnum, trainfileFlavordata[i + (temp - 2) * inputcontrol.flavorMaxnum]);
		// printf("trainfileFlavordata[%d] : %d\n", i + (temp - 3) * inputcontrol.flavorMaxnum, trainfileFlavordata[i + (temp - 3) * inputcontrol.flavorMaxnum]);
		if (trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum] > trainfileFlavordata[i + (temp - 2) * inputcontrol.flavorMaxnum] &&
			trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum] > trainfileFlavordata[i + (temp - 3) * inputcontrol.flavorMaxnum] &&
			trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum] > (NOISE_B + trainfileFlavordata[i + (temp - 2) * inputcontrol.flavorMaxnum] * NOISE_W + trainfileFlavordata[i + (temp - 3) * inputcontrol.flavorMaxnum] * NOISE_W)
			&& (inputFlavor[i].flavor_name == 1 || inputFlavor[i].flavor_name == 4 || inputFlavor[i].flavor_name == 7 || inputFlavor[i].flavor_name == 10 || inputFlavor[i].flavor_name == 13 ||
				inputFlavor[i].flavor_name == 3 || inputFlavor[i].flavor_name == 6 || inputFlavor[i].flavor_name == 9 || inputFlavor[i].flavor_name == 12 || inputFlavor[i].flavor_name == 15
				|| inputFlavor[i].flavor_name == 2
				|| inputFlavor[i].flavor_name == 5
				|| inputFlavor[i].flavor_name == 8
				|| inputFlavor[i].flavor_name == 11
				|| inputFlavor[i].flavor_name == 14
				))
		{
			printf("冷门类型中的高峰  尾部位置[ %d ]   去除之前 : [ %d %d %d ]\n", i, trainfileFlavordata[i + (temp - 3) * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (temp - 2) * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum]);
			// trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum] = (int)(trainfileFlavordata[i + (temp - 2) * inputcontrol.flavorMaxnum] + trainfileFlavordata[i + (temp - 3) * inputcontrol.flavorMaxnum] + 0.5);
			trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum] = (int)(trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum] / 2 + 1);
			printf("冷门类型中的高峰  尾部位置[ %d ]   去除之后 : [ %d %d %d ]\n", i, trainfileFlavordata[i + (temp - 3) * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (temp - 2) * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum]);

		}
	}


	// 先处理中间
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
		for (int j = 1; j < (temp - 1); j++)
		{
			// 仅仅去除大的 尖峰就OK
			// printf("trainfileFlavordata[%d] : %d\n", i + (j - 1)*inputcontrol.flavorMaxnum, trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum]);
			// printf("trainfileFlavordata[%d] : %d\n", i + (j - 0)*inputcontrol.flavorMaxnum, trainfileFlavordata[i + (j - 0)*inputcontrol.flavorMaxnum]);
			// printf("trainfileFlavordata[%d] : %d\n", i + (j + 1)*inputcontrol.flavorMaxnum, trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum]);
			if (trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum] < trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] &&
				trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum] < trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] &&
				((trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum] * NOISE_W + trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum] * NOISE_W + NOISE_B) < trainfileFlavordata[i + j * inputcontrol.flavorMaxnum])
				&& (inputFlavor[i].flavor_name == 1 || inputFlavor[i].flavor_name == 4 || inputFlavor[i].flavor_name == 7 || inputFlavor[i].flavor_name == 10 || inputFlavor[i].flavor_name == 13 ||
					inputFlavor[i].flavor_name == 3 || inputFlavor[i].flavor_name == 6 || inputFlavor[i].flavor_name == 9 || inputFlavor[i].flavor_name == 12 || inputFlavor[i].flavor_name == 15
					//  || inputFlavor[i].flavor_name == 2 || inputFlavor[i].flavor_name == 5 || inputFlavor[i].flavor_name == 8 || inputFlavor[i].flavor_name == 11 || inputFlavor[i].flavor_name == 14
					))
			{
				// 求平均，再四舍五入！
				printf("冷门类型中的高峰  中间位置[ %d %d ]   去除之前 : [ %d %d %d ]\n", i, j, trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum], trainfileFlavordata[i + j * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum]);
				//trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] = (int)(trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum] + trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum] + 0.5);
				trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] = (int)(trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] / 2 + 0.5);
				printf("冷门类型中的高峰  中间位置[ %d %d ]   去除之后 : [ %d %d %d ]\n", i, j, trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum], trainfileFlavordata[i + j * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum]);
			}

		}
	}

	// 处理中间的 低谷数据
	/*for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
	for (int j = 1; j < (temp - 1); j++)
	{
	// 仅仅去除大的 尖峰就OK
	// printf("trainfileFlavordata[%d] : %d\n", i + (j - 1)*inputcontrol.flavorMaxnum, trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum]);
	// printf("trainfileFlavordata[%d] : %d\n", i + (j - 0)*inputcontrol.flavorMaxnum, trainfileFlavordata[i + (j - 0)*inputcontrol.flavorMaxnum]);
	// printf("trainfileFlavordata[%d] : %d\n", i + (j + 1)*inputcontrol.flavorMaxnum, trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum]);
	if (trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum] > trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] &&
	trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum] > trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] &&
	((trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum] * 0.5 + trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum] * 0.5) > 4*(NOISE_W * trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] + NOISE_B))
	&& (inputFlavor[i].flavor_name == 1 || inputFlavor[i].flavor_name == 4 || inputFlavor[i].flavor_name == 7 || inputFlavor[i].flavor_name == 10 || inputFlavor[i].flavor_name == 13 ||
	inputFlavor[i].flavor_name == 3 || inputFlavor[i].flavor_name == 6 || inputFlavor[i].flavor_name == 9 || inputFlavor[i].flavor_name == 12 || inputFlavor[i].flavor_name == 15
	|| inputFlavor[i].flavor_name == 2 || inputFlavor[i].flavor_name == 5 || inputFlavor[i].flavor_name == 8 || inputFlavor[i].flavor_name == 11 || inputFlavor[i].flavor_name == 14
	))
	{
	// 求平均，再四舍五入！
	printf("冷门类型中的低谷  中间位置[ %d %d ]   去除之前 : [ %d %d %d ]\n", i, j, trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum], trainfileFlavordata[i + j * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum]);
	trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] = (int)( (trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum] + trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum])*0.2 + trainfileFlavordata[i + j * inputcontrol.flavorMaxnum]);
	// trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] = (int)(trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] / 2 + 0.5);
	printf("冷门类型中的低谷  中间位置[ %d %d ]   去除之后 : [ %d %d %d ]\n", i, j, trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum], trainfileFlavordata[i + j * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum]);
	}

	}
	}*/



}


// 打印样本
void print_data(int *data, int hangshu)
{
	printf("\n[");
	for (int i = 0; i < (hangshu * inputcontrol.flavorMaxnum); i++)
	{
		printf(" %d", data[i]);
		if ((i + 1) % inputcontrol.flavorMaxnum == 0 && (hangshu * inputcontrol.flavorMaxnum - 1) != i) 	printf("]\n[");
		if ((i + 1) % inputcontrol.flavorMaxnum == 0 && (hangshu * inputcontrol.flavorMaxnum - 1) == i) 	printf("]\n");
	}
}


/*
函数目标:获取最近一天的样本
输入:*traindata 为训练数据
num_vm  为虚拟服务器种类数量
num_traindata 为训练数据组数
preidct_sample_day_num  需要预测的天数
*/
double *predict_get_ori_data(int *traindata, int num_vm, int num_traindata, int preidct_sample_day_num) {
	double *out_double = new double[num_vm];
	int line_num = num_traindata - 1; //最后一行的行号
	for (int i = 0; i < num_vm; i++) {
		out_double[i] = (double)traindata[line_num*num_vm + i] * ((double)preidct_sample_day_num / CYC_SAMPLE);
	}
	return out_double;
}



/*
函数目标:预测下一阶段的需求量
方法:简单逻辑回归
输入:*traindata 为训练数据
num_vm  为虚拟服务器种类数量
num_traindata 为训练数据组数
preidct_sample_day_num  需要预测的天数
*/
double *predict_run_liner(int *traindata, int num_vm, int num_traindata, int preidct_sample_day_num) {
	double *out_double = new double[num_vm];
	double *out_int = new double[num_vm];
	double *W = new double[num_traindata];
	double sum_W = 0;
	//构造权值矩阵
	for (int i = 0; i < num_traindata; i++) {  //权值线性增长
		W[i] = i * i;
		sum_W += W[i];
	}
	for (int i = 0; i < num_traindata; i++) {
		W[i] = W[i] / sum_W;
	}



	//预测
	for (int i = 0; i < num_vm; i++) {
		out_double[i] = 1;
	}
	for (int i = 0; i < num_vm; i++) {
		for (int j = 0; j < num_traindata; j++) {
			out_double[i] += (double)traindata[j*num_vm + i] * W[j];
		}
	}
	//cout << "\n";
	for (int i = 0; i < num_vm; i++) {
		out_int[i] = out_double[i] * ((double)preidct_sample_day_num / CYC_SAMPLE);
	}
	return out_int;
}


/*
函数目标:预测下一阶段的需求量
方法: 循环最小二乘
输入:*traindata 为训练数据
num_vm  为虚拟服务器种类数量
num_traindata 为训练数据组数
输出:out_int为每种虚拟机的需求量
preidct_sample_day_num  需要预测的天数
*/
int *predict_run_RLMS(int *traindata, int num_vm, int num_traindata, int preidct_sample_day_num) {
	int i, j;
	// 多少组训练一次
	int N = 8;  // 入参preidct_sample_day_num必须大于N
	double erfa = 0.1; //W的学习率

	if (N > num_traindata) {
		printf("Error! NN Training Dosen't Has Enough Training Data");
		printf("num_traindata Must Larger Than N !");
	}
	// 构造初始权值矩阵
	double *W_line = new double[N];
	double *W = new double[num_vm*N];   //每一类都有一个权值数组
	double sum_W = 0;
	for (i = 0; i < N; i++) {  //权值线性增长
		W_line[i] = i + 1;
		sum_W += W_line[i];
	}
	for (i = 0; i < N; i++) {
		W_line[i] = W_line[i] / sum_W;
	}
	for (i = 0; i < num_vm; i++) {
		for (j = 0; j < N; j++) {
			W[i*N + j] = W_line[j];  //每一行代表一个类的权值
		}
	}

	/*printf("\nW_line:\n"); //显示天与天之间的权值
	for (i = 0; i < N; i++) {
	printf("%f ", W_line[i]);
	}*/

	//开始训练
	double *x = new double[N]; //单次训练的网络输入x，N维矩阵
	double y_exp;   //单次训练的网络期望输出y_exp
	double y;   //单次训练的网络实际输出y_exp
	double dw;   //w偏差
	double da, dz;

	int train_time;//当前训练次数
	int train_class_time;  //当前训练第几类
	for (train_class_time = 0; train_class_time < num_vm; train_class_time++) { //训练第train_class_time类虚拟机
																				//单维度的权值
		double *w_single = new double[N];
		for (i = 0; i < N; i++) {  //总权值赋值给单维度
			w_single[i] = W[train_class_time*N + i];
		}
		for (train_time = 0; train_time < (num_traindata - N); train_time++) {  //第train_time次训练
																				/*-------------------------前向------------------------------------*/
																				// 构造当次训练样本并预测
			y = 0;
			for (i = 0; i < N; i++) {  //构建输入x
				x[i] = (double)traindata[(train_time + i)*num_vm + train_class_time];
				y += x[i] * w_single[i]; //预测输出
			}
			y_exp = (double)traindata[(train_time + N)*num_vm + train_class_time];

			//显示
			printf("\n输入\n");
			for (i = 0; i < N; i++) {
				printf("%f ", x[i]);
			}
			printf("\n期望输出：%f ", y_exp);
			printf("\n实际输出：%f ", y);

			y_exp = 1 / (1 + exp(-y_exp));  //激活函数sigmoid
			y = 1 / (1 + exp(-y));
			da = -(y_exp / y + 0.0000001) + (1 - y_exp) / (1 - y + 0.0000001);
			dz = da * y*(1 - y);

			printf("\n更新前W\n");
			for (i = 0; i < N; i++) {
				printf("%f ", w_single[i]);
			}

			//更新权值
			for (i = 0; i < N; i++) {
				w_single[i] = w_single[i] + erfa * dz * x[i];
				if (w_single[i] < 0) w_single[i] = 0;
			}

			printf("\n更新后W\n");
			for (i = 0; i < N; i++) {
				printf("%f ", w_single[i]);
			}
		}
		for (i = 0; i < N; i++) {  //迭代后单维度赋值给总权值矩阵
			W[train_class_time*N + i] = w_single[i];
		}

		printf("\n\n");
	}


	printf("\nW\n");
	for (i = 0; i < num_vm; i++) {
		printf("\n");
		for (j = 0; j < N; j++) {
			printf("%f ", W[i*N + j]);
		}
	}


	//预测
	int predict_num_int = preidct_sample_day_num / CYC_SAMPLE;  //需要预测的样本整数
	int predict_num_rem = preidct_sample_day_num % CYC_SAMPLE;  //需要预测的样本余数
	double *out_double = new double[num_vm];
	for (i = 0; i < num_vm; i++) {
		out_double[i] = 0;
	}
	for (train_class_time = 0; train_class_time < num_vm; train_class_time++) { //训练第train_class_time类虚拟机
																				//单维度的权值
		double *w_single = new double[N];
		for (i = 0; i < N; i++) {  //总权值赋值给单维度
			w_single[i] = W[train_class_time*N + i];
		}
		double *predict_input = new double[N + predict_num_int];
		for (i = 0; i < N; i++) {
			predict_input[i] = (double)traindata[((num_traindata - N) + i)*num_vm + train_class_time];
		}

		/*printf("\ninput:\n"); //显示天与天之间的权值
		for (i = 0; i < N; i++) {
		printf("%f ", predict_input[i]);
		}*/

		y = 0;
		for (int pre_count = 0; pre_count < predict_num_int; pre_count++) {
			for (i = pre_count; i < N + pre_count; i++) {
				y += predict_input[i] * w_single[i]; //预测输出
			}

			/*printf("\n\npredict_input:\n");
			for (i = 0; i < N; i++) {
			printf("%f ", predict_input[i]);
			}

			printf("\nw_single:\n"); //显示天与天之间的权值
			for (i = 0; i < N; i++) {
			printf("%f ", w_single[i]);
			}*/

			predict_input[N + pre_count] = y;
			out_double[train_class_time] += y;
		}
		if (predict_num_rem != 0) { //还有余数
			out_double[train_class_time] += y * (double)predict_num_rem / (double)CYC_SAMPLE;
		}
	}

	/*printf("\nout\n");
	for (i = 0; i < num_vm; i++) {
	printf("%f ", out_double[i]);
	}*/

	int *out_int = new int[num_vm];
	for (i = 0; i < num_vm; i++) {
		out_int[i] = round_my(out_double[i]);
	}

	return out_int;
}



/*
函数目标:预测下一阶段的需求量
方法: 循环W均值
输入:*traindata 为训练数据
num_vm  为虚拟服务器种类数量
num_traindata 为训练数据组数
输出:out_int为每种虚拟机的需求量
preidct_sample_day_num  需要预测的天数
*/
int *predict_run_Wmean(int *traindata, int num_vm, int num_traindata, int preidct_sample_day_num) {
	int i, j;
	// 多少组训练一次
	int N = num_traindata - 4;  // 入参preidct_sample_day_num必须大于N

	if (N <= 0) {
		printf("dosne't has enough data!!");
		while (1); //没法算 ,就死在这
	}
	// 构造初始权值矩阵
	double *W_line = new double[N];
	double *W = new double[num_vm*N];   //每一类都有一个权值数组
	double sum_W = 0;
	/*-------------初始权值-------------*/
	for (i = 0; i < N; i++) {  //权值线性增长
		W_line[i] = i + 1;
		sum_W += W_line[i];
	}
	for (i = 0; i < N; i++) {
		W_line[i] = W_line[i] / sum_W;
	}
	for (i = 0; i < num_vm; i++) {
		for (j = 0; j < N; j++) {
			W[i*N + j] = W_line[j];  //每一行代表一个类的权值
		}
	}

	/*-------------初始权值-------------*/
	double *x = new double[N]; //单次训练的网络输入x，N维矩阵
	double y_exp;   //单次训练的网络期望输出y_exp

	int train_totol_time = num_traindata - N;//总训练次数
	int train_time;//当前训练次数
	int train_class_time;  //当前训练第几类
	for (train_class_time = 0; train_class_time < num_vm; train_class_time++) { //训练第train_class_time类虚拟机
																				//单维度的权值
		double *w_single = new double[N];
		double *w_single_temp = new double[N];
		double sum_w_single_temp;
		for (i = 0; i < N; i++) {  //总权值赋值给单维度
			w_single[i] = 0;
		}
		for (train_time = 0; train_time < train_totol_time; train_time++) {  //第train_time次训练
																			 // 构造当次训练样本并预测
			y_exp = (double)traindata[(train_time + N)*num_vm + train_class_time];  //期望输出y
			sum_w_single_temp = 0; //权值之和
			for (i = 0; i < N; i++) {
				x[i] = (double)traindata[(train_time + i)*num_vm + train_class_time]; //构建输入x
				w_single_temp[i] = (y_exp - x[i])*(y_exp - x[i]); //求输入与期望的偏差
				sum_w_single_temp += w_single_temp[i];
			}
			for (i = 0; i < N; i++) {
				w_single_temp[i] = w_single_temp[i] / sum_w_single_temp;
				w_single[i] += w_single_temp[i];
			}


		}
		for (i = 0; i < N; i++) {  //迭代后单维度赋值给总权值矩阵
			W[train_class_time*N + i] = w_single[i] / train_totol_time;
		}

		printf("\n\n");
	}


	printf("\nW\n");
	for (i = 0; i < num_vm; i++) {
		printf("\n");
		for (j = 0; j < N; j++) {
			printf("%f ", W[i*N + j]);
		}
	}


	//预测
	int predict_num_int = preidct_sample_day_num / CYC_SAMPLE;  //需要预测的样本整数
	int predict_num_rem = preidct_sample_day_num % CYC_SAMPLE;  //需要预测的样本余数
	double *out_double = new double[num_vm];
	double y;
	for (i = 0; i < num_vm; i++) {
		out_double[i] = 0;
	}
	for (train_class_time = 0; train_class_time < num_vm; train_class_time++) { //训练第train_class_time类虚拟机
																				//单维度的权值
		double *w_single = new double[N];
		for (i = 0; i < N; i++) {  //总权值赋值给单维度
			w_single[i] = W[train_class_time*N + i];
		}
		double *predict_input = new double[N + predict_num_int];
		for (i = 0; i < N; i++) {
			predict_input[i] = (double)traindata[((num_traindata - N) + i)*num_vm + train_class_time];
		}

		/*printf("\ninput:\n"); //显示天与天之间的权值
		for (i = 0; i < N; i++) {
		printf("%f ", predict_input[i]);
		}*/

		y = 0;
		for (int pre_count = 0; pre_count < predict_num_int; pre_count++) {
			for (i = pre_count; i < N + pre_count; i++) {
				y += predict_input[i] * w_single[i]; //预测输出
			}

			/*printf("\n\npredict_input:\n");
			for (i = 0; i < N; i++) {
			printf("%f ", predict_input[i]);
			}

			printf("\nw_single:\n"); //显示天与天之间的权值
			for (i = 0; i < N; i++) {
			printf("%f ", w_single[i]);
			}*/

			predict_input[N + pre_count] = y;
			out_double[train_class_time] += y;
		}
		if (predict_num_rem != 0) { //还有余数
			out_double[train_class_time] += y * (double)predict_num_rem / (double)CYC_SAMPLE;
		}
	}

	/*printf("\nout\n");
	for (i = 0; i < num_vm; i++) {
	printf("%f ", out_double[i]);
	}*/

	int *out_int = new int[num_vm];
	for (i = 0; i < num_vm; i++) {
		out_int[i] = round_my(out_double[i]);
	}

	return out_int;
}



/*
函数目标:预测下一阶段的需求量
方法: RLS自适应滤波
输入:*traindata 为训练数据
num_vm  为虚拟服务器种类数量
num_traindata 为训练数据组数
输出:out_int为每种虚拟机的需求量
*/
int *predict_run_RLS(int *traindata, int num_vm, int num_traindata) {
	/*运行参数*/
	int i, j;
	int *out_int = new int[num_vm];
	int M = num_vm; //滤波器阶数:虚拟机种类数量
	int Signal_Len = num_traindata; //训练样本数
									/*算法参数*/
	double lamda = 1; //遗忘因子
	double c = 1;  //小正数 保证矩阵P非奇异
				   /*算法变量*/
	double *out_double = new double[M*M]; //输出
	double *I = new double[M*M]; //M*M的单位阵
	for (i = 0; i < M; i++) {  //I单位阵赋初值
		for (j = 0; j < M; j++) {
			if (i == j) {
				I[i*M + j] = 1;
			}
			else {
				I[i*M + j] = 0;
			}
		}
	}
	double *P_last = new double[M*M];
	double *W_last = new double[M*M];
	double *W = new double[M*M];
	double *P = new double[M*M];
	double *K_x = new double[M*M]; //K*x' 维度M*M
	double *x = new double[M]; //每次训练的输入数据容器
	double *d = new double[M]; //每次训练的期望数据容器
	double *y = new double[M]; //每次训练的输出数据容器
	double *Eta = new double[M]; //每次训练的误差容器
	double *x_plast = new double[M]; //x'*P_last:维度1*M
	double *x_plast_x = new double[1]; //x'*P_last*x:维度1
	double *K = new double[M]; //P_last*x,维度M*1
							   /*算法主体*/
	for (int train_times = 0; train_times < Signal_Len - 1; train_times++) {
		if (train_times == 0) { //第一次训练开始前初始化参数
			for (i = 0; i < M; i++) {
				for (j = 0; j < M; j++) {
					P_last[i*M + j] = I[i*M + j] / c; //P初始化
					W_last[i*M + j] = 0;  //W初始化为0方阵
				}
			}
		}
		//输入数据
		for (i = 0; i < M; i++) {
			x[i] = (double)traindata[train_times*M + i];    //输入新的信号矢量
			d[i] = (double)traindata[(train_times + 1)*M + i];  //输入新的期望信号
		}


		//开始计算
		mat_mult(x_plast, x, 1, M, P_last, M, M);
		mat_mult(x_plast_x, x_plast, 1, M, x, M, 1);
		mat_mult(K, P_last, M, M, x, M, 1);
		for (i = 0; i < M; i++) {
			K[i] /= (x_plast_x[0] + lamda);
		}
		mat_mult(y, x, 1, M, W_last, M, M);

		for (i = 0; i < M; i++) {  //误差
			Eta[i] = d[i] - y[i];
		}

		mat_mult(W, K, M, 1, Eta, 1, M);
		for (i = 0; i < M; i++) { //更新W
			for (j = 0; j < M; j++) {
				W_last[i*M + j] = W_last[i*M + j] + W[i*M + j];
			}
		}
		mat_mult(K_x, K, M, 1, x, 1, M);
		for (i = 0; i < M; i++) { //更新P的中间过程
			for (j = 0; j < M; j++) {
				K_x[i*M + j] = I[i*M + j] - K_x[i*M + j];
			}
		}
		mat_mult(P, K_x, M, M, P_last, M, M);
		for (i = 0; i < M; i++) { //更新P
			for (j = 0; j < M; j++) {
				P_last[i*M + j] = P[i*M + j] / lamda;
			}
		}
	}
	for (i = 0; i < M; i++) {
		x[i] = (double)traindata[(Signal_Len - 1)*M + i];    //输入新的信号矢量
	}
	mat_mult(out_double, x, 1, M, W_last, M, M);
	for (i = 0; i < M; i++) {
		out_int[i] = round_my(out_double[i]);
	}
	return out_int;
}

/*
矩阵乘法:mat1*mat2
输入参数:mat1-矩阵1  mat1_height-矩阵1高度  mat1_width-矩阵1宽度
mat2-矩阵2  mat1_height-矩阵2高度  mat2_width-矩阵1宽度
*/
void mat_mult(double *mult_out, double *mat1, int mat1_height, int mat1_width, double *mat2, int mat2_height, int mat2_width) {
	if (mat1_width != mat2_height)  printf("矩阵乘法输入格式错误");
	for (int i = 0; i < mat1_height; i++) {
		for (int j = 0; j < mat2_width; j++) {
			mult_out[i*mat2_width + j] = 0;
			for (int i_aline = 0; i_aline < mat1_width; i_aline++) {
				mult_out[i*mat2_width + j] += mat1[i*mat1_width + i_aline] * mat2[i_aline*mat2_width + j];
			}
		}
	}
}


/*
函数目标:放置虚拟机
方法:	贪心FFD
输入:	*require_vm		虚拟机需求
num_vm			虚拟机类型总数
*inputFlavor	虚拟机性能表
resource_pm		服务器性能结构体
opt_target		优化目标 0:CPU 1:MEM
*result_save	服务器数据表(结果)
输出:	创建的服务器个数
*/
int putVM_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	// 保存一份完好的初始化的 虚拟机性能表！！！
	/*
	Flavor *inputFlavor_data = new Flavor[inputcontrol.flavorMaxnum];
	for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
	{
	inputFlavor_data[i].cpu_core_num = inputFlavor[i].cpu_core_num;
	inputFlavor_data[i].mem_size = inputFlavor[i].mem_size;
	inputFlavor_data[i].flavor_name = inputFlavor[i].flavor_name;
	}
	*/

	/***********************************************************排序部分*****************************************************************/
	if (opt_target == 0)
	{
		// 实现排序从大到小
		for (int i = 0; i < num_vm - 1; i++)
		{
			for (int j = i + 1; j < num_vm; j++)
			{
				if (inputFlavor[j].cpu_core_num>inputFlavor[i].cpu_core_num) // 后面cpu大于前面,交换
				{
					Flavor tmp_flavor;
					// 性能表位置交换
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// 需求位置交换
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
				else if (inputFlavor[j].cpu_core_num == inputFlavor[i].cpu_core_num && inputFlavor[j].mem_size>inputFlavor[i].mem_size) // 后面cpu相等,mem大于前面,交换
				{
					Flavor tmp_flavor;
					// 性能表位置交换
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// 需求位置交换
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
			}
		}
	}
	else if (opt_target == 1)
	{
		// 实现排序从大到小
		for (int i = 0; i < num_vm - 1; i++)
		{
			for (int j = i + 1; j < num_vm; j++)
			{
				if (inputFlavor[j].mem_size>inputFlavor[i].mem_size) // 后面mem大于前面,交换
				{
					Flavor tmp_flavor;
					// 性能表位置交换
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// 需求位置交换
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
				else if (inputFlavor[j].mem_size == inputFlavor[i].mem_size && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // mem相等 后面CPU大于前面,交换
				{
					Flavor tmp_flavor;
					// 性能表位置交换
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// 需求位置交换
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
			}
		}
	}
	else    // 目标选择错误 退出,异常
	{
		return -1;
	}

	// 需求copy
	int *require_vm_copy = new int[num_vm];
	for (int i = 0; i < num_vm; i++)
		require_vm_copy[i] = require_vm[i];



	/********************************************************************放置部分***********************************************************************/
	// 创建服务器余量记录表
	Server *server_remain = new Server[2000];
	// 数据赋值(这里面的2000要和函数外面的max_serve_py对应,函数可以考虑多加一个max_serve_py接口)
	for (int i = 0; i < 2000; i++)
	{
		server_remain[i].cpu_core_num = resource_pm.cpu_core_num;
		server_remain[i].mem_size = resource_pm.mem_size;
	}

	int server_num = 1;
	for (int i = 0; i < num_vm; i++)	// 一种一种放置
	{
		// 当前虚拟机种类的起始搜索索引
		int j = 0;
		while (require_vm_copy[i] != 0)		// 每一种一个一个放置,直到放完
		{
			// 当前此个虚拟机放置标志位
			int flag_put = 0;
			// 遍历搜索
			for (; j < server_num; j++)
			{
				// 不能放下
				if (server_remain[j].cpu_core_num < inputFlavor[i].cpu_core_num || server_remain[j].mem_size < inputFlavor[i].mem_size)		// 任意资源超标
				{
					continue;	// 直接进行下一次的搜索
				}
				else if (server_remain[j].cpu_core_num >= inputFlavor[i].cpu_core_num && server_remain[j].mem_size >= inputFlavor[i].mem_size)	// 保证放下时资源充足,则放下
				{
					server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
					server_remain[j].mem_size -= inputFlavor[i].mem_size;
					flag_put = 1;
					result_save[j*num_vm + i]++;	// 放置记录
					break;	// 搜索到箱子,跳出循环
				}
			}
			// 出上面的for循环有可能是遍历完都放不下,也有可能是放置好了直接跳出
			if (flag_put == 0)
			{
				server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
				server_remain[j].mem_size -= inputFlavor[i].mem_size;
				result_save[j*num_vm + i]++;	// 放置记录
				server_num++;	// 新开一个服务器

			}

			require_vm_copy[i]--; // 此种虚拟机数量减一
		}
	}

	// 进行还原！！！
	/*
	for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
	{
	inputFlavor[i].cpu_core_num = inputFlavor_data[i].cpu_core_num;
	inputFlavor[i].mem_size = inputFlavor_data[i].mem_size;
	inputFlavor[i].flavor_name = inputFlavor_data[i].flavor_name;
	}
	*/

	return server_num;
}

int putVM_greedy_AAA(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)

{

	/***********************************************************排序部分*****************************************************************/
	if (opt_target == 0)
	{
		// 实现排序从大到小
		for (int i = 0; i < num_vm - 1; i++)
		{
			for (int j = i + 1; j < num_vm; j++)
			{
				if (inputFlavor[j].cpu_core_num>inputFlavor[i].cpu_core_num) // 后面cpu大于前面,交换
				{
					Flavor tmp_flavor;
					// 性能表位置交换
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// 需求位置交换
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
				else if (inputFlavor[j].cpu_core_num == inputFlavor[i].cpu_core_num && inputFlavor[j].mem_size>inputFlavor[i].mem_size) // 后面cpu相等,mem大于前面,交换
				{
					Flavor tmp_flavor;
					// 性能表位置交换
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// 需求位置交换
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
			}
		}
	}
	else if (opt_target == 1)
	{
		// 实现排序从大到小
		for (int i = 0; i < num_vm - 1; i++)
		{
			for (int j = i + 1; j < num_vm; j++)
			{
				if (inputFlavor[j].mem_size>inputFlavor[i].mem_size) // 后面mem大于前面,交换
				{
					Flavor tmp_flavor;
					// 性能表位置交换
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// 需求位置交换
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
				else if (inputFlavor[j].mem_size == inputFlavor[i].mem_size && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // mem相等 后面CPU大于前面,交换
				{
					Flavor tmp_flavor;
					// 性能表位置交换
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// 需求位置交换
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
			}
		}
	}
	else    // 目标选择错误 退出,异常
	{
		return -1;
	}


	// 需求copy
	int *require_vm_copy = new int[num_vm];
	for (int i = 0; i < num_vm; i++)
		require_vm_copy[i] = require_vm[i];



	/********************************************************************放置部分***********************************************************************/
	// 创建服务器余量记录表
	Server *server_remain = new Server[2000];
	// 数据赋值(这里面的2000要和函数外面的max_serve_py对应,函数可以考虑多加一个max_serve_py接口)
	for (int i = 0; i < 2000; i++)
	{
		server_remain[i].cpu_core_num = resource_pm.cpu_core_num;
		server_remain[i].mem_size = resource_pm.mem_size;
	}

	int server_num = 1;
	for (int i = 0; i < num_vm; i++)	// 一种一种放置
	{
		// 当前虚拟机种类的起始搜索索引
		int j = 0;
		while (require_vm_copy[i] != 0)		// 每一种一个一个放置,直到放完
		{
			// 当前此个虚拟机放置标志位
			int flag_put = 0;
			// 遍历搜索
			for (; j < server_num; j++)
			{
				// 不能放下
				if (server_remain[j].cpu_core_num < inputFlavor[i].cpu_core_num || server_remain[j].mem_size < inputFlavor[i].mem_size)		// 任意资源超标
				{
					continue;	// 直接进行下一次的搜索
				}
				else if (server_remain[j].cpu_core_num >= inputFlavor[i].cpu_core_num && server_remain[j].mem_size >= inputFlavor[i].mem_size)	// 保证放下时资源充足,则放下
				{
					server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
					server_remain[j].mem_size -= inputFlavor[i].mem_size;
					flag_put = 1;
					result_save[j*num_vm + i]++;	// 放置记录
					break;	// 搜索到箱子,跳出循环
				}
			}
			// 出上面的for循环有可能是遍历完都放不下,也有可能是放置好了直接跳出
			if (flag_put == 0)
			{
				server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
				server_remain[j].mem_size -= inputFlavor[i].mem_size;
				result_save[j*num_vm + i]++;	// 放置记录
				server_num++;	// 新开一个服务器

			}

			require_vm_copy[i]--; // 此种虚拟机数量减一
		}
	}

	// 进行还原！！！
	/*
	for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
	{
	inputFlavor[i].cpu_core_num = inputFlavor_data[i].cpu_core_num;
	inputFlavor[i].mem_size = inputFlavor_data[i].mem_size;
	inputFlavor[i].flavor_name = inputFlavor_data[i].flavor_name;
	}
	*/

	return server_num;
}

int putVM_greedy_AAA_vmsize(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int *vm_put_priority)

{

	/***********************************************************排序部分*****************************************************************/
	/*if (opt_target == 0)
	{
	// 实现排序从大到小
	for (int i = 0; i < num_vm - 1; i++)
	{
	for (int j = i + 1; j < num_vm; j++)
	{
	if (inputFlavor[j].cpu_core_num>inputFlavor[i].cpu_core_num) // 后面cpu大于前面,交换
	{
	Flavor tmp_flavor;
	// 性能表位置交换
	tmp_flavor = inputFlavor[j];
	inputFlavor[j] = inputFlavor[i];
	inputFlavor[i] = tmp_flavor;

	int tmp_require;
	// 需求位置交换
	tmp_require = require_vm[j];
	require_vm[j] = require_vm[i];
	require_vm[i] = tmp_require;
	}
	else if (inputFlavor[j].cpu_core_num == inputFlavor[i].cpu_core_num && inputFlavor[j].mem_size>inputFlavor[i].mem_size) // 后面cpu相等,mem大于前面,交换
	{
	Flavor tmp_flavor;
	// 性能表位置交换
	tmp_flavor = inputFlavor[j];
	inputFlavor[j] = inputFlavor[i];
	inputFlavor[i] = tmp_flavor;

	int tmp_require;
	// 需求位置交换
	tmp_require = require_vm[j];
	require_vm[j] = require_vm[i];
	require_vm[i] = tmp_require;
	}
	}
	}
	}
	else if (opt_target == 1)
	{
	// 实现排序从大到小
	for (int i = 0; i < num_vm - 1; i++)
	{
	for (int j = i + 1; j < num_vm; j++)
	{
	if (inputFlavor[j].mem_size>inputFlavor[i].mem_size) // 后面mem大于前面,交换
	{
	Flavor tmp_flavor;
	// 性能表位置交换
	tmp_flavor = inputFlavor[j];
	inputFlavor[j] = inputFlavor[i];
	inputFlavor[i] = tmp_flavor;

	int tmp_require;
	// 需求位置交换
	tmp_require = require_vm[j];
	require_vm[j] = require_vm[i];
	require_vm[i] = tmp_require;
	}
	else if (inputFlavor[j].mem_size == inputFlavor[i].mem_size && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // mem相等 后面CPU大于前面,交换
	{
	Flavor tmp_flavor;
	// 性能表位置交换
	tmp_flavor = inputFlavor[j];
	inputFlavor[j] = inputFlavor[i];
	inputFlavor[i] = tmp_flavor;

	int tmp_require;
	// 需求位置交换
	tmp_require = require_vm[j];
	require_vm[j] = require_vm[i];
	require_vm[i] = tmp_require;
	}
	}
	}
	}
	else    // 目标选择错误 退出,异常
	{
	return -1;
	}*/

	int *require_vm_temp = new int[num_vm];
	for (int i = 0; i < num_vm; i++)	require_vm_temp[i] = require_vm[vm_put_priority[i]];
	for (int i = 0; i < num_vm; i++)   require_vm[i] = require_vm_temp[i];
	// 需求copy
	int *require_vm_copy = new int[num_vm];
	for (int i = 0; i < num_vm; i++)	require_vm_copy[i] = require_vm[i];

	/********************************************************************放置部分***********************************************************************/
	// 创建服务器余量记录表
	Server *server_remain = new Server[2000];
	// 数据赋值(这里面的2000要和函数外面的max_serve_py对应,函数可以考虑多加一个max_serve_py接口)
	for (int i = 0; i < 2000; i++)
	{
		server_remain[i].cpu_core_num = resource_pm.cpu_core_num;
		server_remain[i].mem_size = resource_pm.mem_size;
	}

	int server_num = 1;
	for (int i = 0; i < num_vm; i++)	// 一种一种放置
	{
		// 当前虚拟机种类的起始搜索索引
		int j = 0;
		while (require_vm_copy[i] != 0)		// 每一种一个一个放置,直到放完
		{
			// 当前此个虚拟机放置标志位
			int flag_put = 0;
			// 遍历搜索
			for (; j < server_num; j++)
			{
				// 不能放下
				if (server_remain[j].cpu_core_num < inputFlavor[i].cpu_core_num || server_remain[j].mem_size < inputFlavor[i].mem_size)		// 任意资源超标
				{
					continue;	// 直接进行下一次的搜索
				}
				else if (server_remain[j].cpu_core_num >= inputFlavor[i].cpu_core_num && server_remain[j].mem_size >= inputFlavor[i].mem_size)	// 保证放下时资源充足,则放下
				{
					server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
					server_remain[j].mem_size -= inputFlavor[i].mem_size;
					flag_put = 1;
					result_save[j*num_vm + i]++;	// 放置记录
					break;	// 搜索到箱子,跳出循环
				}
			}
			// 出上面的for循环有可能是遍历完都放不下,也有可能是放置好了直接跳出
			if (flag_put == 0)
			{
				server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
				server_remain[j].mem_size -= inputFlavor[i].mem_size;
				result_save[j*num_vm + i]++;	// 放置记录
				server_num++;	// 新开一个服务器

			}

			require_vm_copy[i]--; // 此种虚拟机数量减一
		}
	}

	// 进行还原！！！
	/*
	for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
	{
	inputFlavor[i].cpu_core_num = inputFlavor_data[i].cpu_core_num;
	inputFlavor[i].mem_size = inputFlavor_data[i].mem_size;
	inputFlavor[i].flavor_name = inputFlavor_data[i].flavor_name;
	}
	*/

	return server_num;
}


/*  按照  */

// vm_put_priority 放置的优先级！！！

int putVM_greedy_ratio_guided(int *require_vm, int server_num_pre, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int *vm_put_priority)

{
	int *require_vm_temp = new int[inputcontrol.flavorMaxnum];
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)	require_vm_temp[i] = require_vm[vm_put_priority[i]];
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)   require_vm[i] = require_vm_temp[i];
	// 需求copy
	int *require_vm_copy = new int[inputcontrol.flavorMaxnum];
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)	require_vm_copy[i] = require_vm[i];

	/********************************************************************放置部分***********************************************************************/
	// 创建服务器余量记录表
	Server *server_remain = new Server[MAX_SERVER_NUM];
	// 数据赋值(这里面的2000要和函数外面的max_serve_py对应,函数可以考虑多加一个max_serve_py接口)
	for (int i = 0; i < MAX_SERVER_NUM; i++)
	{
		server_remain[i].cpu_core_num = resource_pm.cpu_core_num;
		server_remain[i].mem_size = resource_pm.mem_size;
	}
	int continue_count = 0;
	int flag_count = 0; // 一定小于server_num_pre
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  // 一种一种放置
	{
		for (int j = 0; j < require_vm_copy[i]; j++)  // 每一种虚拟机对应的需求数量
		{
			int flag_put = 0; // 服务器上对应的这台虚拟机是否有空余的位置
			continue_count = 0;
			for (int m = 0; m < server_num_pre; m++)  // 分别按照顺序进行填坑。最多循环的次数！！！
			{
				if (server_remain[(m + flag_count) % server_num_pre].cpu_core_num < inputFlavor[i].cpu_core_num || server_remain[(m + flag_count) % server_num_pre].mem_size < inputFlavor[i].mem_size)		// 任意资源超标
				{
					continue_count++;
					continue;	// 直接进行下一次的搜索

				}
				else
					if (server_remain[(m + flag_count) % server_num_pre].cpu_core_num >= inputFlavor[i].cpu_core_num && server_remain[(m + flag_count) % server_num_pre].mem_size >= inputFlavor[i].mem_size)	// 保证放下时资源充足,则放下
					{
						server_remain[(m + flag_count) % server_num_pre].cpu_core_num -= inputFlavor[i].cpu_core_num;
						server_remain[(m + flag_count) % server_num_pre].mem_size -= inputFlavor[i].mem_size;
						flag_put = 1; //填好了
						flag_count = (flag_count + continue_count) % server_num_pre;
						continue_count = 0;
						result_save[(flag_count++)*inputcontrol.flavorMaxnum + i]++;	// 放置记录
						if (flag_count == server_num_pre)  flag_count = 0;
						//print_data(result_save, server_num_pre);
						//printf("  %d  %d  %d\n", i,j, flag_count);
						break;
					}

			}
			// 如果循环了server_num_pre多次，都没有放置进去。那就再开一个服务器。
			if (flag_put == 0)
			{
				// 开了第j个箱子
				server_remain[server_num_pre].cpu_core_num -= inputFlavor[i].cpu_core_num;
				server_remain[server_num_pre].mem_size -= inputFlavor[i].mem_size;
				result_save[server_num_pre*inputcontrol.flavorMaxnum + i]++;	// 放置记录
				server_num_pre++;	// 新开一个服务器

			}
		}

	}

	return server_num_pre;
}



/*
函数目标:放置虚拟机
方法:	贪心FFD
输入:	*require_vm		虚拟机需求
num_vm			虚拟机类型总数
*inputFlavor	虚拟机性能表
resource_pm		服务器性能结构体
opt_target		优化目标 0:CPU 1:MEM
*result_save	服务器数据表(结果)
输出:	创建的服务器个数
*/
int putVM_greedy_without_seq(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	// 需求copy
	int *require_vm_copy = new int[num_vm];
	for (int i = 0; i < num_vm; i++)
		require_vm_copy[i] = require_vm[i];
	/********************************************************************放置部分***********************************************************************/
	// 创建服务器余量记录表
	Server *server_remain = new Server[2000];
	// 数据赋值(这里面的2000要和函数外面的max_serve_py对应,函数可以考虑多加一个max_serve_py接口)
	for (int i = 0; i < 2000; i++)
	{
		server_remain[i].cpu_core_num = resource_pm.cpu_core_num;
		server_remain[i].mem_size = resource_pm.mem_size;
	}

	int server_num = 1;
	for (int i = 0; i < num_vm; i++)	// 一种一种放置
	{
		// 当前虚拟机种类的起始搜索索引
		int j = 0;
		while (require_vm_copy[i] != 0)		// 每一种一个一个放置,直到放完
		{
			// 当前此个虚拟机放置标志位
			int flag_put = 0;
			// 遍历搜索
			for (; j < server_num; j++)
			{
				// 不能放下
				if (server_remain[j].cpu_core_num < inputFlavor[i].cpu_core_num || server_remain[j].mem_size < inputFlavor[i].mem_size)		// 任意资源超标
				{
					continue;	// 直接进行下一次的搜索
				}
				else if (server_remain[j].cpu_core_num >= inputFlavor[i].cpu_core_num && server_remain[j].mem_size >= inputFlavor[i].mem_size)	// 保证放下时资源充足,则放下
				{
					server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
					server_remain[j].mem_size -= inputFlavor[i].mem_size;
					flag_put = 1;
					result_save[j*num_vm + i]++;	// 放置记录
					break;	// 搜索到箱子,跳出循环
				}
			}
			// 出上面的for循环有可能是遍历完都放不下,也有可能是放置好了直接跳出
			if (flag_put == 0)
			{
				server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
				server_remain[j].mem_size -= inputFlavor[i].mem_size;
				result_save[j*num_vm + i]++;	// 放置记录
				server_num++;	// 新开一个服务器

			}

			require_vm_copy[i]--; // 此种虚拟机数量减一
		}
	}

	// 进行还原！！！
	/*
	for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
	{
	inputFlavor[i].cpu_core_num = inputFlavor_data[i].cpu_core_num;
	inputFlavor[i].mem_size = inputFlavor_data[i].mem_size;
	inputFlavor[i].flavor_name = inputFlavor_data[i].flavor_name;
	}
	*/

	return server_num;
}


/*
函数目标:排序
输入:	*require_vm		虚拟机需求
num_vm			虚拟机类型总数
*inputFlavor	虚拟机性能表
resource_pm		服务器性能结构体
opt_target		优化目标 0:CPU 1:MEM
*result_save	服务器数据表(结果)
输出:	创建的服务器个数
*/
void putVM_seq(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	/***********************************************************排序部分*****************************************************************/
	if (opt_target == 0)
	{
		// 实现排序从大到小
		for (int i = 0; i < num_vm - 1; i++)
		{
			for (int j = i + 1; j < num_vm; j++)
			{
				if (inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // 后面cpu大于前面,交换
				{
					Flavor tmp_flavor;
					// 性能表位置交换
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// 需求位置交换
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
				else if (inputFlavor[j].cpu_core_num == inputFlavor[i].cpu_core_num && inputFlavor[j].mem_size > inputFlavor[i].mem_size) // 后面cpu相等,mem大于前面,交换
				{
					Flavor tmp_flavor;
					// 性能表位置交换
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// 需求位置交换
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
			}
		}
	}
	else if (opt_target == 1)
	{
		// 实现排序从大到小
		for (int i = 0; i < num_vm - 1; i++)
		{
			for (int j = i + 1; j < num_vm; j++)
			{
				if (inputFlavor[j].mem_size > inputFlavor[i].mem_size) // 后面mem大于前面,交换
				{
					Flavor tmp_flavor;
					// 性能表位置交换
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// 需求位置交换
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
				else if (inputFlavor[j].mem_size == inputFlavor[i].mem_size && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // mem相等 后面CPU大于前面,交换
				{
					Flavor tmp_flavor;
					// 性能表位置交换
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// 需求位置交换
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
			}
		}
	}
}

void putVM_seq_vmsize(int *require_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, long *vm_size_data, double *ratio_vm_data, double *ratio_vm_diff_data, int *vm_put_priority, int Flag_put_priority)
{

	print_data((int *)vm_size_data, 1);
	print_data(vm_put_priority, 1);
	/***********************************************************排序部分*****************************************************************/

	// 采用方案一的排序  【vm_size_data块的大小】
	if (Flag_put_priority == 1)
	{
		if (opt_target == 0)
		{
			// 实现排序从大到小
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (vm_size_data[j] > vm_size_data[i]) // 后面cpu大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						/*long temp_vm_size;
						temp_vm_size = vm_size_data[j];
						vm_size_data[j] = vm_size_data[i];
						vm_size_data[i] = temp_vm_size;*/
					}
					else if (vm_size_data[j] == vm_size_data[i] && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // 后面cpu相等,mem大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						/*long temp_vm_size;
						temp_vm_size = vm_size_data[j];
						vm_size_data[j] = vm_size_data[i];
						vm_size_data[i] = temp_vm_size;*/
					}
				}
			}
		}
		else if (opt_target == 1)
		{
			// 实现排序从大到小
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (vm_size_data[j] > vm_size_data[i]) // 后面mem大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						/*long temp_vm_size;
						temp_vm_size = vm_size_data[j];
						vm_size_data[j] = vm_size_data[i];
						vm_size_data[i] = temp_vm_size;*/
					}
					else if (vm_size_data[j] == vm_size_data[i] && inputFlavor[j].mem_size > inputFlavor[i].mem_size) // mem相等 后面CPU大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						/*long temp_vm_size;
						temp_vm_size = vm_size_data[j];
						vm_size_data[j] = vm_size_data[i];
						vm_size_data[i] = temp_vm_size;*/
					}
				}
			}
		}
	}
	// 采用方案二的排序  【虚拟机核数或内存的大小】
	if (Flag_put_priority == 2)
	{
		if (opt_target == 0)
		{
			// 实现排序从大到小
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // 后面cpu大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;
					}
					else if (inputFlavor[j].cpu_core_num == inputFlavor[i].cpu_core_num && inputFlavor[j].mem_size > inputFlavor[i].mem_size) // 后面cpu相等,mem大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;
					}
				}
			}
		}
		else if (opt_target == 1)
		{
			// 实现排序从大到小
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (inputFlavor[j].mem_size > inputFlavor[i].mem_size) // 后面mem大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;
					}
					else if (inputFlavor[j].mem_size == inputFlavor[i].mem_size && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // mem相等 后面CPU大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;
					}
				}
			}
		}
	}
	// 采用方案三的排序  【ratio_vm的大小】
	if (Flag_put_priority == 3)
	{
		if (opt_target == 0)
		{
			// 实现排序从大到小
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (ratio_vm_data[j] > ratio_vm_data[i]) // 后面cpu利用率大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						/*double temp_vm_ratio;
						temp_vm_ratio = ratio_vm_data[j];
						ratio_vm_data[j] = ratio_vm_data[i];
						ratio_vm_data[i] = temp_vm_ratio;*/
					}
					else if (ratio_vm_data[j] == ratio_vm_data[i] && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // 后面cpu相等,mem大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						/*double temp_vm_ratio;
						temp_vm_ratio = ratio_vm_data[j];
						ratio_vm_data[j] = ratio_vm_data[i];
						ratio_vm_data[i] = temp_vm_ratio;*/
					}
				}
			}
		}
		else if (opt_target == 1)
		{
			// 实现排序从大到小
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (ratio_vm_data[j] < ratio_vm_data[i]) // 后面mem大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						/*double temp_vm_ratio;
						temp_vm_ratio = ratio_vm_data[j];
						ratio_vm_data[j] = ratio_vm_data[i];
						ratio_vm_data[i] = temp_vm_ratio;*/
					}
					else if (ratio_vm_data[j] == ratio_vm_data[i] && inputFlavor[j].mem_size > inputFlavor[i].mem_size) // mem相等 后面CPU大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						/*double temp_vm_ratio;
						temp_vm_ratio = ratio_vm_data[j];
						ratio_vm_data[j] = ratio_vm_data[i];
						ratio_vm_data[i] = temp_vm_ratio;*/
					}
				}
			}
		}
	}
	// 采用方案四的排序  【ratio_vm_diff的大小】
	if (Flag_put_priority == 4)
	{
		if (opt_target == 0)
		{
			// 实现排序从大到小
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (ratio_vm_diff_data[j] < ratio_vm_diff_data[i]) // 后面cpu利用率大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						double temp_vm_ratio_diff;
						temp_vm_ratio_diff = ratio_vm_diff_data[j];
						ratio_vm_diff_data[j] = ratio_vm_diff_data[i];
						ratio_vm_diff_data[i] = temp_vm_ratio_diff;

						long temp_vm_size;
						temp_vm_size = vm_size_data[j];
						vm_size_data[j] = vm_size_data[i];
						vm_size_data[i] = temp_vm_size;
					}
					else if (ratio_vm_diff_data[j] == ratio_vm_diff_data[i] && vm_size_data[j] > vm_size_data[i]) // 后面cpu相等,mem大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						double temp_vm_ratio_diff;
						temp_vm_ratio_diff = ratio_vm_diff_data[j];
						ratio_vm_diff_data[j] = ratio_vm_diff_data[i];
						ratio_vm_diff_data[i] = temp_vm_ratio_diff;

						long temp_vm_size;
						temp_vm_size = vm_size_data[j];
						vm_size_data[j] = vm_size_data[i];
						vm_size_data[i] = temp_vm_size;
					}
				}
			}
		}
		else if (opt_target == 1)
		{
			// 实现排序从大到小
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (ratio_vm_diff_data[j] < ratio_vm_diff_data[i]) // 后面mem大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						double temp_vm_ratio_diff;
						temp_vm_ratio_diff = ratio_vm_diff_data[j];
						ratio_vm_diff_data[j] = ratio_vm_diff_data[i];
						ratio_vm_diff_data[i] = temp_vm_ratio_diff;

						long temp_vm_size;
						temp_vm_size = vm_size_data[j];
						vm_size_data[j] = vm_size_data[i];
						vm_size_data[i] = temp_vm_size;
					}
					else if (ratio_vm_diff_data[j] == ratio_vm_diff_data[i] && vm_size_data[j] > vm_size_data[i]) // mem相等 后面CPU大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						double temp_vm_ratio_diff;
						temp_vm_ratio_diff = ratio_vm_diff_data[j];
						ratio_vm_diff_data[j] = ratio_vm_diff_data[i];
						ratio_vm_diff_data[i] = temp_vm_ratio_diff;

						long temp_vm_size;
						temp_vm_size = vm_size_data[j];
						vm_size_data[j] = vm_size_data[i];
						vm_size_data[i] = temp_vm_size;
					}
				}
			}
		}
	}
	// 采用方案五的排序  【vm_size_data块的大小】
	if (Flag_put_priority == 5)
	{
		if (opt_target == 0)
		{
			// 实现排序从大到小
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (vm_size_data[j] > vm_size_data[i]) // 后面cpu大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						/*long temp_vm_size;
						temp_vm_size = vm_size_data[j];
						vm_size_data[j] = vm_size_data[i];
						vm_size_data[i] = temp_vm_size;*/
					}
					else if (vm_size_data[j] == vm_size_data[i] && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // 后面cpu相等,mem大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						/*long temp_vm_size;
						temp_vm_size = vm_size_data[j];
						vm_size_data[j] = vm_size_data[i];
						vm_size_data[i] = temp_vm_size;*/
					}
				}
			}
		}
		else if (opt_target == 1)
		{
			// 实现排序从大到小
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (vm_size_data[j] > vm_size_data[i]) // 后面mem大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						/*long temp_vm_size;
						temp_vm_size = vm_size_data[j];
						vm_size_data[j] = vm_size_data[i];
						vm_size_data[i] = temp_vm_size;*/
					}
					else if (vm_size_data[j] == vm_size_data[i] && inputFlavor[j].mem_size > inputFlavor[i].mem_size) // mem相等 后面CPU大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						/*long temp_vm_size;
						temp_vm_size = vm_size_data[j];
						vm_size_data[j] = vm_size_data[i];
						vm_size_data[i] = temp_vm_size;*/
					}
				}
			}
		}
	}
	//print_data(require_vm, 1);
	print_data((int *)vm_size_data, 1);
	print_data(vm_put_priority, 1);
}


void putVM_seq_ratio_guided(int *require_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, double *ratio_vm_data, int *vm_put_priority, int Flag_put_priority)
{

	for (int i = 0; i < inputcontrol.flavorMaxnum; i++) printf(" %f  ", ratio_vm_data[i]);
	print_data(vm_put_priority, 1);
	/***********************************************************排序部分*****************************************************************/

	if (Flag_put_priority == 2)
	{
		if (opt_target == 0)
		{
			// 实现排序从大到小
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // 后面cpu大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;
					}
					else if (inputFlavor[j].cpu_core_num == inputFlavor[i].cpu_core_num && inputFlavor[j].mem_size > inputFlavor[i].mem_size) // 后面cpu相等,mem大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;
					}
				}
			}
		}
		else if (opt_target == 1)
		{
			// 实现排序从大到小
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (inputFlavor[j].mem_size > inputFlavor[i].mem_size) // 后面mem大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;
					}
					else if (inputFlavor[j].mem_size == inputFlavor[i].mem_size && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // mem相等 后面CPU大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;
					}
				}
			}
		}
	}
	// 采用方案三的排序  【ratio_vm的大小】
	if (Flag_put_priority != 2)
	{
		if (opt_target == 0)
		{
			// 实现排序从大到小
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (ratio_vm_data[j] < ratio_vm_data[i]) // 后面cpu利用率大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						double temp_vm_ratio;
						temp_vm_ratio = ratio_vm_data[j];
						ratio_vm_data[j] = ratio_vm_data[i];
						ratio_vm_data[i] = temp_vm_ratio;
					}
					else if (ratio_vm_data[j] == ratio_vm_data[i] && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // 后面cpu相等,mem大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						double temp_vm_ratio;
						temp_vm_ratio = ratio_vm_data[j];
						ratio_vm_data[j] = ratio_vm_data[i];
						ratio_vm_data[i] = temp_vm_ratio;
					}
				}
			}
		}
		else if (opt_target == 1)
		{
			// 实现排序从大到小
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (ratio_vm_data[j] > ratio_vm_data[i]) // 后面mem大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						double temp_vm_ratio;
						temp_vm_ratio = ratio_vm_data[j];
						ratio_vm_data[j] = ratio_vm_data[i];
						ratio_vm_data[i] = temp_vm_ratio;
					}
					else if (ratio_vm_data[j] == ratio_vm_data[i] && inputFlavor[j].mem_size > inputFlavor[i].mem_size) // mem相等 后面CPU大于前面,交换
					{
						Flavor tmp_flavor;
						// 性能表位置交换
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// 需求位置交换
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;

						double temp_vm_ratio;
						temp_vm_ratio = ratio_vm_data[j];
						ratio_vm_data[j] = ratio_vm_data[i];
						ratio_vm_data[i] = temp_vm_ratio;
					}
				}
			}
		}
	}

	for (int i = 0; i < inputcontrol.flavorMaxnum; i++) printf(" %f  ", ratio_vm_data[i]);
	print_data(vm_put_priority, 1);
}


/*
放置算法:顺序放置
输入:require_vm 各个虚拟机需求量
num_vm 虚拟机种类数
resource_pm 硬件服务器性能
opt_target 优化资源 cpu或mem
inputFlavor 各个虚拟机的性能 name 和 cup 和 mem
result_save 存储输出数据的地址
*/
int putVM_liner(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save) {
	int i, j;
	int resource_cpu_remain_now;  //当前服务器剩余CPU资源
	int resource_mem_remain_now; //当前服务器剩余MEM资源
	int pm_num = 0;
	//初始化物理服务器资源
	resource_cpu_remain_now = resource_pm.cpu_core_num;
	resource_mem_remain_now = resource_pm.mem_size << 3;

	for (i = 0; i < num_vm; i++) {
		for (j = 0; j < require_vm[i]; j++) {
			if (resource_cpu_remain_now >= inputFlavor[i].cpu_core_num
				&& resource_mem_remain_now >= inputFlavor[i].mem_size) { //物理资源足够
				resource_cpu_remain_now = resource_cpu_remain_now - inputFlavor[i].cpu_core_num;
				resource_mem_remain_now = resource_mem_remain_now - inputFlavor[i].mem_size;
				result_save[pm_num*num_vm + i]++;
			}
			else { //物理资源不足
				pm_num++;
				resource_cpu_remain_now = resource_pm.cpu_core_num;
				resource_mem_remain_now = resource_pm.mem_size << 3;
				resource_cpu_remain_now = resource_cpu_remain_now - inputFlavor[i].cpu_core_num;
				resource_mem_remain_now = resource_mem_remain_now - inputFlavor[i].mem_size;
				result_save[pm_num*num_vm + i]++;
			}
		}
	}

	return (pm_num + 1);
}


/*
函数目标:放置虚拟机
方法:	动态规划
输入:
*require_vm		虚拟机需求
num_of_total_vm  虚拟机需求总数
num_vm			虚拟机类型总数
*inputFlavor	虚拟机性能表
resource_pm		服务器性能结构体
opt_target		优化目标 0:CPU 1:MEM
*result_save	服务器数据表(结果)
输出:	创建的服务器个数
*/
int putVM_dynamicpro(int *require_vm, int num_of_total_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save) {
	int server_num = 0;  //物理机数量
	int *require_vm_copy = new int[num_vm];
	for (int i = 0; i < num_vm; i++) {
		require_vm_copy[i] = require_vm[i];
	}
	while (1) {
		/*整理格式，将所有虚拟机列成一行*/
		int num_of_total_vm_point = 1; //预测的虚拟机计数器
		num_of_total_vm = 0;//统计虚拟机数量
		for (int i = 0; i < num_vm; i++) {
			for (int j = 0; j < require_vm_copy[i]; j++) {
				num_of_total_vm++;
			}
		}
		Flavor *list_danmicpro = new Flavor[num_of_total_vm + 1]; //动态规划表，将需求虚拟机 一个一个放进去,数组大小为虚拟机总数+1
		Flavor *list_danmicpro_ori = list_danmicpro;  //保存原始指针
		list_danmicpro[0].cpu_core_num = 0;
		list_danmicpro[0].mem_size = 0;
		for (int i = 0; i < num_vm; i++) {
			for (int j = 0; j < require_vm_copy[i]; j++) {
				list_danmicpro[num_of_total_vm_point] = inputFlavor[i];
				num_of_total_vm_point++;
				//printf("\n");
				//printf("虚拟机%d  cpu: %d   mem: %d", num_of_total_vm_point - 1, list_danmicpro[num_of_total_vm_point - 1].cpu_core_num, list_danmicpro[num_of_total_vm_point - 1].mem_size);
			}
		}
		/*动态规划初始化*/
		int restict_cpu = resource_pm.cpu_core_num; //物理机cpu总量
		int restict_mem = resource_pm.mem_size;   //物理机mem总量
		int *f = new int[(restict_cpu + 1)*(restict_mem + 1)];
		char *chose_or_no = new char[(num_of_total_vm_point + 1)*(restict_cpu + 1)*(restict_mem + 1)];
		int *f_ori = f;   //保存原始指针
		char *chose_or_no_ori = chose_or_no;   //保存原始指针
		for (int i = 0; i < (num_of_total_vm_point + 1)*(restict_cpu + 1)*(restict_mem + 1); i++) {
			chose_or_no[i] = 0;
		}
		for (int i = 0; i < (restict_cpu + 1)*(restict_mem + 1); i++) {
			f[i] = 0;
		}
		/*规划过程*/
		for (int i = 1; i <= num_of_total_vm; i++) {
			for (int j = restict_cpu; j >= list_danmicpro[i].cpu_core_num; j--) {
				for (int k = restict_mem; k >= list_danmicpro[i].mem_size; k--) {
					int value1, value2;
					if (opt_target == 0) {  //优化cpu
						value1 = f[(j - list_danmicpro[i].cpu_core_num)*(restict_mem + 1) + (k - list_danmicpro[i].mem_size)] + list_danmicpro[i].cpu_core_num;
					}
					else {   //优化内存
						value1 = f[(j - list_danmicpro[i].cpu_core_num)*(restict_mem + 1) + (k - list_danmicpro[i].mem_size)] + list_danmicpro[i].mem_size;
					}
					value2 = f[j*(restict_mem + 1) + k];
					if (value1 > value2) {   //取这一个
						f[j*(restict_mem + 1) + k] = value1;
						chose_or_no[i*(restict_cpu + 1)*(restict_mem + 1) + j * (restict_mem + 1) + k] = 1;
					}
					else {      //不取这一个
						f[j*(restict_mem + 1) + k] = value2;
						chose_or_no[i*(restict_cpu + 1)*(restict_mem + 1) + j * (restict_mem + 1) + k] = 0;
					}
				}
			}
		}
		/*总结选取*/
		int out_put[10000] = { 0 };  //选择哪几个虚拟机放入，最多10000个虚拟机
		int CPU_temp = resource_pm.cpu_core_num;
		int MEM_temp = resource_pm.mem_size;
		for (int i = num_of_total_vm_point; i > 0; i--) {
			if (chose_or_no[i*(restict_cpu + 1)*(restict_mem + 1) + CPU_temp * (restict_mem + 1) + MEM_temp] == 1) {
				out_put[i] = 1;
				CPU_temp -= list_danmicpro[i].cpu_core_num;
				MEM_temp -= list_danmicpro[i].mem_size;
			}
			else {
				out_put[i] = 0;
			}
		}
		int xiaolv = f[restict_cpu*(restict_mem + 1) + restict_mem];
		//printf("\n%d\n", f[restict_cpu*(restict_mem + 1) + restict_mem]);
		//printf("取或不取：\n");
		for (int j = 0; j < num_of_total_vm_point; j++) {
			//printf("%d ", out_put[j]);
		}
		//printf("\n虚拟性能：\n");
		for (int j = 0; j < num_of_total_vm_point; j++) {
			//printf("%d ", list_danmicpro[j].cpu_core_num);
		}

		/*一轮动态规划结束，输出放入result_save*/
		for (int i = 0; i < num_of_total_vm_point; i++) {
			if (out_put[i] == 1) {
				for (int j = 0; j < num_vm; j++) {
					if (list_danmicpro[i].flavor_name == inputFlavor[j].flavor_name) { //匹配名字
						require_vm_copy[j]--; //当前虚拟机需求减
						result_save[server_num*num_vm + j]++;
						break;
					}
				}
			}
		}
		/*决定是否开启新服务器*/
		char creat_new_py_flag = 0; //决定是否开启新服务器
		for (int i = 0; i < num_vm; i++) {
			if (require_vm_copy[i] != 0) {
				creat_new_py_flag = 1;
				break;
			}
		}
		/*释放堆内存*/
		delete[] list_danmicpro_ori;
		delete[] chose_or_no_ori;
		delete[] f_ori;
		/*开启新服务器*/
		if (creat_new_py_flag == 1) {  //还有虚拟机没有分配完
			server_num++;
		}
		else {  //分配结束，break出while
			break;
		}
	}

	return server_num + 1;
}




int putVM_dynamicpro_ratio_guided(int *require_vm, int num_of_total_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int *vm_put_priority) {


	int *require_vm_temp = new int[inputcontrol.flavorMaxnum];
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)	require_vm_temp[i] = require_vm[vm_put_priority[i]];
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)   require_vm[i] = require_vm_temp[i];


	int server_num = 0;  //物理机数量
	int *require_vm_copy = new int[num_vm];
	for (int i = 0; i < num_vm; i++) {
		require_vm_copy[i] = require_vm[i];
	}
	while (1) {
		/*整理格式，将所有虚拟机列成一行*/
		int num_of_total_vm_point = 1; //预测的虚拟机计数器
		num_of_total_vm = 0;//统计虚拟机数量
		for (int i = 0; i < num_vm; i++) {
			for (int j = 0; j < require_vm_copy[i]; j++) {
				num_of_total_vm++;
			}
		}
		Flavor *list_danmicpro = new Flavor[num_of_total_vm + 1]; //动态规划表，将需求虚拟机 一个一个放进去,数组大小为虚拟机总数+1
		Flavor *list_danmicpro_ori = list_danmicpro;  //保存原始指针
		list_danmicpro[0].cpu_core_num = 0;
		list_danmicpro[0].mem_size = 0;
		for (int i = 0; i < num_vm; i++) {
			for (int j = 0; j < require_vm_copy[i]; j++) {
				list_danmicpro[num_of_total_vm_point] = inputFlavor[i];
				num_of_total_vm_point++;
				//printf("\n");
				//printf("虚拟机%d  cpu: %d   mem: %d", num_of_total_vm_point - 1, list_danmicpro[num_of_total_vm_point - 1].cpu_core_num, list_danmicpro[num_of_total_vm_point - 1].mem_size);
			}
		}
		/*动态规划初始化*/
		int restict_cpu = resource_pm.cpu_core_num; //物理机cpu总量
		int restict_mem = resource_pm.mem_size;   //物理机mem总量
		int *f = new int[(restict_cpu + 1)*(restict_mem + 1)];
		char *chose_or_no = new char[(num_of_total_vm_point + 1)*(restict_cpu + 1)*(restict_mem + 1)];
		int *f_ori = f;   //保存原始指针
		char *chose_or_no_ori = chose_or_no;   //保存原始指针
		for (int i = 0; i < (num_of_total_vm_point + 1)*(restict_cpu + 1)*(restict_mem + 1); i++) {
			chose_or_no[i] = 0;
		}
		for (int i = 0; i < (restict_cpu + 1)*(restict_mem + 1); i++) {
			f[i] = 0;
		}
		/*规划过程*/
		for (int i = 1; i <= num_of_total_vm; i++) {
			for (int j = restict_cpu; j >= list_danmicpro[i].cpu_core_num; j--) {
				for (int k = restict_mem; k >= list_danmicpro[i].mem_size; k--) {
					int value1, value2;
					if (opt_target == 0) {  //优化cpu
						value1 = f[(j - list_danmicpro[i].cpu_core_num)*(restict_mem + 1) + (k - list_danmicpro[i].mem_size)] + list_danmicpro[i].cpu_core_num;
					}
					else {   //优化内存
						value1 = f[(j - list_danmicpro[i].cpu_core_num)*(restict_mem + 1) + (k - list_danmicpro[i].mem_size)] + list_danmicpro[i].mem_size;
					}
					value2 = f[j*(restict_mem + 1) + k];
					if (value1 > value2) {   //取这一个
						f[j*(restict_mem + 1) + k] = value1;
						chose_or_no[i*(restict_cpu + 1)*(restict_mem + 1) + j * (restict_mem + 1) + k] = 1;
					}
					else {      //不取这一个
						f[j*(restict_mem + 1) + k] = value2;
						chose_or_no[i*(restict_cpu + 1)*(restict_mem + 1) + j * (restict_mem + 1) + k] = 0;
					}
				}
			}
		}
		/*总结选取*/
		int out_put[10000] = { 0 };  //选择哪几个虚拟机放入，最多10000个虚拟机
		int CPU_temp = resource_pm.cpu_core_num;
		int MEM_temp = resource_pm.mem_size;
		for (int i = num_of_total_vm_point; i > 0; i--) {
			if (chose_or_no[i*(restict_cpu + 1)*(restict_mem + 1) + CPU_temp * (restict_mem + 1) + MEM_temp] == 1) {
				out_put[i] = 1;
				CPU_temp -= list_danmicpro[i].cpu_core_num;
				MEM_temp -= list_danmicpro[i].mem_size;
			}
			else {
				out_put[i] = 0;
			}
		}
		int xiaolv = f[restict_cpu*(restict_mem + 1) + restict_mem];
		//printf("\n%d\n", f[restict_cpu*(restict_mem + 1) + restict_mem]);
		//printf("取或不取：\n");
		for (int j = 0; j < num_of_total_vm_point; j++) {
			//printf("%d ", out_put[j]);
		}
		//printf("\n虚拟性能：\n");
		for (int j = 0; j < num_of_total_vm_point; j++) {
			//printf("%d ", list_danmicpro[j].cpu_core_num);
		}

		/*一轮动态规划结束，输出放入result_save*/
		for (int i = 0; i < num_of_total_vm_point; i++) {
			if (out_put[i] == 1) {
				for (int j = 0; j < num_vm; j++) {
					if (list_danmicpro[i].flavor_name == inputFlavor[j].flavor_name) { //匹配名字
						require_vm_copy[j]--; //当前虚拟机需求减
						result_save[server_num*num_vm + j]++;
						break;
					}
				}
			}
		}
		/*决定是否开启新服务器*/
		char creat_new_py_flag = 0; //决定是否开启新服务器
		for (int i = 0; i < num_vm; i++) {
			if (require_vm_copy[i] != 0) {
				creat_new_py_flag = 1;
				break;
			}
		}
		/*释放堆内存*/
		delete[] list_danmicpro_ori;
		delete[] chose_or_no_ori;
		delete[] f_ori;
		/*开启新服务器*/
		if (creat_new_py_flag == 1) {  //还有虚拟机没有分配完
			server_num++;
		}
		else {  //分配结束，break出while
			break;
		}
	}

	return server_num + 1;
}




void print_resource(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int num_py) {
	int i, j;
	/*资源初始化*/
	double *py_ratio_cpu = new double[num_py];  //一个物理机的cpu资源利用率
	double *py_ratio_mem = new double[num_py];  //一个物理机的内存资源利用率 
	int *resource_rest_cpu = new int[num_py];   //一个物理机的cpu剩余资源
	int *resource_rest_mem = new int[num_py];   //一个物理机的内存剩余资源
	int *flavor_num_in_py = new int[num_py];    //一个物理机中包含的虚拟机个数
	int flavor_totol_mun = 0;  //需求虚拟机的总量
	int num_py_new = num_py; //新的物理机数量
	for (i = 0; i < num_py; i++) {
		resource_rest_cpu[i] = resource_pm.cpu_core_num;
		resource_rest_mem[i] = resource_pm.mem_size;
		flavor_num_in_py[i] = 0;
	}
	for (i = 0; i < num_vm; i++) {
		flavor_totol_mun += require_vm[i];
	}


	printf("\n\n");
	printf("矫正后\n\n");
	for (i = 0; i < num_vm; i++) {
		printf("名称：flavor%d  ", inputFlavor[i].flavor_name);
		printf("内存：%d  ", inputFlavor[i].mem_size);
		printf("CPU： %d  ", inputFlavor[i].cpu_core_num);
		printf("需求：%d  ", require_vm[i]);
		printf("\n");
	}


	/*计算每个硬件服务器利用率*/
	for (i = 0; i < num_py; i++) {
		// 第i个物理机的利用率和剩余资源 
		int sum_temp_cpu = 0;
		int sum_temp_mem = 0;
		for (j = 0; j < num_vm; j++) {
			if (result_save[i*num_vm + j] != 0) {
				sum_temp_cpu += result_save[i*num_vm + j] * inputFlavor[j].cpu_core_num;
				sum_temp_mem += result_save[i*num_vm + j] * inputFlavor[j].mem_size;
				flavor_num_in_py[i] += result_save[i*num_vm + j];
			}
		}
		py_ratio_cpu[i] = (double)sum_temp_cpu / (double)resource_pm.cpu_core_num;
		py_ratio_mem[i] = (double)sum_temp_mem / (double)resource_pm.mem_size;
		resource_rest_cpu[i] -= sum_temp_cpu;
		resource_rest_mem[i] -= sum_temp_mem;
	}
	printf("\n");
	for (i = 0; i < num_py; i++) {
		printf("\n CPU%d利用率：%f  还剩资源：%d", i, py_ratio_cpu[i], resource_rest_cpu[i]);
	}
	printf("\n");
	for (i = 0; i < num_py; i++) {
		printf("\n MEM%d利用率：%f  还剩资源：%d", i, py_ratio_mem[i], resource_rest_mem[i]);
	}

	double ratio_cpu = 0;
	double ratio_mem = 0;
	for (i = 0; i<num_py; i++) {
		ratio_cpu += py_ratio_cpu[i];
		ratio_mem += py_ratio_mem[i];
	}
	ratio_cpu /= num_py;
	ratio_mem /= num_py;
	printf("\n");
	printf("\n CPU总利用率：%f", ratio_cpu);
	printf("\n MEM总利用率：%f", ratio_mem);

	/*if (opt_target == 0) {
	if (ratio_cpu<0.8) {
	while (1) {

	}
	}
	}
	else {
	if (ratio_mem<0.8) {
	while (1) {

	}
	}
	}*/


}


/*
函数目标:对放置方法进行矫正
方法:	尽量保证100%利用率
输入:	*require_vm		虚拟机需求
num_vm			虚拟机类型总数
*inputFlavor	虚拟机性能表
resource_pm		服务器性能结构体
opt_target		优化目标 0:CPU 1:MEM
*result_save	服务器数据表(结果)
num_py      	创建的服务器个数
输出:   矫正后服务器个数
*/
int putVM_correct(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int num_py) {
	int i, j;
	/*资源初始化*/
	double *py_ratio_cpu = new double[num_py];  //一个物理机的cpu资源利用率
	double *py_ratio_mem = new double[num_py];  //一个物理机的内存资源利用率 
	int *resource_rest_cpu = new int[num_py];   //一个物理机的cpu剩余资源
	int *resource_rest_mem = new int[num_py];   //一个物理机的内存剩余资源
	int *flavor_num_in_py = new int[num_py];    //一个物理机中包含的虚拟机个数
	int flavor_totol_mun = 0;  //需求虚拟机的总量
	int num_py_new = num_py; //新的物理机数量
	for (i = 0; i < num_py; i++) {
		resource_rest_cpu[i] = resource_pm.cpu_core_num;
		resource_rest_mem[i] = resource_pm.mem_size;
		flavor_num_in_py[i] = 0;
	}
	for (i = 0; i < num_vm; i++) {
		flavor_totol_mun += require_vm[i];
	}
	/*计算每个硬件服务器利用率*/
	for (i = 0; i < num_py; i++) {
		// 第i个物理机的利用率和剩余资源 
		int sum_temp_cpu = 0;
		int sum_temp_mem = 0;
		for (j = 0; j < num_vm; j++) {
			if (result_save[i*num_vm + j] != 0) {
				sum_temp_cpu += result_save[i*num_vm + j] * inputFlavor[j].cpu_core_num;
				sum_temp_mem += result_save[i*num_vm + j] * inputFlavor[j].mem_size;
				flavor_num_in_py[i] += result_save[i*num_vm + j];
			}
		}
		py_ratio_cpu[i] = (double)sum_temp_cpu / (double)resource_pm.cpu_core_num;
		py_ratio_mem[i] = (double)sum_temp_mem / (double)resource_pm.mem_size;
		resource_rest_cpu[i] -= sum_temp_cpu;
		resource_rest_mem[i] -= sum_temp_mem;
	}
	//printf("\n");
	for (i = 0; i < num_py; i++) {
		//printf("\n CPU%d利用率：%f  还剩资源：%d", i, py_ratio_cpu[i], resource_rest_cpu[i]);
	}
	//printf("\n");
	for (i = 0; i < num_py; i++) {
		//printf("\n MEM%d利用率：%f  还剩资源：%d", i, py_ratio_mem[i], resource_rest_mem[i]);
	}


	/*开始矫正*/
	int opt_sub_timer;  //减优化的虚拟机个数
	int opt_add_timer;  //加优化的虚拟机个数
	opt_sub_timer = flavor_num_in_py[num_py - 1];
	opt_add_timer = 0;
	int *require_vm_copy = new int[num_vm];
	int *result_save_copy = new int[num_vm*num_py];
	for (i = 0; i < num_py; i++) {
		for (j = 0; j < num_vm; j++) {
			result_save_copy[i*num_vm + j] = result_save[i*num_vm + j];
		}
	}
	for (i = 0; i < num_vm; i++) {
		require_vm_copy[i] = require_vm[i];
	}

	//i = num_py - 1;  //只考虑最后一个硬件服务器 
	i = 0;
	while (i < num_py) {
		opt_sub_timer = flavor_num_in_py[i];
		if (resource_rest_cpu[i] != 0 && resource_rest_mem[i] != 0)
		{ //剩余资源不为0，才可矫正
			for (j = 0; j < num_vm; j++) {
				while (inputFlavor[j].cpu_core_num <= resource_rest_cpu[i] && inputFlavor[j].mem_size <= resource_rest_mem[i]) {
					require_vm_copy[j]++;
					result_save_copy[i*num_vm + j]++;
					opt_add_timer++;
					resource_rest_cpu[i] -= inputFlavor[j].cpu_core_num;
					resource_rest_mem[i] -= inputFlavor[j].mem_size;
				}
			}
			if (opt_add_timer <= opt_sub_timer && opt_add_timer != 0) { //选择 加虚拟机优化
				for (int kk = 0; kk < num_py; kk++) {
					for (j = 0; j < num_vm; j++) {
						result_save[kk*num_vm + j] = result_save_copy[kk*num_vm + j];
					}
				}
				for (int kk = 0; kk < num_vm; kk++) {
					require_vm[kk] = require_vm_copy[kk];
				}
			}
			else {  //选择 减虚拟机优化
				for (j = 0; j < num_vm; j++) {
					if (result_save[i*num_vm + j] != 0) {
						require_vm[j] -= result_save[i*num_vm + j];
						result_save[i*num_vm + j] = 0;
					}
				}
				for (int kk = i; kk < num_py_new; kk++) {
					for (j = 1; j < num_vm; j++) {
						result_save[i*num_vm + j] = result_save[(i + 1)*num_vm + j];
					}
				}
				num_py_new--;
				continue;
			}
		}
		i++;
	}

	return num_py_new;
}

int putVM_correct_vmsize(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int num_py) {
	int i, j;
	/*资源初始化*/
	double *py_ratio_cpu = new double[num_py];  //一个物理机的cpu资源利用率
	double *py_ratio_mem = new double[num_py];  //一个物理机的内存资源利用率 
	int *resource_rest_cpu = new int[num_py];   //一个物理机的cpu剩余资源
	int *resource_rest_mem = new int[num_py];   //一个物理机的内存剩余资源
	int *flavor_num_in_py = new int[num_py];    //一个物理机中包含的虚拟机个数
	int flavor_totol_mun = 0;  //需求虚拟机的总量
	int num_py_new = num_py; //新的物理机数量
	for (i = 0; i < num_py; i++) {
		resource_rest_cpu[i] = resource_pm.cpu_core_num;
		resource_rest_mem[i] = resource_pm.mem_size;
		flavor_num_in_py[i] = 0;
	}
	for (i = 0; i < num_vm; i++) {
		flavor_totol_mun += require_vm[i];
	}
	/*计算每个硬件服务器利用率*/
	for (i = 0; i < num_py; i++) {
		// 第i个物理机的利用率和剩余资源 
		int sum_temp_cpu = 0;
		int sum_temp_mem = 0;
		for (j = 0; j < num_vm; j++) {
			if (result_save[i*num_vm + j] != 0) {
				sum_temp_cpu += result_save[i*num_vm + j] * inputFlavor[j].cpu_core_num;
				sum_temp_mem += result_save[i*num_vm + j] * inputFlavor[j].mem_size;
				flavor_num_in_py[i] += result_save[i*num_vm + j];
			}
		}
		py_ratio_cpu[i] = (double)sum_temp_cpu / (double)resource_pm.cpu_core_num;
		py_ratio_mem[i] = (double)sum_temp_mem / (double)resource_pm.mem_size;
		resource_rest_cpu[i] -= sum_temp_cpu;
		resource_rest_mem[i] -= sum_temp_mem;
	}
	//printf("\n");
	for (i = 0; i < num_py; i++) {
		//printf("\n CPU%d利用率：%f  还剩资源：%d", i, py_ratio_cpu[i], resource_rest_cpu[i]);
	}
	//printf("\n");
	for (i = 0; i < num_py; i++) {
		//printf("\n MEM%d利用率：%f  还剩资源：%d", i, py_ratio_mem[i], resource_rest_mem[i]);
	}


	/*开始矫正*/
	int opt_sub_timer;  //减优化的虚拟机个数
	int opt_add_timer;  //加优化的虚拟机个数
	opt_sub_timer = flavor_num_in_py[num_py - 1];
	opt_add_timer = 0;
	int *require_vm_copy = new int[num_vm];
	int *result_save_copy = new int[num_vm*num_py];
	for (i = 0; i < num_py; i++) {
		for (j = 0; j < num_vm; j++) {
			result_save_copy[i*num_vm + j] = result_save[i*num_vm + j];
		}
	}
	for (i = 0; i < num_vm; i++) {
		require_vm_copy[i] = require_vm[i];
	}

	//i = num_py - 1;  //只考虑最后一个硬件服务器 
	i = 0;
	while (i < num_py) {
		opt_sub_timer = flavor_num_in_py[i];
		if (resource_rest_cpu[i] != 0 && resource_rest_mem[i] != 0)
		{ //剩余资源不为0，才可矫正
			for (j = 0; j < num_vm; j++) {
				while (inputFlavor[j].cpu_core_num <= resource_rest_cpu[i] && inputFlavor[j].mem_size <= resource_rest_mem[i]) {
					require_vm_copy[j]++;
					result_save_copy[i*num_vm + j]++;
					opt_add_timer++;
					resource_rest_cpu[i] -= inputFlavor[j].cpu_core_num;
					resource_rest_mem[i] -= inputFlavor[j].mem_size;
				}
			}
			if (opt_add_timer <= opt_sub_timer && opt_add_timer != 0) { //选择 加虚拟机优化
				for (int kk = 0; kk < num_py; kk++) {
					for (j = 0; j < num_vm; j++) {
						result_save[kk*num_vm + j] = result_save_copy[kk*num_vm + j];
					}
				}
				for (int kk = 0; kk < num_vm; kk++) {
					require_vm[kk] = require_vm_copy[kk];
				}
			}
			else {  //选择 减虚拟机优化
				for (j = 0; j < num_vm; j++) {
					if (result_save[i*num_vm + j] != 0) {
						require_vm[j] -= result_save[i*num_vm + j];
						result_save[i*num_vm + j] = 0;
					}
				}
				for (int kk = i; kk < num_py_new; kk++) {
					for (j = 1; j < num_vm; j++) {
						result_save[i*num_vm + j] = result_save[(i + 1)*num_vm + j];
					}
				}
				num_py_new--;
				continue;
			}
		}
		i++;
	}

	return num_py_new;
}



/*
函数目标:种群使用贪心竞争策略
*/
int putVM_compet_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	// 扩大 require_vm 样本的种群！！！
	int population_num = 16;

	// 增幅！！！
	int resize_begin = -2;
	int resize_end = 1;



	// 一组解到多组解的演变
	int **result_predict_super = new int *[population_num];
	for (int i = 0; i < population_num; i++)    result_predict_super[i] = new int[inputcontrol.flavorMaxnum];
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			result_predict_super[i][j] = require_vm[j];
		}
	}
	//找见最大的
	int max_count_a_num = 0;
	int max_count_b_num = 0;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_a_num] <= require_vm[i])  max_count_a_num = i;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_b_num] <= require_vm[i] && i != max_count_a_num)  max_count_b_num = i;

	for (int i = resize_begin; i <= resize_end; i++)
	{
		for (int j = resize_begin; j <= resize_end; j++)
		{
			//printf(" %d ", result_predict_super[(i + 1) * 4 + (j + 1)][max_count_a_num]);
			//printf(" %d \n", result_predict_super[(i + 1) * 4 + (j + 1)][max_count_b_num]);
			result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_a_num] += i;
			result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_b_num] += j;
			printf("case %d :  %d ", (i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin), result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_a_num]);
			printf(" %d \n", result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_b_num]);
		}
	}
	for (int i = 0; i < population_num; i++)
	{
		print_data(result_predict_super[i], 1);
	}

	// 针对多组解的多种放置方案！！！
	int max_serve_py = 2000;  // 服务器最大的个数!!!
	int **result_save_super = new int *[population_num];
	for (int i = 0; i < population_num; i++)    result_save_super[i] = new int[max_serve_py*inputcontrol.flavorMaxnum];
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < max_serve_py*inputcontrol.flavorMaxnum; j++)
		{
			result_save_super[i][j] = 0;
		}
	}

	int *result_serve_num = new int[population_num];
	Flavor *inputFlavor_data = new Flavor[inputcontrol.flavorMaxnum];

	for (int i = 0; i < population_num; i++)
	{
		if (i == 0)
		{
			for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
			{
				inputFlavor_data[i].cpu_core_num = inputFlavor[i].cpu_core_num;
				inputFlavor_data[i].mem_size = inputFlavor[i].mem_size;
				inputFlavor_data[i].flavor_name = inputFlavor[i].flavor_name;
			}
		}

		print_data(result_predict_super[i], 1);
		result_serve_num[i] = putVM_greedy(result_predict_super[i], inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i]);
		print_data(result_predict_super[i], 1);

		if (i != population_num - 1)
			for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
			{
				inputFlavor[i].cpu_core_num = inputFlavor_data[i].cpu_core_num;
				inputFlavor[i].mem_size = inputFlavor_data[i].mem_size;
				inputFlavor[i].flavor_name = inputFlavor_data[i].flavor_name;
			}

		// print_data(result_save_super[i], result_serve_num[i]);
		printf("case %d：result_serve_num: %d\n", i + 1, result_serve_num[i]);
	}



	double **py_ratio_cpu = new double *[population_num];  // 一个物理机的cpu资源利用率
	double **py_ratio_mem = new double *[population_num];  // 一个物理机的内存资源利用率 

	double *py_ratio_cpu_final = new double[population_num];  //
	double *py_ratio_mem_final = new double[population_num];  // 

	for (int i = 0; i < population_num; i++)
	{
		py_ratio_cpu[i] = new double[result_serve_num[i]];
		py_ratio_mem[i] = new double[result_serve_num[i]];

		for (int j = 0; j < result_serve_num[i]; j++)
		{
			// 第j个物理机的利用率和剩余资源 
			int sum_temp_cpu = 0;
			int sum_temp_mem = 0;
			for (int k = 0; k < num_vm; k++)
			{
				if (result_save_super[i][j*num_vm + k] != 0)
				{
					sum_temp_cpu += result_save_super[i][j*num_vm + k] * inputFlavor[k].cpu_core_num;
					sum_temp_mem += result_save_super[i][j*num_vm + k] * inputFlavor[k].mem_size;
				}
			}
			// 服务器上的单个利用率
			py_ratio_cpu[i][j] = (double)sum_temp_cpu / (double)resource_pm.cpu_core_num;
			py_ratio_mem[i][j] = (double)sum_temp_mem / (double)resource_pm.mem_size;
		}

		double sum_ratio_cpu = 0;
		double sum_ratio_mem = 0;
		for (int m = 0; m < result_serve_num[i]; m++)
		{
			sum_ratio_cpu += py_ratio_cpu[i][m];
			sum_ratio_mem += py_ratio_mem[i][m];
		}

		// 最终的利用率
		py_ratio_cpu_final[i] = sum_ratio_cpu / result_serve_num[i];
		py_ratio_mem_final[i] = sum_ratio_mem / result_serve_num[i];
		printf("case %d cpu ratio：%f\n", i + 1, py_ratio_cpu_final[i]);
		printf("case %d mem ratio：%f\n", i + 1, py_ratio_mem_final[i]);

	}

	int return_serve_num;

	// 优化 cpu or mem？
	// opt_target = 0;
	if (opt_target == 0)
	{
		int temp = 0;
		for (int i = 1; i < population_num; i++)
		{
			if (py_ratio_cpu_final[i] >= py_ratio_cpu_final[temp])  temp = i;
		}
		// require_vm = result_predict_super[temp];
		return_serve_num = result_serve_num[temp];
		// result_save = result_save_super[temp];

		// 进行复制！！！
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = result_predict_super[temp][i];
		for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = result_save_super[temp][i];

		print_data(result_save_super[temp], result_serve_num[temp]);
		print_data(result_save, return_serve_num);

		printf("FINAL case %d ：", temp);
		// 打印模型预测结果
		printf("[");
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
		printf("]\n");
		printf("FINAL case ratio：%f\n", py_ratio_cpu_final[temp]);

	}
	else
	{
		int temp = 0;
		for (int i = 1; i < population_num; i++)
		{
			if (py_ratio_mem_final[i] >= py_ratio_mem_final[temp])  temp = i;
		}
		// require_vm = result_predict_super[temp];
		return_serve_num = result_serve_num[temp];
		// result_save = result_save_super[temp];

		// 进行复制！！！
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = result_predict_super[temp][i];
		for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = result_save_super[temp][i];

		// print_data(result_save_super[temp], result_serve_num[temp]);
		// print_data(result_save, return_serve_num);
		printf("FINAL case：");
		// 打印模型预测结果
		printf("[");
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
		printf("]\n");
		printf("FINAL case ratio：%f\n", py_ratio_mem_final[temp]);
	}


	return return_serve_num;
}

/*
函数目标: 分数！！！ 种群使用贪心竞争策略
*/
int putVM_score_compet_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	// 扩大 require_vm 样本的种群！！！
	int population_num = 16;

	// 增幅！！！
	int resize_begin = -2;
	int resize_end = 1;

	// 一组解到多组解的演变
	int **result_predict_super = new int *[population_num];
	for (int i = 0; i < population_num; i++)    result_predict_super[i] = new int[inputcontrol.flavorMaxnum];
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			result_predict_super[i][j] = require_vm[j];
		}
	}
	//找见最大的
	int max_count_a_num = 0;
	int max_count_b_num = 0;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_a_num] <= require_vm[i])  max_count_a_num = i;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_b_num] <= require_vm[i] && i != max_count_a_num)  max_count_b_num = i;

	for (int i = resize_begin; i <= resize_end; i++)
	{
		for (int j = resize_begin; j <= resize_end; j++)
		{
			//printf(" %d ", result_predict_super[(i + 1) * 4 + (j + 1)][max_count_a_num]);
			//printf(" %d \n", result_predict_super[(i + 1) * 4 + (j + 1)][max_count_b_num]);
			result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_a_num] += i;
			result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_b_num] += j;
			printf("case %d :  %d ", (i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin), result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_a_num]);
			printf(" %d \n", result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_b_num]);
		}
	}
	for (int i = 0; i < population_num; i++)
	{
		print_data(result_predict_super[i], 1);
	}

	// 针对多组解的多种放置方案！！！
	int max_serve_py = 2000;  // 服务器最大的个数!!!
	int **result_save_super = new int *[population_num];
	for (int i = 0; i < population_num; i++)    result_save_super[i] = new int[max_serve_py*inputcontrol.flavorMaxnum];
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < max_serve_py*inputcontrol.flavorMaxnum; j++)
		{
			result_save_super[i][j] = 0;
		}
	}

	int *result_serve_num = new int[population_num];
	double *score_result = new double[population_num];
	Flavor *inputFlavor_data = new Flavor[inputcontrol.flavorMaxnum];

	for (int i = 0; i < population_num; i++)
	{
		if (i == 0)
		{
			for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
			{
				inputFlavor_data[i].cpu_core_num = inputFlavor[i].cpu_core_num;
				inputFlavor_data[i].mem_size = inputFlavor[i].mem_size;
				inputFlavor_data[i].flavor_name = inputFlavor[i].flavor_name;
			}
		}

		//print_data(result_predict_super[i], 1);
		result_serve_num[i] = putVM_greedy(result_predict_super[i], inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i]);
		//矫正
		result_serve_num[i] = putVM_correct(result_predict_super[i], inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i], result_serve_num[i]);

		//得分
		score_result[i] = get_score(require_vm, result_predict_super[i], inputFlavor, opt_target, result_serve_num[i]);

		//print_data(result_predict_super[i], 1);

		if (i != population_num - 1)
			for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
			{
				inputFlavor[i].cpu_core_num = inputFlavor_data[i].cpu_core_num;
				inputFlavor[i].mem_size = inputFlavor_data[i].mem_size;
				inputFlavor[i].flavor_name = inputFlavor_data[i].flavor_name;
			}

		//print_data(result_save_super[i], result_serve_num[i]);
		printf("case %d：result_serve_num: %d\n", i + 1, result_serve_num[i]);
	}

	int return_serve_num;

	int temp = 0;
	for (int i = 1; i < population_num; i++)
	{
		if (score_result[i] >= score_result[temp])  temp = i;
	}
	// require_vm = result_predict_super[temp];
	return_serve_num = result_serve_num[temp];
	// result_save = result_save_super[temp];

	// 进行复制！！！
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = result_predict_super[temp][i];
	for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = result_save_super[temp][i];

	//print_data(result_save_super[temp], result_serve_num[temp]);
	//print_data(result_save, return_serve_num);

	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf("%f \n", score_result[i]);

	printf("FINAL case %d ：", temp);
	// 打印模型预测结果
	printf("[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
	printf("]\n");
	printf("FINAL case score：%f\n", score_result[temp]);


	return return_serve_num;
}


/*
函数目标:种群使用动归竞争策略
*/
int putVM_compet_dynamicpro(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	// 扩大 require_vm 样本的种群！！！
	int population_num = 25;

	// 增幅！！！
	int resize_begin = -2;
	int resize_end = 2;



	// 一组解到多组解的演变
	int **result_predict_super = new int *[population_num];
	for (int i = 0; i < population_num; i++)    result_predict_super[i] = new int[inputcontrol.flavorMaxnum];
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			result_predict_super[i][j] = require_vm[j];
		}
	}
	//找见最大的
	int max_count_a_num = 0;
	int max_count_b_num = 0;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_a_num] <= require_vm[i])  max_count_a_num = i;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_b_num] <= require_vm[i] && i != max_count_a_num)  max_count_b_num = i;

	for (int i = resize_begin; i <= resize_end; i++)
	{
		for (int j = resize_begin; j <= resize_end; j++)
		{
			//printf(" %d ", result_predict_super[(i + 1) * 4 + (j + 1)][max_count_a_num]);
			//printf(" %d \n", result_predict_super[(i + 1) * 4 + (j + 1)][max_count_b_num]);
			result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_a_num] += i;
			result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_b_num] += j;
			//printf("case %d :  %d ", (i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin), result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_a_num]);
			//printf(" %d \n", result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_b_num]);
		}
	}
	for (int i = 0; i < population_num; i++)
	{
		print_data(result_predict_super[i], 1);
	}

	// 针对多组解的多种放置方案！！！
	int max_serve_py = 2000;  // 服务器最大的个数!!!
	int **result_save_super = new int *[population_num];
	for (int i = 0; i < population_num; i++)    result_save_super[i] = new int[max_serve_py*inputcontrol.flavorMaxnum];
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < max_serve_py*inputcontrol.flavorMaxnum; j++)
		{
			result_save_super[i][j] = 0;
		}
	}

	int *result_serve_num = new int[population_num];
	double *score_result = new double[population_num];
	Flavor *inputFlavor_data = new Flavor[inputcontrol.flavorMaxnum];

	for (int i = 0; i < population_num; i++)
	{
		// 保留一份初始顺序
		if (i == 0)
		{
			for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
			{
				inputFlavor_data[i].cpu_core_num = inputFlavor[i].cpu_core_num;
				inputFlavor_data[i].mem_size = inputFlavor[i].mem_size;
				inputFlavor_data[i].flavor_name = inputFlavor[i].flavor_name;
			}
		}

		int num_of_Serve_vm = 0;
		//print_data(result_predict_super[i], 1);
		result_serve_num[i] = putVM_dynamicpro(result_predict_super[i], num_of_Serve_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i]);
		//score_result[i] = get_score(require_vm,result_predict_super[i], inputFlavor, opt_target, result_serve_num[i]);

		//print_data(result_predict_super[i], 1);

		if (i != population_num - 1)
			for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
			{
				inputFlavor[i].cpu_core_num = inputFlavor_data[i].cpu_core_num;
				inputFlavor[i].mem_size = inputFlavor_data[i].mem_size;
				inputFlavor[i].flavor_name = inputFlavor_data[i].flavor_name;
			}

		//print_data(result_save_super[i], result_serve_num[i]);
		//printf("case %d：result_serve_num: %d\n", i + 1, result_serve_num[i]);
	}



	double **py_ratio_cpu = new double *[population_num];  // 一个物理机的cpu资源利用率
	double **py_ratio_mem = new double *[population_num];  // 一个物理机的内存资源利用率 

	double *py_ratio_cpu_final = new double[population_num];  //
	double *py_ratio_mem_final = new double[population_num];  // 

	for (int i = 0; i < population_num; i++)
	{
		py_ratio_cpu[i] = new double[result_serve_num[i]];
		py_ratio_mem[i] = new double[result_serve_num[i]];

		for (int j = 0; j < result_serve_num[i]; j++)
		{
			// 第j个物理机的利用率和剩余资源 
			int sum_temp_cpu = 0;
			int sum_temp_mem = 0;
			for (int k = 0; k < num_vm; k++)
			{
				if (result_save_super[i][j*num_vm + k] != 0)
				{
					sum_temp_cpu += result_save_super[i][j*num_vm + k] * inputFlavor[k].cpu_core_num;
					sum_temp_mem += result_save_super[i][j*num_vm + k] * inputFlavor[k].mem_size;
				}
			}
			// 服务器上的单个利用率
			py_ratio_cpu[i][j] = (double)sum_temp_cpu / (double)resource_pm.cpu_core_num;
			py_ratio_mem[i][j] = (double)sum_temp_mem / (double)resource_pm.mem_size;
		}

		double sum_ratio_cpu = 0;
		double sum_ratio_mem = 0;
		for (int m = 0; m < result_serve_num[i]; m++)
		{
			sum_ratio_cpu += py_ratio_cpu[i][m];
			sum_ratio_mem += py_ratio_mem[i][m];
		}

		// 最终的利用率
		py_ratio_cpu_final[i] = sum_ratio_cpu / result_serve_num[i];
		py_ratio_mem_final[i] = sum_ratio_mem / result_serve_num[i];
		//printf("case %d cpu ratio：%f\n", i + 1, py_ratio_cpu_final[i]);
		//printf("case %d mem ratio：%f\n", i + 1, py_ratio_mem_final[i]);

	}

	int return_serve_num;

	// 优化 cpu or mem？
	// opt_target = 0;
	if (opt_target == 0)
	{
		int temp = 0;
		for (int i = 1; i < population_num; i++)
		{
			if (py_ratio_cpu_final[i] >= py_ratio_cpu_final[temp])  temp = i;
		}
		// require_vm = result_predict_super[temp];
		return_serve_num = result_serve_num[temp];
		// result_save = result_save_super[temp];

		// 进行复制！！！
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = result_predict_super[temp][i];
		for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = result_save_super[temp][i];

		//print_data(result_save_super[temp], result_serve_num[temp]);
		//print_data(result_save, return_serve_num);

		printf("FINAL case %d ：", temp);
		// 打印模型预测结果
		printf("[");
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
		printf("]\n");
		printf("FINAL case ratio：%f\n", py_ratio_cpu_final[temp]);

	}
	else
	{
		int temp = 0;
		for (int i = 1; i < population_num; i++)
		{
			if (py_ratio_mem_final[i] >= py_ratio_mem_final[temp])  temp = i;
		}
		// require_vm = result_predict_super[temp];
		return_serve_num = result_serve_num[temp];
		// result_save = result_save_super[temp];

		// 进行复制！！！
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = result_predict_super[temp][i];
		for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = result_save_super[temp][i];

		// print_data(result_save_super[temp], result_serve_num[temp]);
		// print_data(result_save, return_serve_num);
		printf("FINAL case：");
		// 打印模型预测结果
		printf("[");
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
		printf("]\n");
		printf("FINAL case ratio：%f\n", py_ratio_mem_final[temp]);
	}


	return return_serve_num;
}

/*
函数目标:分数！！！种群使用动归竞争策略
*/
int putVM_score_compet_dynamicpro(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	// 扩大 require_vm 样本的种群！！！
	int population_num = 25;

	// 增幅！！！
	int resize_begin = -2;
	int resize_end = 2;



	// 一组解到多组解的演变
	int **result_predict_super = new int *[population_num];
	for (int i = 0; i < population_num; i++)    result_predict_super[i] = new int[inputcontrol.flavorMaxnum];
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			result_predict_super[i][j] = require_vm[j];
		}
	}
	//找见最大的
	int max_count_a_num = 0;
	int max_count_b_num = 0;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_a_num] <= require_vm[i])  max_count_a_num = i;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_b_num] <= require_vm[i] && i != max_count_a_num)  max_count_b_num = i;

	for (int i = resize_begin; i <= resize_end; i++)
	{
		for (int j = resize_begin; j <= resize_end; j++)
		{
			//printf(" %d ", result_predict_super[(i + 1) * 4 + (j + 1)][max_count_a_num]);
			//printf(" %d \n", result_predict_super[(i + 1) * 4 + (j + 1)][max_count_b_num]);
			result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_a_num] += i;
			result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_b_num] += j;
			//printf("case %d :  %d ", (i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin), result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_a_num]);
			//printf(" %d \n", result_predict_super[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)][max_count_b_num]);
		}
	}
	for (int i = 0; i < population_num; i++)
	{
		print_data(result_predict_super[i], 1);
	}

	// 针对多组解的多种放置方案！！！
	int max_serve_py = 2000;  // 服务器最大的个数!!!
	int **result_save_super = new int *[population_num];
	for (int i = 0; i < population_num; i++)    result_save_super[i] = new int[max_serve_py*inputcontrol.flavorMaxnum];
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < max_serve_py*inputcontrol.flavorMaxnum; j++)
		{
			result_save_super[i][j] = 0;
		}
	}

	int *result_serve_num = new int[population_num];
	double *score_result = new double[population_num];
	Flavor *inputFlavor_data = new Flavor[inputcontrol.flavorMaxnum];

	for (int i = 0; i < population_num; i++)
	{
		// 保留一份初始顺序
		if (i == 0)
		{
			for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
			{
				inputFlavor_data[i].cpu_core_num = inputFlavor[i].cpu_core_num;
				inputFlavor_data[i].mem_size = inputFlavor[i].mem_size;
				inputFlavor_data[i].flavor_name = inputFlavor[i].flavor_name;
			}
		}


		int num_of_Serve_vm = 0;
		//print_data(result_predict_super[i], 1);
		result_serve_num[i] = putVM_dynamicpro(result_predict_super[i], num_of_Serve_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i]);

		//矫正
		result_serve_num[i] = putVM_correct(result_predict_super[i], inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i], result_serve_num[i]);

		//得分
		score_result[i] = get_score(require_vm, result_predict_super[i], inputFlavor, opt_target, result_serve_num[i]);


		//print_data(result_predict_super[i], 1);

		if (i != population_num - 1)
			for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
			{
				inputFlavor[i].cpu_core_num = inputFlavor_data[i].cpu_core_num;
				inputFlavor[i].mem_size = inputFlavor_data[i].mem_size;
				inputFlavor[i].flavor_name = inputFlavor_data[i].flavor_name;
			}

		//print_data(result_save_super[i], result_serve_num[i]);
		//printf("case %d：result_serve_num: %d\n", i + 1, result_serve_num[i]);
	}
	int return_serve_num;

	int temp = 0;
	for (int i = 1; i < population_num; i++)
	{
		if (score_result[i] >= score_result[temp])  temp = i;
	}
	// require_vm = result_predict_super[temp];
	return_serve_num = result_serve_num[temp];
	// result_save = result_save_super[temp];

	// 进行复制！！！
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = result_predict_super[temp][i];
	for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = result_save_super[temp][i];

	//print_data(result_save_super[temp], result_serve_num[temp]);
	//print_data(result_save, return_serve_num);

	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf("%f \n", score_result[i]);

	printf("FINAL case %d ：", temp);
	// 打印模型预测结果
	printf("[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
	printf("]\n");
	printf("FINAL case score：%f\n", score_result[temp]);


	return return_serve_num;
}

/*
require_vm :我们预测部分预测的个体，我们将其作为初始的第一代
*/

int gene_dynamicpro(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{


	//遗传迭代次数
	const int gene_total_num = 20;

	// 种群规模！！！
	const int population_num = 25;

	// 选择天灾之后剩余的种群个体数量
	const int population_last = 12;


	// 随机数产生几个阶梯？
	const int ladder_rand = 20;


	// 初始第一代的增幅！！！
	const int resize_begin = -1;
	const int resize_end = 3;

	// 种群中全部个体的基因信息
	GeneInfoUnit *geneinfounit = new GeneInfoUnit[population_num];
	// 种群中剩余个体的基因信息
	// GeneInfoUnit *geneinfounit_last = new GeneInfoUnit[population_last];

	// 种群中个体基因信息继承
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			// 表示第i个个体的第j个基因  从初始继承下来
			geneinfounit[i].gene[j] = require_vm[j];
		}
	}

	//找见最大的两个
	int max_count_a_num = 0;
	int max_count_b_num = 0;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_a_num] <= require_vm[i])  max_count_a_num = i;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_b_num] <= require_vm[i] && i != max_count_a_num)  max_count_b_num = i;

	// 通过最大那两个，诞生第一代种群
	for (int i = resize_begin; i <= resize_end; i++)
	{
		for (int j = resize_begin; j <= resize_end; j++)
		{
			geneinfounit[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)].gene[max_count_a_num] += i;
			geneinfounit[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)].gene[max_count_b_num] += j;
		}
	}

	//打印第一代
	for (int i = 0; i < population_num; i++)
	{
		print_data(geneinfounit[i].gene, 1);
	}

	// int *result_serve_num = new int[population_num];  //返回的服务器数量
	// double *score_result = new double[population_num];  //得分的结果（适应度）
	Flavor *inputFlavor_data = new Flavor[inputcontrol.flavorMaxnum];  //保存初始的虚拟机性能表信息

																	   //开始遗传迭代 	 gene_temp_num : 当前种群处于第几代？ 初始为第1代
	for (int gene_temp_num = 1; gene_temp_num <= gene_total_num; )
	{
		//计算种群的适应度总值
		double sum_value_fit = 0;

		//开始计算种群中每个个体的适应度
		for (int i = 0; i < population_num; i++)
		{
			if (i == 0) // 第一次进来保留初始的虚拟机性能表信息
			{
				for (int j = 0; j<inputcontrol.flavorMaxnum; j++)
				{
					inputFlavor_data[j].cpu_core_num = inputFlavor[j].cpu_core_num;
					inputFlavor_data[j].mem_size = inputFlavor[j].mem_size;
					inputFlavor_data[j].flavor_name = inputFlavor[j].flavor_name;
				}
			}
			// 对个体的放置方案进行初始化
			for (int j = 0; j < 16 * 2000; j++)  geneinfounit[i].result_save_gene[j] = 0;

			int num_of_Serve_vm = 0;
			//print_data(result_predict_super[i], 1);

			print_data(geneinfounit[i].gene, 1);
			// 动归
			geneinfounit[i].result_serve_num = putVM_dynamicpro(geneinfounit[i].gene, num_of_Serve_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, geneinfounit[i].result_save_gene);
			print_data(geneinfounit[i].gene, 1);
			//矫正
			geneinfounit[i].result_serve_num = putVM_correct(geneinfounit[i].gene, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, geneinfounit[i].result_save_gene, geneinfounit[i].result_serve_num);
			print_data(geneinfounit[i].gene, 1);
			//个体适应度评估
			geneinfounit[i].value_fit = get_score(require_vm, geneinfounit[i].gene, inputFlavor, opt_target, geneinfounit[i].result_serve_num);

			//print_data(result_predict_super[i], 1);

			if ((i != population_num - 1) || (gene_temp_num != gene_total_num))
				for (int j = 0; j<inputcontrol.flavorMaxnum; j++)
				{
					inputFlavor[j].cpu_core_num = inputFlavor_data[j].cpu_core_num;
					inputFlavor[j].mem_size = inputFlavor_data[j].mem_size;
					inputFlavor[j].flavor_name = inputFlavor_data[j].flavor_name;
				}

			// 计算种群适应度的总和
			sum_value_fit += geneinfounit[i].value_fit;
		}
		// 打印适应度的值
		//for (int a = 0; a < population_num; a++)  printf("%f ", geneinfounit[a].value_fit);
		printf("第%d代种群中全部个体的平均适应度 :%f\n", gene_temp_num, sum_value_fit / population_num);

		// 下一代开始！！！！！
		gene_temp_num++;

		//	如果是最后一次迭代，那么就不需要产生新的个体！！
		if (gene_temp_num != gene_total_num)
		{
			// 保存所有个体的排序序号 初始化
			// int *last_count = new int[population_num];
			// for (int a = 0; a < population_num; a++)  last_count[a] = a;

			// 评估完毕之后，制造轰炸区（天灾）筛选优秀个体！！！
			//排序
			GeneInfoUnit geneinfounit_temp;
			for (int i = 0; i < population_num; i++)
			{
				for (int j = i; j < population_num; j++)
				{
					if (geneinfounit[j].value_fit >= geneinfounit[i].value_fit)
					{
						geneinfounit_temp = geneinfounit[i];
						geneinfounit[i] = geneinfounit[j];
						geneinfounit[j] = geneinfounit_temp;
					}
				}
			}
			// 打印排序之后的！！！
			// for (int a = 0; a < population_num; a++)  printf("%f ", geneinfounit[a].value_fit);

			//种群中最优秀的分数！！！
			printf("第%d代种群中最优秀的个体 :%f\n", gene_temp_num - 1, geneinfounit[0].value_fit);

			// 产生新的个体
			/*  注意：geneinfounit种群中前population_last是保留下来的，后面的个体将被取代！！！！*/
			/*     通过保留下来的优秀个体来产生新的个体（扩大种群）    */
			double coeff_anneal = 0;

			// 种群的产生方案！！！
			for (int i = population_last; i < population_num; i++)
			{
				for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
				{
					//随机数开启
					srand((unsigned)time(NULL));
					//模拟退火
					coeff_anneal = ((double)(rand() % ladder_rand) / ladder_rand - 0.5)*0.5*((double)(gene_total_num - gene_temp_num + 1) / gene_total_num);
					geneinfounit[i].gene[j] = (int)(geneinfounit[i%population_last].gene[j] + coeff_anneal * geneinfounit[i%population_last].gene[j]);
					if (geneinfounit[i].gene[j] < 0)  geneinfounit[i].gene[j] = 0;
				}
			}

			// 种群固定遍历法去搜索解

		}
		else
		{
			printf("第%d代种群中最优秀的个体 :%f\n", gene_temp_num - 1, geneinfounit[0].value_fit);
		}
	}

	int return_serve_num;

	int temp = 0;
	for (int i = 1; i < population_num; i++)
	{
		if (geneinfounit[i].value_fit > geneinfounit[temp].value_fit)  temp = i;
	}
	// require_vm = result_predict_super[temp];
	return_serve_num = geneinfounit[temp].result_serve_num;
	// result_save = result_save_super[temp];

	// 进行复制！！！
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = geneinfounit[temp].gene[i];
	for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = geneinfounit[temp].result_save_gene[i];

	//print_data(result_save_super[temp], result_serve_num[temp]);
	//print_data(result_save, return_serve_num);

	// for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf("%f \n", score_result[i]);

	printf("FINAL case %d ：", temp);
	// 打印模型预测结果
	printf("[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
	printf("]\n");
	printf("FINAL case score：%f\n", geneinfounit[temp].value_fit);


	return return_serve_num;
}

int gene_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{


	//遗传迭代次数
	const int gene_total_num = 20;

	// 种群规模！！！
	const int population_num = 25;

	// 选择天灾之后剩余的种群个体数量
	const int population_last = 12;


	// 随机数产生几个阶梯？
	const int ladder_rand = 20;


	putVM_seq(require_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);//原序列

																												   // 初始第一代的增幅！！！
	const int resize_begin = -1;
	const int resize_end = 3;

	// 种群中全部个体的基因信息
	GeneInfoUnit *geneinfounit = new GeneInfoUnit[population_num];
	// 种群中剩余个体的基因信息
	// GeneInfoUnit *geneinfounit_last = new GeneInfoUnit[population_last];

	// 种群中个体基因信息继承
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			// 表示第i个个体的第j个基因  从初始继承下来
			geneinfounit[i].gene[j] = require_vm[j];
		}
	}

	//找见最大的两个
	int max_count_a_num = 0;
	int max_count_b_num = 0;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_a_num] <= require_vm[i])  max_count_a_num = i;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_b_num] <= require_vm[i] && i != max_count_a_num)  max_count_b_num = i;

	// 通过最大那两个，诞生第一代种群
	for (int i = resize_begin; i <= resize_end; i++)
	{
		for (int j = resize_begin; j <= resize_end; j++)
		{
			geneinfounit[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)].gene[max_count_a_num] += i;
			geneinfounit[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)].gene[max_count_b_num] += j;
		}
	}

	//打印第一代
	for (int i = 0; i < population_num; i++)
	{
		print_data(geneinfounit[i].gene, 1);
	}

	// int *result_serve_num = new int[population_num];  //返回的服务器数量
	// double *score_result = new double[population_num];  //得分的结果（适应度）
	Flavor *inputFlavor_data = new Flavor[inputcontrol.flavorMaxnum];  //保存初始的虚拟机性能表信息

																	   //开始遗传迭代 	 gene_temp_num : 当前种群处于第几代？ 初始为第1代
	for (int gene_temp_num = 1; gene_temp_num <= gene_total_num; )
	{
		//计算种群的适应度总值
		double sum_value_fit = 0;

		//开始计算种群中每个个体的适应度
		for (int i = 0; i < population_num; i++)
		{
			if (i == 0) // 第一次进来保留初始的虚拟机性能表信息
			{
				for (int j = 0; j<inputcontrol.flavorMaxnum; j++)
				{
					inputFlavor_data[j].cpu_core_num = inputFlavor[j].cpu_core_num;
					inputFlavor_data[j].mem_size = inputFlavor[j].mem_size;
					inputFlavor_data[j].flavor_name = inputFlavor[j].flavor_name;
				}
			}
			// 对个体的放置方案进行初始化
			for (int j = 0; j < 16 * 2000; j++)  geneinfounit[i].result_save_gene[j] = 0;

			int num_of_Serve_vm = 0;
			//print_data(result_predict_super[i], 1);

			print_data(geneinfounit[i].gene, 1);


			int *temp_gene = new int[inputcontrol.flavorMaxnum];
			// 放置预测信息被改变，在这里进行拷贝。
			for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
			{
				temp_gene[j] = geneinfounit[i].gene[j];
			}

			//贪婪
			// geneinfounit[i].result_serve_num = putVM_greedy(temp_gene, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, geneinfounit[i].result_save_gene);
			putVM_greedy_without_seq(geneinfounit[i].gene, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, geneinfounit[i].result_save_gene);//原序列

			print_data(geneinfounit[i].gene, 1);
			//print_data(result_predict_super[i], 1);


			//矫正
			geneinfounit[i].result_serve_num = putVM_correct(geneinfounit[i].gene, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, geneinfounit[i].result_save_gene, geneinfounit[i].result_serve_num);




			print_data(geneinfounit[i].gene, 1);



			//个体适应度评估
			geneinfounit[i].value_fit = get_score(require_vm, geneinfounit[i].gene, inputFlavor, opt_target, geneinfounit[i].result_serve_num);
			print_data(geneinfounit[i].gene, 1);

			if ((i != population_num - 1) || (gene_temp_num != gene_total_num))
				for (int j = 0; j<inputcontrol.flavorMaxnum; j++)
				{
					inputFlavor[j].cpu_core_num = inputFlavor_data[j].cpu_core_num;
					inputFlavor[j].mem_size = inputFlavor_data[j].mem_size;
					inputFlavor[j].flavor_name = inputFlavor_data[j].flavor_name;
				}

			// 计算种群适应度的总和
			sum_value_fit += geneinfounit[i].value_fit;
		}
		// 打印适应度的值
		//for (int a = 0; a < population_num; a++)  printf("%f ", geneinfounit[a].value_fit);
		printf("第%d代种群中全部个体的平均适应度 :%f\n", gene_temp_num, sum_value_fit / population_num);

		// 下一代开始！！！！！
		gene_temp_num++;

		//	如果是最后一次迭代，那么就不需要产生新的个体！！
		if (gene_temp_num != gene_total_num)
		{
			// 保存所有个体的排序序号 初始化
			// int *last_count = new int[population_num];
			// for (int a = 0; a < population_num; a++)  last_count[a] = a;

			// 评估完毕之后，制造轰炸区（天灾）筛选优秀个体！！！
			//排序
			GeneInfoUnit geneinfounit_temp;
			for (int i = 0; i < population_num; i++)
			{
				for (int j = i; j < population_num; j++)
				{
					if (geneinfounit[j].value_fit >= geneinfounit[i].value_fit)
					{
						geneinfounit_temp = geneinfounit[i];
						geneinfounit[i] = geneinfounit[j];
						geneinfounit[j] = geneinfounit_temp;
					}
				}
			}
			// 打印排序之后的！！！
			// for (int a = 0; a < population_num; a++)  printf("%f ", geneinfounit[a].value_fit);

			//种群中最优秀的分数！！！
			printf("第%d代种群中最优秀的个体 :%f\n", gene_temp_num - 1, geneinfounit[0].value_fit);

			// 产生新的个体
			/*  注意：geneinfounit种群中前population_last是保留下来的，后面的个体将被取代！！！！*/
			/*     通过保留下来的优秀个体来产生新的个体（扩大种群）    */
			double coeff_anneal = 0;
			for (int i = population_last; i < population_num; i++)
			{
				for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
				{
					//随机数开启
					srand((unsigned)time(NULL));
					//模拟退火
					coeff_anneal = ((double)(rand() % ladder_rand) / ladder_rand - 0.5)*0.5*((double)(gene_total_num - gene_temp_num + 1) / gene_total_num);
					geneinfounit[i].gene[j] = (int)(geneinfounit[i%population_last].gene[j] + coeff_anneal * geneinfounit[i%population_last].gene[j]);
					if (geneinfounit[i].gene[j] < 0)  geneinfounit[i].gene[j] = 0;
				}
			}
		}
		else
		{
			printf("第%d代种群中最优秀的个体 :%f\n", gene_temp_num - 1, geneinfounit[0].value_fit);
		}
	}

	int return_serve_num;

	int temp = 0;
	for (int i = 1; i < population_num; i++)
	{
		if (geneinfounit[i].value_fit > geneinfounit[temp].value_fit)  temp = i;
	}
	// require_vm = result_predict_super[temp];
	return_serve_num = geneinfounit[temp].result_serve_num;
	// result_save = result_save_super[temp];

	// 进行复制！！！
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = geneinfounit[temp].gene[i];
	for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = geneinfounit[temp].result_save_gene[i];

	//print_data(result_save_super[temp], result_serve_num[temp]);
	//print_data(result_save, return_serve_num);

	// for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf("%f \n", score_result[i]);

	printf("FINAL case %d ：", temp);
	// 打印模型预测结果
	printf("[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
	printf("]\n");
	printf("FINAL case score：%f\n", geneinfounit[temp].value_fit);


	return return_serve_num;
}

/*
函数目标:模拟退火算法SAA 放置虚拟机
方法:	模拟退火
输入:	*require_vm		虚拟机需求
num_vm			虚拟机类型总数
resource_pm     物理机性能
opt_target		优化目标 0:CPU 1:MEM
*inputFlavor	虚拟机性能表
*result_save	服务器数据表(结果)
输出:	创建的服务器个数
*/
int putVM_score_SAA_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save) {
	srand((unsigned)time(NULL));

	double max_score = 0;

	int result_serve_num; //最终服务器的数量
	int max_serve_py = 2000;  //最大服务器数量

	putVM_seq(require_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);//原序列

	double score_final;  //最终得分
	int *require_vm_iter = new int[num_vm];
	int *require_vm_copy = new int[num_vm];
	int *require_vm_copy_temp = new int[num_vm];

	/*printf("\npredict final result : [");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
	printf("]\n");*/

	for (int i = 0; i < num_vm; i++) {
		require_vm_iter[i] = require_vm[i];
	}

	float Temperature_first = 1.5;       //初始温度
	int Num_of_inner_first = 40;         //内圈循环开始的次数，温度越低迭代次数逐渐增加
	float Step = 0.1;                    //温度Temperature的步进
	float Temperature_end = 0.0001;               //截至温度
	int *result_save_temp1 = new int[max_serve_py*num_vm]; //原序列
	int *result_save_temp2 = new int[max_serve_py*num_vm]; //新序列
	int result_serve_num1; //原序列
	int result_serve_num2; //新序列
	double score_result1;   //原序列得分
	double score_result2;   //新序列得分

	int time_iter_saa = 20; //重复退火次数
	for (int iter_i = 0; iter_i < time_iter_saa; iter_i++) {
		/*-----------------------退火开始---------------------------------------------------*/
		for (int i = 0; i < num_vm; i++) {
			require_vm_copy[i] = require_vm[i];
		}
		float Temperature = Temperature_first;  //当前温度
												/*--------------------外循环控制温度----------------------------*/
		int range_rand;  //随机数的范围-range_rand到+range_rand
		float range_rand_start = 10; //初始随机数范围
		while (Temperature > Temperature_end) {
			int Num_of_inner = Num_of_inner_first + (int)((Temperature_first - Temperature) * 1 / Step) * 2; //内循环次数,随温度降低而下降
																											 //int Num_of_inner = Num_of_inner_first;
			int range_rand = (int)(range_rand_start * (Temperature / Temperature_first));
			if (range_rand <= 1) {
				range_rand = 2;
			}
			//内循环开始
			for (int i = 0; i < Num_of_inner; i++) {
				//产生随机数
				int num_change = rand() % num_vm;       //改变第几个虚拟机
				int predict_change = rand() % (2 * range_rand) - range_rand;   //-range_rand到+range_rand的随机数
				while (predict_change == 0) {
					predict_change = rand() % (2 * range_rand) - range_rand;   //取到0就再取一次
				}

				//改变原序列
				for (int ii = 0; ii < num_vm; ii++) {
					require_vm_copy_temp[ii] = require_vm_copy[ii];
				}
				require_vm_copy_temp[num_change] += predict_change;
				if (require_vm_copy_temp[num_change] < 0) {  //虚拟机个数不能小于0
					require_vm_copy_temp[num_change] = 0;
				}
				//开启放置
				for (int zreo_i = 0; zreo_i < max_serve_py*num_vm; zreo_i++) {
					result_save_temp1[zreo_i] = 0;
					result_save_temp2[zreo_i] = 0;
				}

				/*printf("\n [");
				for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm_copy[i]);
				printf("]\n");*/

				result_serve_num1 = putVM_greedy_without_seq(require_vm_copy, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_temp1);//原序列
				result_serve_num2 = putVM_greedy_without_seq(require_vm_copy_temp, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_temp2);//新序列

																																												  //开始评分
				score_result1 = get_score(require_vm, require_vm_copy, inputFlavor, opt_target, result_serve_num1);
				score_result2 = get_score(require_vm, require_vm_copy_temp, inputFlavor, opt_target, result_serve_num2);

				//是否更新序列
				if (score_result1 < score_result2) {  //若新样本更优，则替换原样本
					score_final = score_result2;
					result_serve_num = result_serve_num2;
					for (int ii = 0; ii < num_vm; ii++) {
						require_vm_copy[ii] = require_vm_copy_temp[ii];
					}
				}
				else {  //若新样本较差，则有有一定概率代替原样本
					double derta_s = (score_result1 - score_result2) * 100; //分值归到0-100
					double D_saa = exp(-derta_s / Temperature);
					int D_saa_int = (int)(D_saa * 100);
					int rand_saa = rand() % 100;
					if (D_saa_int>rand_saa) { //替换
						score_final = score_result2;
						result_serve_num = result_serve_num2;
						for (int ii = 0; ii < num_vm; ii++) {
							require_vm_copy[ii] = require_vm_copy_temp[ii];
						}
					}
					else { //不替换
						score_final = score_result1;
						result_serve_num = result_serve_num1;
					}
				}
			}
			Temperature -= Step;
			//打印
			//printf("\n当前温度：%f  新样本得分：%f", Temperature, score_final);
		}

		printf("\n当前温度：%f  新样本得分：%f", Temperature, score_final);
		if (score_final > max_score) {
			max_score = score_final;
			for (int i = 0; i < num_vm; i++) {  //一次退火结果赋值
				require_vm_iter[i] = require_vm_copy[i];
			}
		}
		/*------------------------------退火结束--------------------------------------------*/
	}


	for (int i = 0; i < num_vm; i++) {  //最终结果赋值
		require_vm[i] = require_vm_iter[i];
	}
	result_serve_num = putVM_greedy(require_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);//原序列


	return result_serve_num;
}

/*  老版SAA  */
/*
int putVM_score_SAA_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save) {
int result_serve_num; //最终服务器的数量
int max_serve_py = 2000;  //最大服务器数量

putVM_seq(require_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);//原序列

double score_final;  //最终得分
int *require_vm_copy = new int[num_vm];
int *require_vm_copy_temp = new int[num_vm];

//printf("\npredict final result : [");
//for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
//printf("]\n");

for (int i = 0; i < num_vm; i++) {
require_vm_copy[i] = require_vm[i];
}

float Temperature_first = 100;       //初始温度
int Num_of_inner_first = 30;         //内圈循环开始的次数，温度越低迭代次数逐渐增加
float Step = 1;                    //温度Temperature的步进
float Temperature_end = 5;               //截至温度
float Temperature = Temperature_first;  //当前温度

int *result_save_temp1 = new int[max_serve_py*num_vm]; //原序列
int *result_save_temp2 = new int[max_serve_py*num_vm]; //新序列
int result_serve_num1; //原序列
int result_serve_num2; //新序列
double score_result1;   //原序列得分
double score_result2;   //新序列得分
//--------------------外循环控制温度----------------------------
int range_rand;  //随机数的范围-range_rand到+range_rand
float range_rand_start = 10; //初始随机数范围
while (Temperature > Temperature_end) {
int Num_of_inner = Num_of_inner_first + Temperature_first - Temperature; //内循环次数,随温度降低而下降
int range_rand = (int)(range_rand_start * (Temperature / Temperature_first));
if (range_rand <= 1) {
range_rand = 2;
}
//内循环开始
for (int i = 0; i < Num_of_inner; i++) {
//产生随机数
int num_change = rand() % num_vm;       //改变第几个虚拟机
int predict_change = rand() % (2 * range_rand) - range_rand;   //-range_rand到+range_rand的随机数
while (predict_change == 0) {
predict_change = rand() % (2 * range_rand) - range_rand;   //取到0就再取一次
}

//改变原序列
for (int ii = 0; ii < num_vm; ii++) {
require_vm_copy_temp[ii] = require_vm_copy[ii];
}
require_vm_copy_temp[num_change] += predict_change;
if (require_vm_copy_temp[num_change] < 0) {  //虚拟机个数不能小于0
require_vm_copy_temp[num_change] = 0;
}
//开启放置
for (int zreo_i = 0; zreo_i < max_serve_py*num_vm; zreo_i++) {
result_save_temp1[zreo_i] = 0;
result_save_temp2[zreo_i] = 0;
}

//printf("\n [");
//for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm_copy[i]);
//printf("]\n");

result_serve_num1 = putVM_greedy_without_seq(require_vm_copy, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_temp1);//原序列
result_serve_num2 = putVM_greedy_without_seq(require_vm_copy_temp, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_temp2);//新序列

//开始评分
score_result1 = get_score(require_vm, require_vm_copy, inputFlavor, opt_target, result_serve_num1);
score_result2 = get_score(require_vm, require_vm_copy_temp, inputFlavor, opt_target, result_serve_num2);

//是否更新序列
if (score_result1 < score_result2) {
score_final = score_result2;
result_serve_num = result_serve_num2;
for (int ii = 0; ii < num_vm; ii++) {
require_vm_copy[ii] = require_vm_copy_temp[ii];
}
}
else {
score_final = score_result1;
result_serve_num = result_serve_num1;
}
}
Temperature -= Step;
//打印
printf("\n当前温度：%f  新样本得分：%f", Temperature, score_final);
}

for (int i = 0; i < num_vm; i++) {  //最终结果赋值
require_vm[i] = require_vm_copy[i];
}

result_serve_num = putVM_greedy(require_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);//原序列


return result_serve_num;
}
*/


/*   根据服务器类型性能和虚拟机需求来定向优化放置方案    */

/*
首先知道
MEM     CPU
flavor1 1 1024         1       1
flavor2 1 2048         2       0.5
flavor3 1 4096         4       0.25
flavor4 2 2048         1       1
flavor5 2 4096         2       0.5
flavor6 2 8192         4       0.25
flavor7 4 4096         1       1
flavor8 4 8192         2       0.5
flavor9 4 16384        4       0.25
flavor10 8 8192        1       1
flavor11 8 16384       2       0.5
flavor12 8 32768       4       0.25
flavor13 16 16384      1       1
flavor14 16 32768      2       0.5
flavor15 16 65536      4       0.25

服务器的类型性能：
举例 56  128           2       0.5
如果是按CPU优化：56/128 = 0.5 那些 = 0.5的虚拟机 是完美对象     > 0.5的虚拟机 是优化超分对象    < 0.5的虚拟机 是超分对象
如果是按MEM优化：56/128 = 0.5 那些 = 0.5的虚拟机 是完美对象     < 0.5的虚拟机 是优化超分对象    > 0.5的虚拟机 是超分对象
// 如果是按MEM优化：128/56 = 2.0 那些 = 2.0的虚拟机 是完美对象     > 2.0的虚拟机 是优化超分对象    < 2.0的虚拟机 是超分对象


inputServer.cpu_core_num/inputServer.mem_size：服务器（优化cpu）系数。
// inputServer.mem_size/inputServer.cpu_core_num：服务器（优化mem）系数。

inputFlavor.cpu_core_num/inputFlavor.mem_size：虚拟机（优化cpu）系数。
// inputFlavor.mem_size/inputFlavor.cpu_core_num：虚拟机（优化mem）系数。

虚拟机（优化cpu）系数 = 服务器（优化cpu）系数：   【完美对象】          不会成为超分的限制因素：增加或者减少都OK
虚拟机（优化cpu）系数 > 服务器（优化cpu）系数：   【超分对象】          成为超分的唯一限制因素：相对而言尽可能减少会好点
虚拟机（优化cpu）系数 < 服务器（优化cpu）系数：   【优化超分对象】      作用：用来优化超分对象，增加或者减少都OK，如果超分对象很严重，那么相对增加优化超分对象就会缓解超分现象


所有的情况综合考虑：
（1）先考虑cpu的优化：
A：服务器（优化cpu）系数 :(0,0.25] 【不可能出现超分的情况】 ----------- 【可行解系方案1】
B：服务器（优化cpu）系数 :(1,+max) 【必定出现超分的情况】 ----------- 【可行解系方案2】
C：服务器（优化cpu）系数 :(0.25,1] 【可能出现超分的情况】(再根据虚拟机性能进行判别) ----------- 【可行解系方案3】
note：服务器（优化mem）系数越大，出现的超分的可能性越大。
（2）先考虑mem的优化：
A：服务器（优化mem）系数 :[1,+max) 【不可能出现超分的情况】 ----------- 【可行解系方案1】
B：服务器（优化mem）系数 :(0,0.25) 【必定出现超分的情况】 ----------- 【可行解系方案2】
C：服务器（优化mem）系数 :[0.25,1) 【可能出现超分的情况】(再根据虚拟机性能进行判别) ----------- 【可行解系方案3】
note：服务器（优化mem）系数越小，出现的超分的可能性越大。

【可行解系方案1】：因为必定不超分，一般情况下，利用率已经很高，预测数据上下浮动都ok。
【可行解系方案2】：因为必定超分且没有任何方案可以解决，一般情况下，利用率已经很低，对【超分对象】虚拟机（优化cpu）系数排序，预测数据尽可能减少好点。
***  重点考虑对象  ***
【可行解系方案3】：可能出现超分情况，但我们要尽可能解决掉这部分超分情况！！！【超分对象】和【优化超分对象】尽可能互补完成。
怎么解决？ 例如：预测结果：【a,b,c,d,e,f,g,h】 划分为3类【完美对象】【超分对象】【优化超分对象】
假如划分如下：【完美对象 e，f】 【超分对象a，b，h】 【优化超分对象c，d，g】

块类型的大小规模分析：
【完美块的大小规模】：如果这块规模大，没有多大影响。
【超分块的大小规模】：这块如果规模大就很严重。 坑的大小！！！尽可能缩小坑？
【优化超分块的大小规模】：填坑专用！！！尽可能去加大力度？
【超分块的大小规模】和【优化超分块的大小规模】怎么样可以尽可能优化？？？

【完美对象】：在增加过程中，不会给超分带来负担，随意增减都OK。
【超分对象】：在增加过程中，一定会给超分带来负担，尽可能少会好点。
【优化超分对象】：在增加过程中，会缓解超分的负担，尽可能多会好点。


*/

/*
方法：定向遍历方案
输入:	*require_vm		虚拟机需求
num_vm			虚拟机类型总数
resource_pm     物理机性能
opt_target		优化目标 0:CPU 1:MEM
*inputFlavor	虚拟机性能表
*result_save	服务器数据表(结果)
输出:	创建的服务器个数
*/
int putVM_directTraversal(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	//分析部分
	double ratio_pm = (double)(resource_pm.cpu_core_num) / resource_pm.mem_size;
	double *ratio_vm = new double[inputcontrol.flavorMaxnum];

	// 肯定需要开的服务器数量。
	int SERVER_NUM_PREDICT = 0;

	//使用方案的分类
	int flag_method = 1;  // 默认是方案1
						  // 【不可能出现超分的情况】  ------  1
						  // 【必定出现超分的情况】    ------  2
						  // 【可能出现超分的情况】    ------  3


	int *vm_master = new int[inputcontrol.flavorMaxnum];  // 存放完美对象的容器
	int *vm_good = new int[inputcontrol.flavorMaxnum]; // 存放优化超分对象的容器
	int *vm_bad = new int[inputcontrol.flavorMaxnum]; // 存放超分对象的容器

	int count_master = 0; // 完美对象的计数器
	int count_good = 0;  // 优化超分对象的计数器
	int count_bad = 0; // 超分对象的计数器

	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
		ratio_vm[i] = (double)inputFlavor[i].cpu_core_num / inputFlavor[i].mem_size;

		printf("%f  %f\n", ratio_vm[i], ratio_pm);
		//分为3类
		//如果是按CPU优化：56 / 128 = 0.5 那些 = 0.5的虚拟机 是完美对象     > 0.5的虚拟机 是优化超分对象    < 0.5的虚拟机 是超分对象
		//如果是按MEM优化：56 / 128 = 0.5 那些 = 0.5的虚拟机 是完美对象     < 0.5的虚拟机 是优化超分对象    > 0.5的虚拟机 是超分对象
		if (opt_target == 0) //cpu
		{
			// 对于优化cpu ratio_vm越大越好。
			if (ratio_vm[i] == ratio_pm)  vm_master[count_master++] = i;
			if (ratio_vm[i] > ratio_pm)  vm_good[count_good++] = i;
			if (ratio_vm[i] < ratio_pm)  vm_bad[count_bad++] = i;
		}
		if (opt_target == 1) //mem
		{
			// 对于优化mem ratio_vm越小越好。
			if (ratio_vm[i] == ratio_pm)  vm_master[count_master++] = i;
			if (ratio_vm[i] < ratio_pm)  vm_good[count_good++] = i;
			if (ratio_vm[i] > ratio_pm)  vm_bad[count_bad++] = i;
		}

	}

	long *vm_size = new long[inputcontrol.flavorMaxnum]; // 类型块的大小
	long *vm_size_temp = new long[inputcontrol.flavorMaxnum]; // 类型块的大小
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
		if (opt_target == 0) //cpu
		{
			vm_size[i] = (long)(require_vm[i] * inputFlavor[i].cpu_core_num);
		}
		if (opt_target == 1) //mem
		{
			vm_size[i] = (long)(require_vm[i] * inputFlavor[i].mem_size);
		}
	}
	// 我们用vm_size来评估优化的性价比，优化的性价比越高，放在前面提前优化。



	//对vm_size进行排序
	int *vm_size_count = new int[inputcontrol.flavorMaxnum]; // 类型块的大小排序的序号！！！
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  vm_size_count[i] = i;
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  vm_size_temp[i] = vm_size[i];
	printf("块原始序列：");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)   printf(" %d ", vm_size[i]);	// 打印排序结果
	printf("\n");

	/*
	// 打印排序结果
	for (int i = 0; i < count_good; i++)
	{
	printf(" %d ", vm_size[i]);
	}

	printf("\n");
	// 打印排序结果
	for (int i = 0; i < count_good; i++)
	{
	printf(" %d ", vm_good[i]);
	}
	printf("\n");
	*/

	// 排序，大的放在前面
	int temp, temp_size;
	if (count_master > 1)
	{
		for (int i = 0; i < count_master - 1; i++)
		{
			for (int j = i + 1; j < count_master; j++)
			{
				if (vm_size[j] > vm_size[i])
				{
					temp = vm_master[i];
					vm_master[i] = vm_master[j];
					vm_master[j] = temp;

					temp_size = vm_size[j];
					vm_size[j] = vm_size[i];
					vm_size[i] = temp_size;
				}
				if (vm_size[j] == vm_size[i] && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num)
				{
					temp = vm_master[i];
					vm_master[i] = vm_master[j];
					vm_master[j] = temp;

					temp_size = vm_size[j];
					vm_size[j] = vm_size[i];
					vm_size[i] = temp_size;
				}
			}
		}
	}

	if (count_good > 1)
	{
		for (int i = 0; i < count_good - 1; i++)
		{
			for (int j = i + 1; j < count_good; j++)
			{
				if (vm_size[j] > vm_size[i])
				{
					temp = vm_good[i];
					vm_good[i] = vm_good[j];
					vm_good[j] = temp;

					temp_size = vm_size[j];
					vm_size[j] = vm_size[i];
					vm_size[i] = temp_size;
				}
				if (vm_size[j] == vm_size[i] && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num)
				{
					temp = vm_good[i];
					vm_good[i] = vm_good[j];
					vm_good[j] = temp;

					temp_size = vm_size[j];
					vm_size[j] = vm_size[i];
					vm_size[i] = temp_size;
				}
			}
		}
	}

	if (count_bad > 1)
	{
		for (int i = 0; i < count_bad - 1; i++)
		{
			for (int j = i + 1; j < count_bad; j++)
			{
				if (vm_size[j] > vm_size[i])
				{
					temp = vm_bad[i];
					vm_bad[i] = vm_bad[j];
					vm_bad[j] = temp;

					temp_size = vm_size[j];
					vm_size[j] = vm_size[i];
					vm_size[i] = temp_size;
				}
				if (vm_size[j] == vm_size[i] && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num)
				{
					temp = vm_bad[i];
					vm_bad[i] = vm_bad[j];
					vm_bad[j] = temp;

					temp_size = vm_size[j];
					vm_size[j] = vm_size[i];
					vm_size[i] = temp_size;
				}
			}
		}
	}

	printf("块排序序列：");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  	printf(" %d ", vm_size[i]);
	printf("\n");

	printf("块还原序列：");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  vm_size[i] = vm_size_temp[i];
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  	printf(" %d ", vm_size[i]);
	printf("\n");

	// 打印排序结果
	printf("good:");
	for (int i = 0; i < count_good; i++)
	{
		printf(" %d ", vm_good[i]);
	}
	printf("\n");

	printf("master:");
	for (int i = 0; i < count_master; i++)
	{
		printf(" %d ", vm_master[i]);
	}
	printf("\n");

	printf("bad:");
	for (int i = 0; i < count_bad; i++)
	{
		printf(" %d ", vm_bad[i]);
	}
	printf("\n");


	int *vm_opt_priority = new int[inputcontrol.flavorMaxnum]; // 首先先确定优先优化的目标，（序号）排在前面！！！，默认首先优化
	int *vm_put_priority = new int[inputcontrol.flavorMaxnum]; // 放置部分的优先放置顺序！！！
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  vm_put_priority[i] = i;   //放置部分的初始化。

																				   // 方案一 ：优先 good -- bad -- master
	for (int i = 0; i < count_good; i++)  vm_opt_priority[i] = vm_good[i];
	for (int i = 0; i < count_bad; i++)  vm_opt_priority[i + count_good] = vm_bad[i];
	for (int i = 0; i < count_master; i++)  vm_opt_priority[i + count_bad + count_good] = vm_master[i];
	print_data(vm_opt_priority, 1);

	// 方案二：优先 bad -- good -- master
	/*for (int i = 0; i < count_bad; i++)  vm_opt_priority[i] = vm_bad[i];
	for (int i = 0; i < count_good; i++)  vm_opt_priority[i + count_bad] = vm_good[i];
	for (int i = 0; i < count_master; i++)  vm_opt_priority[i + count_bad + count_good] = vm_master[i];*/

	//方案三；优先

	print_data(vm_opt_priority, 1);

	int population_max_num = 330000;	 // require_vm 定向可行解的最大数量，放置超时.实际是3000
	int population_num = 1; //实际可行解的数量
	int *vm_opt_range = new int[inputcontrol.flavorMaxnum]; // 优化的范围（对应优化优先级）
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  vm_opt_range[i] = 1;  // 初始化
	int opt_range_count = 0;
	while (population_num < population_max_num)
	{
		population_num = 1;
		vm_opt_range[opt_range_count++] += 1;
		if (opt_range_count == inputcontrol.flavorMaxnum)   opt_range_count = 0;
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  population_num *= vm_opt_range[i];
		// print_data(vm_opt_range, 1);
	}
	print_data(vm_opt_range, 1);

	int *vm_opt_bit = new int[inputcontrol.flavorMaxnum];  // 优化范围确定之后
	vm_opt_bit[0] = 1;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)  vm_opt_bit[i] = vm_opt_bit[i - 1] * vm_opt_range[i - 1];
	print_data(vm_opt_bit, 1);

	// 一组可行解到多组可行解的演变
	int **result_predict_super = new int *[population_num];
	for (int i = 0; i < population_num; i++)    result_predict_super[i] = new int[inputcontrol.flavorMaxnum];

	int AAAA = 0;  //对当前值AAAA的解析！！！
	int BBBB = 0;
	int FLAG = 1;
	for (int i = 0; i < population_num; i++)
	{
		AAAA = i;
		for (int j = inputcontrol.flavorMaxnum - 1; j >= 0; j--)
		{
			BBBB = AAAA / vm_opt_bit[j];
			AAAA = AAAA - BBBB * vm_opt_bit[j];
			for (int aa = 0; aa < count_bad; aa++)
			{
				if (j == vm_bad[aa])
				{
					BBBB = (-1)*BBBB;
					break;
				}
			}
			/*for (int aa = 0; aa < count_master; aa++)
			{
			if (j == vm_master[aa])
			{
			BBBB = (int)(BBBB*FLAG*0.5 + 0.5*FLAG);
			FLAG = FLAG*(-1);
			break;
			}
			}*/

			result_predict_super[i][j] = require_vm[j] + BBBB;
			if (result_predict_super[i][j] < 0)   result_predict_super[i][j] = 0;

		}
	}
	// for (int i = 0; i < population_num; i++)   print_data(result_predict_super[i],1);


	// 针对多组解的多种放置方案！！！
	int max_serve_py = MAX_SERVER_NUM;  // 服务器最大的个数!!!
	int EPISODE_MAX_NUM = 5000;
	int **result_save_super = new int *[EPISODE_MAX_NUM];  //每一轮执行5000个
	for (int i = 0; i < EPISODE_MAX_NUM; i++)    result_save_super[i] = new int[max_serve_py*inputcontrol.flavorMaxnum];
	
	//开辟的放置迭代空间
	for (int i = 0; i < EPISODE_MAX_NUM; i++)
	{
		for (int j = 0; j < max_serve_py*inputcontrol.flavorMaxnum; j++)
		{
			result_save_super[i][j] = 0;
		}
	}

	// flag_method 放置顺序方法的选择


	// 【不可能出现超分的情况】 【方案2】(根本不需要优化，给定的虚拟机和服务器是完美的)
	if (count_bad == 0)  flag_method = 2;

	// 【必定出现超分的情况】 【方案3】(给定的虚拟机和服务器是最差的情况，不存在可以优化项)
	if (count_bad != 0 && count_good == 0)  flag_method = 3;

	// if (count_bad != 0 && count_good != 0)  flag_method = 4;
	// for (int i = 0; i < inputcontrol.flavorMaxnum;i++)   printf("\n %f ", ratio_vm[i]);
	// 贪婪放置-顺序的选择  计算完成后，require_vm和inputFlavor 的顺序都改变了，并且vm_put_priority记录了是怎么改变的！！！
	// putVM_seq_vmsize(require_vm, resource_pm, opt_target, inputFlavor,  vm_size, ratio_vm, ratio_vm_diff,vm_put_priority, flag_method);

	// 按照ratio导向的排序！！！
	putVM_seq_ratio_guided(require_vm, resource_pm, opt_target, inputFlavor, ratio_vm, vm_put_priority, flag_method);


	int *result_serve_num = new int[EPISODE_MAX_NUM];  // 保存返回的服务器的数量
	double *score_result = new double[EPISODE_MAX_NUM]; // 保存得分情况
	Flavor *inputFlavor_data = new Flavor[inputcontrol.flavorMaxnum];
	int return_serve_num; // 最终返回的服务器数量 
	double FINAL_score_result = 0;  //最高分数

    // 最优的
	for (int AAA = 0; AAA < population_num / EPISODE_MAX_NUM; AAA++)
	{
		for (int i = 0; i < EPISODE_MAX_NUM; i++)
		{

			//计算至少需要开的服务器数量。
			long EEEEE = 0;
			for (int aa = 0; aa < inputcontrol.flavorMaxnum; aa++)  EEEEE += result_predict_super[i][aa] * inputFlavor[aa].cpu_core_num;
			SERVER_NUM_PREDICT = EEEEE / resource_pm.cpu_core_num;

			EEEEE = 0;
			for (int aa = 0; aa < inputcontrol.flavorMaxnum; aa++)  EEEEE += result_predict_super[i][aa] * inputFlavor[aa].mem_size;
			if (SERVER_NUM_PREDICT > EEEEE / resource_pm.mem_size)  SERVER_NUM_PREDICT = EEEEE / resource_pm.mem_size;
			// printf("SERVER_NUM_PREDICT: %d\n",SERVER_NUM_PREDICT);
			result_serve_num[i] = putVM_greedy_ratio_guided(result_predict_super[i+ EPISODE_MAX_NUM*AAA], SERVER_NUM_PREDICT, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i], vm_put_priority);


			//result_serve_num[i] = putVM_dynamicpro_ratio_guided(result_predict_super[i], GGGGG, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i], vm_put_priority);

			// result_serve_num[i] = putVM_greedy_AAA_vmsize(result_predict_super[i], inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i], vm_put_priority);
			//putVM_seq_vmsize(require_vm, num_vm, resource_pm, opt_target, inputFlavor, result_save, vm_size);
			//putVM_seq(require_vm, num_vm, resource_pm, opt_target, inputFlavor, result_save);
			//print_data(result_predict_super[i], 1);
			//result_serve_num[i] = putVM_greedy_AAA(result_predict_super[i], inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i]);

			//矫正
			//result_serve_num[i] = putVM_correct_vmsize(result_predict_super[i + EPISODE_MAX_NUM*AAA], inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i], result_serve_num[i]);
			//result_serve_num[i] = putVM_correct(result_predict_super[i], inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i], result_serve_num[i]);

			//print_data(require_vm, 1);
			//print_data(result_predict_super[i], 1);
			//得分
			score_result[i] = get_score(require_vm, result_predict_super[i + EPISODE_MAX_NUM*AAA], inputFlavor, opt_target, result_serve_num[i]);

		}
		
		//完成迭代之后，看看有没有好的分数
		for (int i = 0; i < EPISODE_MAX_NUM; i++)
		{
			if (score_result[i] > FINAL_score_result)
			{
				FINAL_score_result = score_result[i];
				return_serve_num = result_serve_num[i]; //返回的服务器数量。													  
				for (int m = 0; m < inputcontrol.flavorMaxnum; m++)    require_vm[m] = result_predict_super[i + EPISODE_MAX_NUM*AAA][m];// 进行复制！！！
				for (int m = 0; m < return_serve_num * inputcontrol.flavorMaxnum; m++)   result_save[m] = result_save_super[i][m];
			}
		}

		// 放置完成之后将结果清空！！！
		for (int m = 0; m < EPISODE_MAX_NUM; m++)
		{
			for (int n = 0; n < max_serve_py*inputcontrol.flavorMaxnum; n++)
			{
				result_save_super[m][n] = 0;  //放置清空
			}
			score_result[m] = 0.0; //得分结果清空
			result_serve_num[m] = 0;  //服务器数量
		}

		//end
	}

	// 打印模型预测结果
	printf("[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
	printf("]\n");
	printf("FINAL case score：%f\n", FINAL_score_result);

	return return_serve_num;

}



/*
评分公式
*/
double get_score(int *result_predict_real, int *result_predict_temp, Flavor *inputFlavor_temp, int cpuormem, int serve_num)
{
	// 左边
	double sum_result_predict_real = 0;  // 分母右边
	double sum_result_predict_temp = 0;  // 分母左边
	double sum_result_predict_diff = 0;  // 上边分子

										 //右边
	double sum_source_fenzi_num = 0; //右边分子！！！
	double sum_source_fenmu_num = 0; //右边分母！！！

	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
		sum_result_predict_real += (result_predict_real[i] * result_predict_real[i]);
		sum_result_predict_temp += (result_predict_temp[i] * result_predict_temp[i]);
		sum_result_predict_diff += ((result_predict_real[i] - result_predict_temp[i]) * (result_predict_real[i] - result_predict_temp[i]));
	}

	sum_result_predict_real = sqrt(sum_result_predict_real / inputcontrol.flavorMaxnum);
	sum_result_predict_temp = sqrt(sum_result_predict_temp / inputcontrol.flavorMaxnum);
	sum_result_predict_diff = sqrt(sum_result_predict_diff / inputcontrol.flavorMaxnum);

	if (cpuormem == 0)  //cpu
	{
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
		{
			sum_source_fenzi_num += (result_predict_temp[i] * inputFlavor_temp[i].cpu_core_num);
		}
		sum_source_fenmu_num = serve_num * inputServer.cpu_core_num;
	}
	else if (cpuormem == 1)  // mem
	{
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
		{
			sum_source_fenzi_num += (result_predict_temp[i] * inputFlavor_temp[i].mem_size);
		}
		sum_source_fenmu_num = serve_num * inputServer.mem_size;
	}
	double aaaa = ((1 - sum_result_predict_diff / (sum_result_predict_real + sum_result_predict_temp))*(sum_source_fenzi_num / sum_source_fenmu_num));
	return  aaaa;
}


/*
整理输出函数
result_file_local:本地输出数组地址
vm_predict 虚拟机预测结果
vm_require_num 虚拟服务器数量
result_py_save物理服务器存储数据
py_require_num 物理服务器数量
inputFlavor 输入文件中虚拟机的性能结构体:名称,cpu,mem
vm_class_num 虚拟机种类数量
*/
void write_output_to_result(char *result_file_local, int *vm_predict, int vm_require_num, int *result_py_save, int py_require_num, Flavor *inputFlavor, int vm_class_num) {
	int result_point = 0;
	/*虚拟机输出*/
	result_point = input_a_int_num(result_file_local, result_point, vm_require_num); //第一行虚拟机总数
	result_file_local[result_point++] = '\n';   //第一行回车
	for (int i = 0; i < vm_class_num; i++) {
		result_file_local[result_point++] = 'f';
		result_file_local[result_point++] = 'l';
		result_file_local[result_point++] = 'a';
		result_file_local[result_point++] = 'v';
		result_file_local[result_point++] = 'o';
		result_file_local[result_point++] = 'r';
		result_point = input_a_int_num(result_file_local, result_point, inputFlavor[i].flavor_name); //输入名称
		result_file_local[result_point++] = ' ';
		result_point = input_a_int_num(result_file_local, result_point, vm_predict[i]); //输入数量
		result_file_local[result_point++] = '\n';
	}
	/*物理机输出*/
	result_file_local[result_point++] = '\n'; //空行
	result_point = input_a_int_num(result_file_local, result_point, py_require_num); //第一行物理机总数
	result_file_local[result_point++] = '\n';   //第一行回车
	for (int i = 0; i < py_require_num; i++) {
		result_point = input_a_int_num(result_file_local, result_point, (i + 1)); //物理机编号
		for (int j = 0; j < vm_class_num; j++) {
			if (result_py_save[i*vm_class_num + j] != 0) { //第i个物理机中第j个虚拟机分配不为0,则输出
				result_file_local[result_point++] = ' ';
				result_file_local[result_point++] = 'f';
				result_file_local[result_point++] = 'l';
				result_file_local[result_point++] = 'a';
				result_file_local[result_point++] = 'v';
				result_file_local[result_point++] = 'o';
				result_file_local[result_point++] = 'r';
				result_point = input_a_int_num(result_file_local, result_point, inputFlavor[j].flavor_name); //输入名称
				result_file_local[result_point++] = ' ';
				result_point = input_a_int_num(result_file_local, result_point, result_py_save[i*vm_class_num + j]); //输入数量
			}
		}
		//result_file_local[result_point++] = '\n';
		if (i < py_require_num - 1) { //限制最后一行不用换行
			result_file_local[result_point++] = '\n';
		}
	}
}



// 对数组归一化处理，并完成相乘！！！
double *normal_result(double *data, double *data_a, int data_num)
{
	int sum_data = 0;
	double *temp = new double[data_num];
	for (int i = 0; i < data_num; i++)
	{
		if (data[i] < 0)  sum_data += (data[i] * (-1));
		else  sum_data += data[i];
	}
	for (int i = 0; i < data_num; i++)
	{
		if (data[i] > 0)   temp[i] = ((data[i]) / sum_data)*data_a[i];
		else
		{
			temp[i] = ((data[i] * (-1)) / sum_data)*data_a[i];
			// temp[i] = 0;
		}
	}



	return temp;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//你要完成的功能总入口
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	// 需要输出的内容

	char result_file[20000] = "0";
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	printf("\n**************************************************************************\n\n");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/************   模型输入部分开始  ************/
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// if(data_num < 5000) CYC_SAMPLE = 7;


	/****************  参数司令部  *******************/

	int is_valid_div = 0; // 是否去除无效天数进行有效分割？ 0--不去除   1--去除

	int is_noise_oneday = 1; // 是否单天去除？  0--不去出   1--去除

	int is_noise_happyday = 1; // 是否单天去除？  0--不去出   1--去除
							   //两阶差分参数
	double diff_parse[4] = { 0, 1,1,-1.80 };
	/*************************************************/
	//月份的初始化
	month_init();
	// for (int i = 0; i< 13; i++)   printf("%d %d %d \n", month[i].month_name, month[i].month_day_num, month[i].month_day_total_num);

	//训练文件的有效分割信息
	int valid_div_data[200] = { 0 };
	////////////////////////////  输入文件的处理 /////////////////////////////////////////////
	traincontrol.trainfileLinenum = data_num;
	int info_num = 0;
	for (int i = 0; i < MAX_INFO_NUM; i++) {
		if (info[i][4] == '-') {
			info_num = i + 2;
			break;
		}
	}
	inputcontrol.inputfileLinenum = info_num;

	read_flavor_class_num(info); //读取虚拟机种类数量

	Flavor *inputFlavor = new Flavor[inputcontrol.flavorMaxnum];
	do_input_file(info, inputFlavor);  //读取输入文件性质
									   // CYC_SAMPLE = inputcontrol.intervalTime + 1;


									   /************************************  对服务器和虚拟机规格的分析部分  *************************************/





									   /**********************************  END  ******************************************************************/




									   ////////////////////////////  训练文件的处理 //////////////////////////

	read_time_diff(data, data_num); // 读取训练文件性质


	if (is_valid_div)
	{
		traincontrol.sample_num = do_train_file_valid_div(data, valid_div_data);  // 首先对训练文件进行有效分割！！去除不存在的天数！！
		printf("traincontrol.sample_num: %d\n", traincontrol.sample_num);
		for (int i = 0; i < traincontrol.sample_num; i++)
		{
			printf("valid_div_data[%d]: %d\n", i, valid_div_data[i]);
		}
	}
	else
	{
		traincontrol.sample_num = (traincontrol.endTime - traincontrol.startTime + 1) / CYC_SAMPLE;  //样本的个数！！！
		traincontrol.sample_num_noise = (traincontrol.endTime - traincontrol.startTime + 1) / CYC_SAMPLE_NOISE;  //样本的个数！！！
																												 // printf("traincontrol.sample_num: %d\n", traincontrol.sample_num);
		for (int i = 0; i < traincontrol.sample_num_noise; i++)
		{
			valid_div_data[i] = (traincontrol.endTime - i * CYC_SAMPLE_NOISE);
			printf("valid_div_data[%d]: %d\n", i, valid_div_data[i]);
		}
	}

	// 动态定义样本数组
	int *trainfileFlavordata_temp = new int[inputcontrol.flavorMaxnum * traincontrol.sample_num_noise];

	int *trainfileFlavordata = new int[inputcontrol.flavorMaxnum * traincontrol.sample_num];
	int *trainfileFlavordata_diff = new int[inputcontrol.flavorMaxnum * (traincontrol.sample_num - 1)];
	int *trainfileFlavordata_diff_diff = new int[inputcontrol.flavorMaxnum * (traincontrol.sample_num - 2)];
	int *trainfileFlavordata_diff_diff_DIFF = new int[inputcontrol.flavorMaxnum * (traincontrol.sample_num - 3)];
	//数据初始化
	for (int i = 0; i < inputcontrol.flavorMaxnum * traincontrol.sample_num; i++) {
		trainfileFlavordata[i] = 0;
	}
	for (int i = 0; i < inputcontrol.flavorMaxnum * traincontrol.sample_num_noise; i++) {
		trainfileFlavordata_temp[i] = 0;
	}

	do_train_file(data, inputFlavor, trainfileFlavordata_temp, valid_div_data);  //统计训练文件


	print_data(trainfileFlavordata_temp, traincontrol.sample_num_noise); // 打印单天抽取的数据
																		 // 节假日去噪声！！！
	if (is_noise_happyday)
	{
		do_noise_happyday(trainfileFlavordata_temp, valid_div_data, traincontrol.sample_num_noise);

	}

	print_data(trainfileFlavordata_temp, traincontrol.sample_num_noise); // 打印单天抽取的数据

	if (is_noise_oneday)
	{
		do_noise_midfilter(trainfileFlavordata_temp, inputFlavor);
		// print_data(trainfileFlavordata_temp, traincontrol.sample_num_noise); // 打印单天抽取的数据
	}

	// 去噪之后！将数据还原为周期的样本
	for (int i = 0; i < traincontrol.sample_num; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			for (int k = 0; k < CYC_SAMPLE; k++)
			{
				trainfileFlavordata[i*inputcontrol.flavorMaxnum + j] += trainfileFlavordata_temp[inputcontrol.flavorMaxnum*((traincontrol.endTime - traincontrol.startTime + 1) % CYC_SAMPLE + i*CYC_SAMPLE + k) + j];
			}
		}
	}
	// print_data(trainfileFlavordata, traincontrol.sample_num);
	get_trainfileFlavordata_diff(trainfileFlavordata, trainfileFlavordata_diff, traincontrol.sample_num - 1);
	get_trainfileFlavordata_diff(trainfileFlavordata_diff, trainfileFlavordata_diff_diff, traincontrol.sample_num - 2);
	// print_data(trainfileFlavordata_diff, traincontrol.sample_num - 1);
	// print_data(trainfileFlavordata_diff_diff, traincontrol.sample_num - 2);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/************   模型去噪部分开始  ************/
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	print_data(trainfileFlavordata, traincontrol.sample_num);

	print_data(trainfileFlavordata_diff, traincontrol.sample_num - 1);

	print_data(trainfileFlavordata_diff_diff, traincontrol.sample_num - 2);

	// print_data(trainfileFlavordata);
	// do_noise_avgfilter(trainfileFlavordata);


	// print_data(trainfileFlavordata);

	// printf("do_train_file function has been passed.\n");


	/////////////////////////////显示统计结果
	/*for (int i = 0; i < traincontrol.sample_num; i++) {
	cout << "\n";
	for (int j = 0; j < inputcontrol.flavorMaxnum; j++) {
	cout << trainfileFlavordata[i*inputcontrol.flavorMaxnum + j] << " ";
	}
	}
	cout << "\n";*/
	////////////////////////////////// 去燥部分结束 //////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/************   模型预测部分开始  ************/
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int *result_predict = new int[inputcontrol.flavorMaxnum];
	int preidct_sample_num = inputcontrol.endTime - inputcontrol.startTime + 1;

	/////////////////////////////// Wmean 预测部分开始 ////////////////////////////////////////////////////////////////////////////////

	//result_predict = predict_run_Wmean(trainfileFlavordata, inputcontrol.flavorMaxnum, traincontrol.sample_num, preidct_sample_num);

	/////////////////////////////// Wmean 预测部分开始 ////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////// 线性预测部分开始 ////////////////////////////////////////////////////////////////////////////////

	double *result_predict_double = new double[inputcontrol.flavorMaxnum];
	double *result_predict_diff_double = new double[inputcontrol.flavorMaxnum];
	double *result_predict_diff_diff_double = new double[inputcontrol.flavorMaxnum];
	double *result_predict_diff_diff_DIFF_double = new double[inputcontrol.flavorMaxnum];
	double *result_ori_double = new double[inputcontrol.flavorMaxnum];
	// int preidct_sample_num = inputcontrol.endTime - inputcontrol.startTime + 1;
	//result_predict = predict_run_RLS(trainfileFlavordata, inputcontrol.flavorMaxnum, traincontrol.sample_num);





	print_data(trainfileFlavordata_diff, traincontrol.sample_num - 1);

	print_data(trainfileFlavordata_diff_diff, traincontrol.sample_num - 2);

	print_data(trainfileFlavordata_diff_diff_DIFF, traincontrol.sample_num - 3);




	diff_parse[0] = 0;
	diff_parse[1] = 1;
	diff_parse[2] = 1;
	diff_parse[3] = -1.8;
	double diff_parse1 = 0;

	/*******************************/


	// 第一次 ！！！
	result_predict_double = predict_run_liner(trainfileFlavordata, inputcontrol.flavorMaxnum, traincontrol.sample_num, preidct_sample_num);
	// 打印预测结果
	printf("predict result : \n[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %f ", result_predict_double[i]);
	printf("]\n");

	// 第二次 ！！！
	result_predict_diff_double = predict_run_liner(trainfileFlavordata_diff, inputcontrol.flavorMaxnum, traincontrol.sample_num - 1, preidct_sample_num);
	// 打印预测结果
	printf("predict diff result : \n[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %f ", result_predict_diff_double[i]);
	printf("]\n");

	// 第三次 ！！！
	result_predict_diff_diff_double = predict_run_liner(trainfileFlavordata_diff_diff, inputcontrol.flavorMaxnum, traincontrol.sample_num - 2, preidct_sample_num);
	// 打印预测结果

	// 第4次
	result_predict_diff_diff_DIFF_double = predict_run_liner(trainfileFlavordata_diff_diff_DIFF, inputcontrol.flavorMaxnum, traincontrol.sample_num - 3, preidct_sample_num);


	result_predict_diff_diff_double = normal_result(result_predict_diff_double, result_predict_diff_diff_double, inputcontrol.flavorMaxnum);

	printf("predict diff diff result : \n[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %f ", result_predict_diff_diff_double[i]);
	printf("]\n");



	printf("predict diff diff DIFF result : \n[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %f ", result_predict_diff_diff_DIFF_double[i]);
	printf("]\n");




	printf("predict diff diff result NORMAL : \n[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %f ", result_predict_diff_diff_double[i]);
	printf("]\n");


	//最近一组样本的数据!!!!!
	result_ori_double = predict_get_ori_data(trainfileFlavordata, inputcontrol.flavorMaxnum, traincontrol.sample_num, preidct_sample_num);
	printf("the last time data : [");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %f ", result_ori_double[i]);
	printf("]\n");



	// 更新预测结果
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
		// +1原因： 尽可能多放置一个虚拟机，满足客户的需求！！！
		result_predict[i] = (int)round_my(result_ori_double[i] * diff_parse[0] + result_predict_double[i] * diff_parse[1] + result_predict_diff_double[i] * diff_parse[2] + result_predict_diff_diff_double[i] * diff_parse[3] +
			result_predict_diff_diff_DIFF_double[i] * diff_parse1 + 0.2);


		if (result_predict[i]<0) {
			result_predict[i] = 0;
		}

	}


	//////////////////////////////// 线性预测部分结束 ///////////////////////////////////////////////////////


	// 打印模型预测结果
	printf("\npredict final result : [");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", result_predict[i]);
	printf("]\n");

	/* test */
	int AAAAA[5] = { 45,17,85,48,60 };
	printf("REAL    final result : [");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", AAAAA[i]);
	printf("]\n");

	int BBBBB[5] = { 59,17,73,60,66 };
	printf("REAL    final result : [");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", BBBBB[i]);
	printf("]\n");

	/**********************************/

	//////////////////////////////// 模型预测部分结束 /////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/************   模型放置部分开始  ************/
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//动态的创建服务器的最大数量！！！

	int max_serve_py = MAX_SERVER_NUM;
	int *result_save = new int[max_serve_py*inputcontrol.flavorMaxnum];
	for (int i = 0; i < max_serve_py*inputcontrol.flavorMaxnum; i++) {
		result_save[i] = 0;
	}
	int num_of_Serve_py;//所需物理服务器数量
	int num_of_Serve_vm = 0;  //预测的虚拟服务器数量


							  /***********************  退火算法(Simulate Anneal Arithmetic)+ 贪心放置 + 自评得分 方案   *************************/
							  /*if (inputcontrol.flavorMaxnum > 1)
							  {
							  num_of_Serve_py = putVM_score_SAA_greedy(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }
							  else
							  {
							  num_of_Serve_py = putVM_greedy(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }

							  // 是否矫正？（小矫正）
							  num_of_Serve_py = putVM_correct(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save, num_of_Serve_py);
							  */
							  // 打印模型预测结果
							  /*printf("\npredict final result : [");
							  for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", result_predict[i]);
							  printf("]\n");*/
							  /********************************  END  ****************************************************/




							  /***********************  种群竞争 + 贪心 + 调整 + 自评得分 方案   *************************/
							  /*if (inputcontrol.flavorMaxnum > 1)
							  {
							  num_of_Serve_py = putVM_score_compet_greedy(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }
							  else
							  {
							  num_of_Serve_py = putVM_greedy(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }*/
							  /********************************  END  ****************************************************/



							  /*****************************  贪心 + 调整 方案   *****************************************/
							  /*num_of_Serve_py = putVM_greedy(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  num_of_Serve_py = putVM_correct(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save, num_of_Serve_py);*/
							  /********************************  END  ****************************************************/


							  /***********************  种群竞争 + 动归 + 调整 + 自评得分 方案   *************************/
							  /*if (inputcontrol.flavorMaxnum > 1)
							  {
							  num_of_Serve_py = putVM_score_compet_dynamicpro(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }
							  else
							  {
							  num_of_Serve_py = putVM_dynamicpro(result_predict, num_of_Serve_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }*/
							  /********************************  END  ****************************************************/



							  /*****************************  动归 + 调整 方案   *****************************************/
							  /*num_of_Serve_py = putVM_dynamicpro(result_predict, num_of_Serve_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  num_of_Serve_py = putVM_correct(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save, num_of_Serve_py); */
							  /********************************  END  ****************************************************/



							  /***********************  遗传 + 动归（贪心） + 调整 + 自评得分 方案   *************************/

							  // num_of_Serve_py = gene_dynamicpro(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  // num_of_Serve_py = gene_greedy(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);

							  /********************************  END  ****************************************************/


							  /***********************   种群竞争 + 动归 方案   *************************/
							  /*if (inputcontrol.flavorMaxnum > 1)
							  {
							  num_of_Serve_py = putVM_compet_dynamicpro(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }
							  else
							  {
							  num_of_Serve_py = putVM_dynamicpro(result_predict, num_of_Serve_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }
							  num_of_Serve_py = putVM_correct(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save, num_of_Serve_py);
							  */
							  /********************************  END  ***********************************/



							  /***********************   种群竞争 + 贪心 方案   *************************/
							  /*if (inputcontrol.flavorMaxnum > 1)
							  {
							  num_of_Serve_py = putVM_compet_greedy(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }
							  else
							  {
							  num_of_Serve_py = putVM_greedy(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }
							  num_of_Serve_py = putVM_correct(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save, num_of_Serve_py);
							  */
							  /********************************  END  ***********************************/



							  /***********************  定向遍历 + 调整 + 自评得分 方案   *************************/
	num_of_Serve_py = putVM_directTraversal(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
	//num_of_Serve_py = putVM_correct(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save, num_of_Serve_py);

	/********************************  END  ****************************************************/




	/*************************  END  **************************************************************************************************/

	num_of_Serve_vm = 0;  //矫正后再次统计预测的虚拟服务器数量
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++) {
		num_of_Serve_vm += result_predict[i];
	}
	print_resource(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save, num_of_Serve_py);
	write_output_to_result(result_file, result_predict, num_of_Serve_vm, result_save, num_of_Serve_py, inputFlavor, inputcontrol.flavorMaxnum);
	printf("\n**************************************************************************\n");

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// 直接调用输出文件的方法输出到指定文件中(ps请注意格式的正确性,如果有解,第一行只有一个数据;第二行为空;第三行开始才是具体的数据,数据之间用一个空格分隔开)
	write_result(result_file, filename);
}





