#include "predict.h"
#include <stdio.h>













///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/******************************
ȫ���������������岿��
*******************************/


Inputcontrol inputcontrol;
Traincontrol traincontrol;
Server inputServer;

// ���ڣ�����
int CYC_SAMPLE = 9;

// ����ȥ�������� Ĭ��Ϊ1
int CYC_SAMPLE_NOISE = 1;

// Ĭ����[31(1) , 28(2) , 31(3) , 30(4) , 31(5) , 30(6) , 31(7) , 31(8) , 30(9) , 31(10) , 30(11) , 31(12) ] = 365�죡
Month month[13];

int month_data[13] = { 0,31 , 28 , 31 , 30 , 31 , 30 , 31, 31 , 30 , 31 , 30 , 31 };

int HAPPYDAY_TOTALNUM = 4;
// ����ڼ���  Ԫ��1.1 1.2 1.3  ��һ5.1 5.2 5.3 ʮһ10.1 10.2 10.3  ˫11 11.11
int happyday[4] = { 1,   121,   274,   315 };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// �·ݵĳ�ʼ��
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

// 10��power_num����
int poww(int power_num) {
	int power_out = 1;
	for (int i = 0; i < power_num; i++) {
		power_out = power_out * 10;
	}
	return power_out;
}

// 2��power_num����
int poww_two(int power_num) {
	int power_out = 1;
	for (int i = 0; i < power_num; i++) {
		power_out = power_out * 2;
	}
	return power_out;
}

/*��������:double���������������int����*/
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
���ַ�����д��һ��int����
result_file_local:��д������
result_point:������ĵڼ�λ��ʼ����
input_num:��������
��������Ϊ�������ֺ�result_point��λ��
*/
int input_a_int_num(char *result_file_local, int  result_point, int input_num) {
	int weishu = 0; //�������ݵ�λ��
	int num_readytowrite = input_num; //copyһ��input_num
	while (num_readytowrite != 0) {
		num_readytowrite /= 10;
		weishu++;
	}
	if (input_num == 0) { //0Ҳ��1λ��
		weishu = 1;
	}
	int weishu_copy = weishu;//���Եݼ�����
	for (int i = result_point; i < result_point + weishu; i++) {
		result_file_local[i] = '0' + input_num / poww(weishu_copy - 1);
		input_num = input_num - (input_num / poww(weishu_copy - 1)) * poww(weishu_copy - 1);
		weishu_copy--;
	}
	result_point += weishu;
	return result_point;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// �����������������
void read_flavor_class_num(char ** const inputfileBuff)
{
	//�����еĴ���
	///!!!!!!!!!!!!!!!
	if (inputfileBuff[2][1] < '0' || inputfileBuff[2][1] > '9')
		inputcontrol.flavorMaxnum = ((inputfileBuff[2][0] - CONV_ASCII_TO_INT));
	else
		inputcontrol.flavorMaxnum = ((inputfileBuff[2][0] - CONV_ASCII_TO_INT) * 10 + (inputfileBuff[2][1] - CONV_ASCII_TO_INT));
	// inputcontrol.flavorMaxnum = 5;
	printf("inputcontrol.flavorMaxnum : %d \n", inputcontrol.flavorMaxnum);
}


// �����ļ��Ĵ���
void do_input_file(char ** const inputfileBuff, Flavor *inputFlavor)
{
	//��Ӳ����������Դ
	int kong1 = 0;  //�ո�1
	int kong2 = 0;  //�ո�2
	int kong3 = 0;  //ĩβ
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



	//��ȡ�������������������
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
		kong1 = 0;  //�ո�1
		kong2 = 0;  //�ո�2
		kong3 = 0;  //ĩβ
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
		// �� MB ת����Ϊ GB



		//printf("kong1 :%d \n", kong1);
		//printf("kong2 :%d \n", kong2);
		//printf("kong3 :%d \n", kong3);

		//printf("mem_size_temp :%d \n", mem_size_temp);
		mem_size_temp = mem_size_temp >> 10;
		inputFlavor[i].mem_size = (int)mem_size_temp;
		//printf(" %d", inputFlavor[i].mem_size);

	}

	//��ȡҪ��Ԥ���ʱ���

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


	// ��ȡ�Ż���ԴĿ��:cpu or mem ,cpuΪ0,memoryΪ1
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

// ��ѵ���ļ����ݽ�����Ч�ķָ���� ʱ������£���13��19]
int do_train_file_valid_div(char ** const trainfileBuff, int *valid_div_data)
{
	int sample_num_count = 0;
	valid_div_data[0] = traincontrol.endTime;
	int temptime = traincontrol.endTime;
	int temptime_count = 0;
	for (int i = (traincontrol.trainfileLinenum - 2); i >= 0; i--)  //���ڴ�����i�У�����
	{
		// Ѱ��ʱ���
		for (int j = 0, flag_shijianduan = 0; flag_shijianduan == 0; j++)
		{
			// ���Ҽ�ʱ�䲢ȷ��ʱ��  "  - :  - "
			if ((trainfileBuff[i][j] == 45) && (trainfileBuff[i][j + 3] == 45) && (trainfileBuff[i][j + 9] == 58))
			{
				flag_shijianduan = 1;  // �Ҽ���ʱ��α�־λ
				if ((((trainfileBuff[i][j - 2] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j - 1] - CONV_ASCII_TO_INT)) * 365 +
					month[(trainfileBuff[i][j + 1] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j + 2] - CONV_ASCII_TO_INT)].month_day_total_num +
					(trainfileBuff[i][j + 4] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j + 5] - CONV_ASCII_TO_INT)) != temptime)
				{   // �뵱ǰ��ʱ�䲻һ����֤����һ�����µ�ʱ�䣡�������и�ֵ��������һ�Ρ�
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

	// ������
	//  = inputcontrol.startTime - CYC_SAMPLE;

	// ��ǰʱ��
	int temptime = 0;


	// �ָ�ʱ���
	for (int count_div = 0; count_div < traincontrol.sample_num_noise;)
	{
		//int a = 0;
		for (int i = (traincontrol.trainfileLinenum - 1); i >= 0; i--)
		{
			if (count_div >= traincontrol.sample_num_noise) break;
			//printf("\n��%d���ڴ���...", i);
			// Ѱ��ʱ���
			for (int j = 0, flag_shijianduan = 0; flag_shijianduan == 0; j++)
			{
				if (count_div >= traincontrol.sample_num_noise) break;
				// ���Ҽ�ʱ�䲢ȷ��ʱ��  "  - :  - "
				if ((trainfileBuff[i][j] == 45) && (trainfileBuff[i][j + 3] == 45) && (trainfileBuff[i][j + 9] == 58))
				{
					flag_shijianduan = 1;  // �Ҽ���ʱ��α�־λ
					if (count_div >= traincontrol.sample_num_noise) break;

					temptime = (((trainfileBuff[i][j - 2] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j - 1] - CONV_ASCII_TO_INT)) * 365 +
						month[(trainfileBuff[i][j + 1] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j + 2] - CONV_ASCII_TO_INT)].month_day_total_num +
						(trainfileBuff[i][j + 4] - CONV_ASCII_TO_INT) * 10 + (trainfileBuff[i][j + 5] - CONV_ASCII_TO_INT));
					// ��߽� �� �ұ߽� Լ��
					// [13,20) : 13 14 15 16 17 18 19
					if (temptime > valid_div_data[count_div + 1] && temptime <= valid_div_data[count_div])
					{
						// Լ���ɹ� ��ʼͳ��

						// Լ��flavor���
						for (int j_fla = 0, flag_fla = 0; flag_fla == 0; j_fla++)
						{
							if (count_div >= traincontrol.sample_num_noise) break;
							// Ѱ�ҵ� "fla"��Ϊ��ѯ����
							if ((trainfileBuff[i][j_fla] == 102) && (trainfileBuff[i][j_fla + 1] == 108) && (trainfileBuff[i][j_fla + 2] == 97))
							{
								flag_fla = 1;
								// ȷ����flavor������һλ���ֻ�����λ����
								if (trainfileBuff[i][j_fla + 7] > 47 && trainfileBuff[i][j_fla + 7] < 58)
								{
									//��λ��
									for (int kkk = 0; kkk < inputcontrol.flavorMaxnum; kkk++)
									{
										// ƥ�������������ͺ�!!!
										if (((trainfileBuff[i][j_fla + 6] - CONV_ASCII_TO_INT) * 10 + trainfileBuff[i][j_fla + 7] - CONV_ASCII_TO_INT) == inputFlavor[kkk].flavor_name)
											trainfileFlavordata[count_div * inputcontrol.flavorMaxnum + kkk]++;
									}


								}
								else
								{
									for (int kkk = 0; kkk < inputcontrol.flavorMaxnum; kkk++)
									{
										// ƥ�������������ͺ�!!!
										if ((trainfileBuff[i][j_fla + 6] - CONV_ASCII_TO_INT) == inputFlavor[kkk].flavor_name)
										{
											trainfileFlavordata[count_div * inputcontrol.flavorMaxnum + kkk]++;
											//printf("�����е�%d�� -- %d", (count_div * inputcontrol.flavorMaxnum + kkk), trainfileFlavordata[count_div * inputcontrol.flavorMaxnum + kkk]);
										}

									}

								}

								// ��ѯ��������ѭ��,��ֹ����Խ��!!!
								// break;
							}
							// ʣ�µ����ݾͲ���Ҫ��!!!

						}
					}
					else
					{  // Լ��ʧ�� ����һ������(����ʱ���)
						count_div++;
						// ʣ�µ����ݾͲ���Ҫ��!!!
						if (count_div >= traincontrol.sample_num_noise) break;
						// ���ν��ض�Լ���ɹ� �����ͳ��!! �����ִ��������ν׶�ִ����ֻ��ִ��1��
						if (temptime > valid_div_data[count_div + 1] && count_div < traincontrol.sample_num_noise && temptime <= valid_div_data[count_div])
						{
							// Լ���ɹ� ��ʼͳ��
							// Լ��flavor���
							for (int j_fla = 0, flag_fla = 0; flag_fla == 0; j_fla++)
							{
								// Ѱ�ҵ� "fla"��Ϊ��ѯ����
								if ((trainfileBuff[i][j_fla] == 102) && (trainfileBuff[i][j_fla + 1] == 108) && (trainfileBuff[i][j_fla + 2] == 97))
								{
									flag_fla = 1;

									// ȷ����flavor������һλ���ֻ�����λ����
									if (trainfileBuff[i][j_fla + 7] > 47 && trainfileBuff[i][j_fla + 7] < 58)
									{
										//��λ��
										for (int kkk = 0; kkk < inputcontrol.flavorMaxnum; kkk++)
										{
											// ƥ�������������ͺ�!!!
											if (((trainfileBuff[i][j_fla + 6] - CONV_ASCII_TO_INT) * 10 + trainfileBuff[i][j_fla + 7] - CONV_ASCII_TO_INT) == inputFlavor[kkk].flavor_name)
												trainfileFlavordata[count_div * inputcontrol.flavorMaxnum + kkk]++;
										}
									}
									else
									{
										for (int kkk = 0; kkk < inputcontrol.flavorMaxnum; kkk++)
										{
											// ƥ�������������ͺ�!!!
											if ((trainfileBuff[i][j_fla + 6] - CONV_ASCII_TO_INT) == inputFlavor[kkk].flavor_name)
											{
												trainfileFlavordata[count_div * inputcontrol.flavorMaxnum + kkk]++;
												//printf("�����е�%d�� -- %d", (count_div * inputcontrol.flavorMaxnum + kkk), trainfileFlavordata[count_div * inputcontrol.flavorMaxnum + kkk]);
											}
										}
									}
									// ��ѯ��������ѭ��,��ֹ����Խ��!!!
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

	// ˳����֯����˳�� ��Ϊ����
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

// ��ּ��㲿�֡�  tempΪҪ�����ĳ��ȣ���
void get_trainfileFlavordata_diff(int *input, int *ouput, int temp)
{
	//int temp = traincontrol.sample_num - 1;  //Ҫ�����ĳ���
	for (int i = 0; i < temp; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			ouput[j + i * inputcontrol.flavorMaxnum] = input[j + (i + 1) * inputcontrol.flavorMaxnum] - input[j + i * inputcontrol.flavorMaxnum];
		}
	}
}



// ��ֵ�˲���
void do_noise_avgfilter(int *trainfileFlavordata)
{
	int temp = traincontrol.sample_num_noise;  //Ҫ�����ĳ���

	float shouwei_3[3] = { 0.33, 0.33, 0.33 };
	float moban_3[3] = { 0.50, 0.0, 0.50 };  // ����Ϊ3
	float shouwei_5[5] = { 0.2, 0.2, 0.2, 0.2, 0.2 };
	float moban_5[5] = { 0.25, 0.25,0.0,0.25,0.25 };  // ����Ϊ5

													  // ��β����������Ϊ3
	for (int i = 0; i < temp; i++)
	{
		// ��
		trainfileFlavordata[i] = (int)(trainfileFlavordata[i + 0 * inputcontrol.flavorMaxnum] * shouwei_3[0] +
			trainfileFlavordata[i + 1 * inputcontrol.flavorMaxnum] * shouwei_3[1] + trainfileFlavordata[i + 2 * inputcontrol.flavorMaxnum] * shouwei_3[2] + 0.5);
		// β
		trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum] = (int)(trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum] * shouwei_3[0] +
			trainfileFlavordata[i + (temp - 2) * inputcontrol.flavorMaxnum] * shouwei_3[1] + trainfileFlavordata[i + (temp - 3) * inputcontrol.flavorMaxnum] * shouwei_3[2] + 0.5);
	}
	// �м䲿��
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
		for (int j = 1; j < (temp - 1); j++)
		{
			trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] = (int)(trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum] * moban_3[0] +
				trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum] * moban_3[2] + 0.5);
		}
	}
}

// �ڼ���ȥ�룡��
void do_noise_happyday(int *trainfileFlavordata, int *day_data, int tatal_daynum)
{
	for (int i = 0; i < tatal_daynum; i++)
	{
		for (int j = 0; j < HAPPYDAY_TOTALNUM - 1; j++)
		{
			if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[j] && i == 0)  //�ڼ����ǵ�һ��(����˫11)
			{
				for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++) //���θ�ֵ������
				{
					trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i + 3)* inputcontrol.flavorMaxnum + aaa];
					trainfileFlavordata[(i + 1)* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i + 3)* inputcontrol.flavorMaxnum + aaa];
					trainfileFlavordata[(i + 2)* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i + 3)* inputcontrol.flavorMaxnum + aaa];
				}
				i += 2;
				break;
			}
			if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[j] && i > 0 && i<tatal_daynum - 3)  //�ڼ��ղ��ǵ�һ��(����˫11)
			{
				for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++) //���θ�ֵ������
				{
					trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = (trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa] + trainfileFlavordata[(i + 3)* inputcontrol.flavorMaxnum + aaa]) / 2;
					trainfileFlavordata[(i + 1)* inputcontrol.flavorMaxnum + aaa] = (trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa] + trainfileFlavordata[(i + 3)* inputcontrol.flavorMaxnum + aaa]) / 2;
					trainfileFlavordata[(i + 2)* inputcontrol.flavorMaxnum + aaa] = (trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa] + trainfileFlavordata[(i + 3)* inputcontrol.flavorMaxnum + aaa]) / 2;
				}
				i += 2;
				break;
			}

			if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[j] && i > 0 && i == tatal_daynum - 3)  //�ڼ����ǵ���3��(����˫11)
			{
				for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++) //���θ�ֵ������
				{
					trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa];
					trainfileFlavordata[(i + 1)* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa];
					trainfileFlavordata[(i + 2)* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa];
				}
				i += 2;
				break;
			}

			if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[j] && i > 0 && i == tatal_daynum - 2)  //�ڼ��������2��(����˫11)
			{
				for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++) //���θ�ֵ������
				{
					trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa];
					trainfileFlavordata[(i + 1)* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa];
				}
				i += 2;
				break;
			}

			if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[j] && i > 0 && i == tatal_daynum - 1)  //�ڼ��������һ��(����˫11)
			{
				for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++) //���θ�ֵ������
				{
					trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa];
				}
				i += 2;
				break;
			}

		}
		if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[9] && i == 0)  //11.11�ǵ�һ�죬������
		{
			for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++)
			{
				trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i + 1)* inputcontrol.flavorMaxnum + aaa];
			}
		}
		if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[9] && i > 0 && i != (tatal_daynum - 1))  //11.11 ���ǵ�һ�죡��Ҳ�������һ��
		{
			for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++)
			{
				trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = (trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa] + trainfileFlavordata[(i + 1)* inputcontrol.flavorMaxnum + aaa]) / 2;
			}
		}
		if ((day_data[tatal_daynum - 1 - i] % 365) == happyday[9] && i > 0 && i == (tatal_daynum - 1))  //11.11 �����һ��
		{
			for (int aaa = 0; aaa < inputcontrol.flavorMaxnum; aaa++)
			{
				trainfileFlavordata[i* inputcontrol.flavorMaxnum + aaa] = trainfileFlavordata[(i - 1)* inputcontrol.flavorMaxnum + aaa];
			}
		}

	}
}



// ��ֵ�˲�
void do_noise_midfilter(int *trainfileFlavordata, Flavor *inputFlavor)
{
	int temp = traincontrol.sample_num_noise;  //Ҫ�����ĳ���

											   // ȥ��ͷ��β
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
		// ��
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
			printf("���������еĸ߷�  �ײ�λ��[ %d ]   ȥ��֮ǰ : [ %d %d %d ]\n", i, trainfileFlavordata[i], trainfileFlavordata[i + 1 * inputcontrol.flavorMaxnum], trainfileFlavordata[i + 2 * inputcontrol.flavorMaxnum]);
			trainfileFlavordata[i] = (int)(trainfileFlavordata[i + 1 * inputcontrol.flavorMaxnum] + trainfileFlavordata[i + 2 * inputcontrol.flavorMaxnum] + 0.5);
			//trainfileFlavordata[i] = (int)(trainfileFlavordata[i] / 2 + 1);
			printf("���������еĸ߷�  �ײ�λ��[ %d ]   ȥ��֮�� : [ %d %d %d ]\n", i, trainfileFlavordata[i], trainfileFlavordata[i + 1 * inputcontrol.flavorMaxnum], trainfileFlavordata[i + 2 * inputcontrol.flavorMaxnum]);
		}



		// β
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
			printf("���������еĸ߷�  β��λ��[ %d ]   ȥ��֮ǰ : [ %d %d %d ]\n", i, trainfileFlavordata[i + (temp - 3) * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (temp - 2) * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum]);
			// trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum] = (int)(trainfileFlavordata[i + (temp - 2) * inputcontrol.flavorMaxnum] + trainfileFlavordata[i + (temp - 3) * inputcontrol.flavorMaxnum] + 0.5);
			trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum] = (int)(trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum] / 2 + 1);
			printf("���������еĸ߷�  β��λ��[ %d ]   ȥ��֮�� : [ %d %d %d ]\n", i, trainfileFlavordata[i + (temp - 3) * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (temp - 2) * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (temp - 1) * inputcontrol.flavorMaxnum]);

		}
	}


	// �ȴ����м�
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
		for (int j = 1; j < (temp - 1); j++)
		{
			// ����ȥ����� ����OK
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
				// ��ƽ�������������룡
				printf("���������еĸ߷�  �м�λ��[ %d %d ]   ȥ��֮ǰ : [ %d %d %d ]\n", i, j, trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum], trainfileFlavordata[i + j * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum]);
				//trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] = (int)(trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum] + trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum] + 0.5);
				trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] = (int)(trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] / 2 + 0.5);
				printf("���������еĸ߷�  �м�λ��[ %d %d ]   ȥ��֮�� : [ %d %d %d ]\n", i, j, trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum], trainfileFlavordata[i + j * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum]);
			}

		}
	}

	// �����м�� �͹�����
	/*for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
	for (int j = 1; j < (temp - 1); j++)
	{
	// ����ȥ����� ����OK
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
	// ��ƽ�������������룡
	printf("���������еĵ͹�  �м�λ��[ %d %d ]   ȥ��֮ǰ : [ %d %d %d ]\n", i, j, trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum], trainfileFlavordata[i + j * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum]);
	trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] = (int)( (trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum] + trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum])*0.2 + trainfileFlavordata[i + j * inputcontrol.flavorMaxnum]);
	// trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] = (int)(trainfileFlavordata[i + j * inputcontrol.flavorMaxnum] / 2 + 0.5);
	printf("���������еĵ͹�  �м�λ��[ %d %d ]   ȥ��֮�� : [ %d %d %d ]\n", i, j, trainfileFlavordata[i + (j - 1)*inputcontrol.flavorMaxnum], trainfileFlavordata[i + j * inputcontrol.flavorMaxnum], trainfileFlavordata[i + (j + 1)*inputcontrol.flavorMaxnum]);
	}

	}
	}*/



}


// ��ӡ����
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
����Ŀ��:��ȡ���һ�������
����:*traindata Ϊѵ������
num_vm  Ϊ�����������������
num_traindata Ϊѵ����������
preidct_sample_day_num  ��ҪԤ�������
*/
double *predict_get_ori_data(int *traindata, int num_vm, int num_traindata, int preidct_sample_day_num) {
	double *out_double = new double[num_vm];
	int line_num = num_traindata - 1; //���һ�е��к�
	for (int i = 0; i < num_vm; i++) {
		out_double[i] = (double)traindata[line_num*num_vm + i] * ((double)preidct_sample_day_num / CYC_SAMPLE);
	}
	return out_double;
}



/*
����Ŀ��:Ԥ����һ�׶ε�������
����:���߼��ع�
����:*traindata Ϊѵ������
num_vm  Ϊ�����������������
num_traindata Ϊѵ����������
preidct_sample_day_num  ��ҪԤ�������
*/
double *predict_run_liner(int *traindata, int num_vm, int num_traindata, int preidct_sample_day_num) {
	double *out_double = new double[num_vm];
	double *out_int = new double[num_vm];
	double *W = new double[num_traindata];
	double sum_W = 0;
	//����Ȩֵ����
	for (int i = 0; i < num_traindata; i++) {  //Ȩֵ��������
		W[i] = i * i;
		sum_W += W[i];
	}
	for (int i = 0; i < num_traindata; i++) {
		W[i] = W[i] / sum_W;
	}



	//Ԥ��
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
����Ŀ��:Ԥ����һ�׶ε�������
����: ѭ����С����
����:*traindata Ϊѵ������
num_vm  Ϊ�����������������
num_traindata Ϊѵ����������
���:out_intΪÿ���������������
preidct_sample_day_num  ��ҪԤ�������
*/
int *predict_run_RLMS(int *traindata, int num_vm, int num_traindata, int preidct_sample_day_num) {
	int i, j;
	// ������ѵ��һ��
	int N = 8;  // ���preidct_sample_day_num�������N
	double erfa = 0.1; //W��ѧϰ��

	if (N > num_traindata) {
		printf("Error! NN Training Dosen't Has Enough Training Data");
		printf("num_traindata Must Larger Than N !");
	}
	// �����ʼȨֵ����
	double *W_line = new double[N];
	double *W = new double[num_vm*N];   //ÿһ�඼��һ��Ȩֵ����
	double sum_W = 0;
	for (i = 0; i < N; i++) {  //Ȩֵ��������
		W_line[i] = i + 1;
		sum_W += W_line[i];
	}
	for (i = 0; i < N; i++) {
		W_line[i] = W_line[i] / sum_W;
	}
	for (i = 0; i < num_vm; i++) {
		for (j = 0; j < N; j++) {
			W[i*N + j] = W_line[j];  //ÿһ�д���һ�����Ȩֵ
		}
	}

	/*printf("\nW_line:\n"); //��ʾ������֮���Ȩֵ
	for (i = 0; i < N; i++) {
	printf("%f ", W_line[i]);
	}*/

	//��ʼѵ��
	double *x = new double[N]; //����ѵ������������x��Nά����
	double y_exp;   //����ѵ���������������y_exp
	double y;   //����ѵ��������ʵ�����y_exp
	double dw;   //wƫ��
	double da, dz;

	int train_time;//��ǰѵ������
	int train_class_time;  //��ǰѵ���ڼ���
	for (train_class_time = 0; train_class_time < num_vm; train_class_time++) { //ѵ����train_class_time�������
																				//��ά�ȵ�Ȩֵ
		double *w_single = new double[N];
		for (i = 0; i < N; i++) {  //��Ȩֵ��ֵ����ά��
			w_single[i] = W[train_class_time*N + i];
		}
		for (train_time = 0; train_time < (num_traindata - N); train_time++) {  //��train_time��ѵ��
																				/*-------------------------ǰ��------------------------------------*/
																				// ���쵱��ѵ��������Ԥ��
			y = 0;
			for (i = 0; i < N; i++) {  //��������x
				x[i] = (double)traindata[(train_time + i)*num_vm + train_class_time];
				y += x[i] * w_single[i]; //Ԥ�����
			}
			y_exp = (double)traindata[(train_time + N)*num_vm + train_class_time];

			//��ʾ
			printf("\n����\n");
			for (i = 0; i < N; i++) {
				printf("%f ", x[i]);
			}
			printf("\n���������%f ", y_exp);
			printf("\nʵ�������%f ", y);

			y_exp = 1 / (1 + exp(-y_exp));  //�����sigmoid
			y = 1 / (1 + exp(-y));
			da = -(y_exp / y + 0.0000001) + (1 - y_exp) / (1 - y + 0.0000001);
			dz = da * y*(1 - y);

			printf("\n����ǰW\n");
			for (i = 0; i < N; i++) {
				printf("%f ", w_single[i]);
			}

			//����Ȩֵ
			for (i = 0; i < N; i++) {
				w_single[i] = w_single[i] + erfa * dz * x[i];
				if (w_single[i] < 0) w_single[i] = 0;
			}

			printf("\n���º�W\n");
			for (i = 0; i < N; i++) {
				printf("%f ", w_single[i]);
			}
		}
		for (i = 0; i < N; i++) {  //������ά�ȸ�ֵ����Ȩֵ����
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


	//Ԥ��
	int predict_num_int = preidct_sample_day_num / CYC_SAMPLE;  //��ҪԤ�����������
	int predict_num_rem = preidct_sample_day_num % CYC_SAMPLE;  //��ҪԤ�����������
	double *out_double = new double[num_vm];
	for (i = 0; i < num_vm; i++) {
		out_double[i] = 0;
	}
	for (train_class_time = 0; train_class_time < num_vm; train_class_time++) { //ѵ����train_class_time�������
																				//��ά�ȵ�Ȩֵ
		double *w_single = new double[N];
		for (i = 0; i < N; i++) {  //��Ȩֵ��ֵ����ά��
			w_single[i] = W[train_class_time*N + i];
		}
		double *predict_input = new double[N + predict_num_int];
		for (i = 0; i < N; i++) {
			predict_input[i] = (double)traindata[((num_traindata - N) + i)*num_vm + train_class_time];
		}

		/*printf("\ninput:\n"); //��ʾ������֮���Ȩֵ
		for (i = 0; i < N; i++) {
		printf("%f ", predict_input[i]);
		}*/

		y = 0;
		for (int pre_count = 0; pre_count < predict_num_int; pre_count++) {
			for (i = pre_count; i < N + pre_count; i++) {
				y += predict_input[i] * w_single[i]; //Ԥ�����
			}

			/*printf("\n\npredict_input:\n");
			for (i = 0; i < N; i++) {
			printf("%f ", predict_input[i]);
			}

			printf("\nw_single:\n"); //��ʾ������֮���Ȩֵ
			for (i = 0; i < N; i++) {
			printf("%f ", w_single[i]);
			}*/

			predict_input[N + pre_count] = y;
			out_double[train_class_time] += y;
		}
		if (predict_num_rem != 0) { //��������
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
����Ŀ��:Ԥ����һ�׶ε�������
����: ѭ��W��ֵ
����:*traindata Ϊѵ������
num_vm  Ϊ�����������������
num_traindata Ϊѵ����������
���:out_intΪÿ���������������
preidct_sample_day_num  ��ҪԤ�������
*/
int *predict_run_Wmean(int *traindata, int num_vm, int num_traindata, int preidct_sample_day_num) {
	int i, j;
	// ������ѵ��һ��
	int N = num_traindata - 4;  // ���preidct_sample_day_num�������N

	if (N <= 0) {
		printf("dosne't has enough data!!");
		while (1); //û���� ,��������
	}
	// �����ʼȨֵ����
	double *W_line = new double[N];
	double *W = new double[num_vm*N];   //ÿһ�඼��һ��Ȩֵ����
	double sum_W = 0;
	/*-------------��ʼȨֵ-------------*/
	for (i = 0; i < N; i++) {  //Ȩֵ��������
		W_line[i] = i + 1;
		sum_W += W_line[i];
	}
	for (i = 0; i < N; i++) {
		W_line[i] = W_line[i] / sum_W;
	}
	for (i = 0; i < num_vm; i++) {
		for (j = 0; j < N; j++) {
			W[i*N + j] = W_line[j];  //ÿһ�д���һ�����Ȩֵ
		}
	}

	/*-------------��ʼȨֵ-------------*/
	double *x = new double[N]; //����ѵ������������x��Nά����
	double y_exp;   //����ѵ���������������y_exp

	int train_totol_time = num_traindata - N;//��ѵ������
	int train_time;//��ǰѵ������
	int train_class_time;  //��ǰѵ���ڼ���
	for (train_class_time = 0; train_class_time < num_vm; train_class_time++) { //ѵ����train_class_time�������
																				//��ά�ȵ�Ȩֵ
		double *w_single = new double[N];
		double *w_single_temp = new double[N];
		double sum_w_single_temp;
		for (i = 0; i < N; i++) {  //��Ȩֵ��ֵ����ά��
			w_single[i] = 0;
		}
		for (train_time = 0; train_time < train_totol_time; train_time++) {  //��train_time��ѵ��
																			 // ���쵱��ѵ��������Ԥ��
			y_exp = (double)traindata[(train_time + N)*num_vm + train_class_time];  //�������y
			sum_w_single_temp = 0; //Ȩֵ֮��
			for (i = 0; i < N; i++) {
				x[i] = (double)traindata[(train_time + i)*num_vm + train_class_time]; //��������x
				w_single_temp[i] = (y_exp - x[i])*(y_exp - x[i]); //��������������ƫ��
				sum_w_single_temp += w_single_temp[i];
			}
			for (i = 0; i < N; i++) {
				w_single_temp[i] = w_single_temp[i] / sum_w_single_temp;
				w_single[i] += w_single_temp[i];
			}


		}
		for (i = 0; i < N; i++) {  //������ά�ȸ�ֵ����Ȩֵ����
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


	//Ԥ��
	int predict_num_int = preidct_sample_day_num / CYC_SAMPLE;  //��ҪԤ�����������
	int predict_num_rem = preidct_sample_day_num % CYC_SAMPLE;  //��ҪԤ�����������
	double *out_double = new double[num_vm];
	double y;
	for (i = 0; i < num_vm; i++) {
		out_double[i] = 0;
	}
	for (train_class_time = 0; train_class_time < num_vm; train_class_time++) { //ѵ����train_class_time�������
																				//��ά�ȵ�Ȩֵ
		double *w_single = new double[N];
		for (i = 0; i < N; i++) {  //��Ȩֵ��ֵ����ά��
			w_single[i] = W[train_class_time*N + i];
		}
		double *predict_input = new double[N + predict_num_int];
		for (i = 0; i < N; i++) {
			predict_input[i] = (double)traindata[((num_traindata - N) + i)*num_vm + train_class_time];
		}

		/*printf("\ninput:\n"); //��ʾ������֮���Ȩֵ
		for (i = 0; i < N; i++) {
		printf("%f ", predict_input[i]);
		}*/

		y = 0;
		for (int pre_count = 0; pre_count < predict_num_int; pre_count++) {
			for (i = pre_count; i < N + pre_count; i++) {
				y += predict_input[i] * w_single[i]; //Ԥ�����
			}

			/*printf("\n\npredict_input:\n");
			for (i = 0; i < N; i++) {
			printf("%f ", predict_input[i]);
			}

			printf("\nw_single:\n"); //��ʾ������֮���Ȩֵ
			for (i = 0; i < N; i++) {
			printf("%f ", w_single[i]);
			}*/

			predict_input[N + pre_count] = y;
			out_double[train_class_time] += y;
		}
		if (predict_num_rem != 0) { //��������
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
����Ŀ��:Ԥ����һ�׶ε�������
����: RLS����Ӧ�˲�
����:*traindata Ϊѵ������
num_vm  Ϊ�����������������
num_traindata Ϊѵ����������
���:out_intΪÿ���������������
*/
int *predict_run_RLS(int *traindata, int num_vm, int num_traindata) {
	/*���в���*/
	int i, j;
	int *out_int = new int[num_vm];
	int M = num_vm; //�˲�������:�������������
	int Signal_Len = num_traindata; //ѵ��������
									/*�㷨����*/
	double lamda = 1; //��������
	double c = 1;  //С���� ��֤����P������
				   /*�㷨����*/
	double *out_double = new double[M*M]; //���
	double *I = new double[M*M]; //M*M�ĵ�λ��
	for (i = 0; i < M; i++) {  //I��λ�󸳳�ֵ
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
	double *K_x = new double[M*M]; //K*x' ά��M*M
	double *x = new double[M]; //ÿ��ѵ����������������
	double *d = new double[M]; //ÿ��ѵ����������������
	double *y = new double[M]; //ÿ��ѵ���������������
	double *Eta = new double[M]; //ÿ��ѵ�����������
	double *x_plast = new double[M]; //x'*P_last:ά��1*M
	double *x_plast_x = new double[1]; //x'*P_last*x:ά��1
	double *K = new double[M]; //P_last*x,ά��M*1
							   /*�㷨����*/
	for (int train_times = 0; train_times < Signal_Len - 1; train_times++) {
		if (train_times == 0) { //��һ��ѵ����ʼǰ��ʼ������
			for (i = 0; i < M; i++) {
				for (j = 0; j < M; j++) {
					P_last[i*M + j] = I[i*M + j] / c; //P��ʼ��
					W_last[i*M + j] = 0;  //W��ʼ��Ϊ0����
				}
			}
		}
		//��������
		for (i = 0; i < M; i++) {
			x[i] = (double)traindata[train_times*M + i];    //�����µ��ź�ʸ��
			d[i] = (double)traindata[(train_times + 1)*M + i];  //�����µ������ź�
		}


		//��ʼ����
		mat_mult(x_plast, x, 1, M, P_last, M, M);
		mat_mult(x_plast_x, x_plast, 1, M, x, M, 1);
		mat_mult(K, P_last, M, M, x, M, 1);
		for (i = 0; i < M; i++) {
			K[i] /= (x_plast_x[0] + lamda);
		}
		mat_mult(y, x, 1, M, W_last, M, M);

		for (i = 0; i < M; i++) {  //���
			Eta[i] = d[i] - y[i];
		}

		mat_mult(W, K, M, 1, Eta, 1, M);
		for (i = 0; i < M; i++) { //����W
			for (j = 0; j < M; j++) {
				W_last[i*M + j] = W_last[i*M + j] + W[i*M + j];
			}
		}
		mat_mult(K_x, K, M, 1, x, 1, M);
		for (i = 0; i < M; i++) { //����P���м����
			for (j = 0; j < M; j++) {
				K_x[i*M + j] = I[i*M + j] - K_x[i*M + j];
			}
		}
		mat_mult(P, K_x, M, M, P_last, M, M);
		for (i = 0; i < M; i++) { //����P
			for (j = 0; j < M; j++) {
				P_last[i*M + j] = P[i*M + j] / lamda;
			}
		}
	}
	for (i = 0; i < M; i++) {
		x[i] = (double)traindata[(Signal_Len - 1)*M + i];    //�����µ��ź�ʸ��
	}
	mat_mult(out_double, x, 1, M, W_last, M, M);
	for (i = 0; i < M; i++) {
		out_int[i] = round_my(out_double[i]);
	}
	return out_int;
}

/*
����˷�:mat1*mat2
�������:mat1-����1  mat1_height-����1�߶�  mat1_width-����1����
mat2-����2  mat1_height-����2�߶�  mat2_width-����1����
*/
void mat_mult(double *mult_out, double *mat1, int mat1_height, int mat1_width, double *mat2, int mat2_height, int mat2_width) {
	if (mat1_width != mat2_height)  printf("����˷������ʽ����");
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
����Ŀ��:���������
����:	̰��FFD
����:	*require_vm		���������
num_vm			�������������
*inputFlavor	��������ܱ�
resource_pm		���������ܽṹ��
opt_target		�Ż�Ŀ�� 0:CPU 1:MEM
*result_save	���������ݱ�(���)
���:	�����ķ���������
*/
int putVM_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	// ����һ����õĳ�ʼ���� ��������ܱ�������
	/*
	Flavor *inputFlavor_data = new Flavor[inputcontrol.flavorMaxnum];
	for (int i = 0; i<inputcontrol.flavorMaxnum; i++)
	{
	inputFlavor_data[i].cpu_core_num = inputFlavor[i].cpu_core_num;
	inputFlavor_data[i].mem_size = inputFlavor[i].mem_size;
	inputFlavor_data[i].flavor_name = inputFlavor[i].flavor_name;
	}
	*/

	/***********************************************************���򲿷�*****************************************************************/
	if (opt_target == 0)
	{
		// ʵ������Ӵ�С
		for (int i = 0; i < num_vm - 1; i++)
		{
			for (int j = i + 1; j < num_vm; j++)
			{
				if (inputFlavor[j].cpu_core_num>inputFlavor[i].cpu_core_num) // ����cpu����ǰ��,����
				{
					Flavor tmp_flavor;
					// ���ܱ�λ�ý���
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// ����λ�ý���
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
				else if (inputFlavor[j].cpu_core_num == inputFlavor[i].cpu_core_num && inputFlavor[j].mem_size>inputFlavor[i].mem_size) // ����cpu���,mem����ǰ��,����
				{
					Flavor tmp_flavor;
					// ���ܱ�λ�ý���
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// ����λ�ý���
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
			}
		}
	}
	else if (opt_target == 1)
	{
		// ʵ������Ӵ�С
		for (int i = 0; i < num_vm - 1; i++)
		{
			for (int j = i + 1; j < num_vm; j++)
			{
				if (inputFlavor[j].mem_size>inputFlavor[i].mem_size) // ����mem����ǰ��,����
				{
					Flavor tmp_flavor;
					// ���ܱ�λ�ý���
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// ����λ�ý���
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
				else if (inputFlavor[j].mem_size == inputFlavor[i].mem_size && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // mem��� ����CPU����ǰ��,����
				{
					Flavor tmp_flavor;
					// ���ܱ�λ�ý���
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// ����λ�ý���
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
			}
		}
	}
	else    // Ŀ��ѡ����� �˳�,�쳣
	{
		return -1;
	}

	// ����copy
	int *require_vm_copy = new int[num_vm];
	for (int i = 0; i < num_vm; i++)
		require_vm_copy[i] = require_vm[i];



	/********************************************************************���ò���***********************************************************************/
	// ����������������¼��
	Server *server_remain = new Server[2000];
	// ���ݸ�ֵ(�������2000Ҫ�ͺ��������max_serve_py��Ӧ,�������Կ��Ƕ��һ��max_serve_py�ӿ�)
	for (int i = 0; i < 2000; i++)
	{
		server_remain[i].cpu_core_num = resource_pm.cpu_core_num;
		server_remain[i].mem_size = resource_pm.mem_size;
	}

	int server_num = 1;
	for (int i = 0; i < num_vm; i++)	// һ��һ�ַ���
	{
		// ��ǰ������������ʼ��������
		int j = 0;
		while (require_vm_copy[i] != 0)		// ÿһ��һ��һ������,ֱ������
		{
			// ��ǰ�˸���������ñ�־λ
			int flag_put = 0;
			// ��������
			for (; j < server_num; j++)
			{
				// ���ܷ���
				if (server_remain[j].cpu_core_num < inputFlavor[i].cpu_core_num || server_remain[j].mem_size < inputFlavor[i].mem_size)		// ������Դ����
				{
					continue;	// ֱ�ӽ�����һ�ε�����
				}
				else if (server_remain[j].cpu_core_num >= inputFlavor[i].cpu_core_num && server_remain[j].mem_size >= inputFlavor[i].mem_size)	// ��֤����ʱ��Դ����,�����
				{
					server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
					server_remain[j].mem_size -= inputFlavor[i].mem_size;
					flag_put = 1;
					result_save[j*num_vm + i]++;	// ���ü�¼
					break;	// ����������,����ѭ��
				}
			}
			// �������forѭ���п����Ǳ����궼�Ų���,Ҳ�п����Ƿ��ú���ֱ������
			if (flag_put == 0)
			{
				server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
				server_remain[j].mem_size -= inputFlavor[i].mem_size;
				result_save[j*num_vm + i]++;	// ���ü�¼
				server_num++;	// �¿�һ��������

			}

			require_vm_copy[i]--; // ���������������һ
		}
	}

	// ���л�ԭ������
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

	/***********************************************************���򲿷�*****************************************************************/
	if (opt_target == 0)
	{
		// ʵ������Ӵ�С
		for (int i = 0; i < num_vm - 1; i++)
		{
			for (int j = i + 1; j < num_vm; j++)
			{
				if (inputFlavor[j].cpu_core_num>inputFlavor[i].cpu_core_num) // ����cpu����ǰ��,����
				{
					Flavor tmp_flavor;
					// ���ܱ�λ�ý���
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// ����λ�ý���
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
				else if (inputFlavor[j].cpu_core_num == inputFlavor[i].cpu_core_num && inputFlavor[j].mem_size>inputFlavor[i].mem_size) // ����cpu���,mem����ǰ��,����
				{
					Flavor tmp_flavor;
					// ���ܱ�λ�ý���
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// ����λ�ý���
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
			}
		}
	}
	else if (opt_target == 1)
	{
		// ʵ������Ӵ�С
		for (int i = 0; i < num_vm - 1; i++)
		{
			for (int j = i + 1; j < num_vm; j++)
			{
				if (inputFlavor[j].mem_size>inputFlavor[i].mem_size) // ����mem����ǰ��,����
				{
					Flavor tmp_flavor;
					// ���ܱ�λ�ý���
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// ����λ�ý���
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
				else if (inputFlavor[j].mem_size == inputFlavor[i].mem_size && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // mem��� ����CPU����ǰ��,����
				{
					Flavor tmp_flavor;
					// ���ܱ�λ�ý���
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// ����λ�ý���
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
			}
		}
	}
	else    // Ŀ��ѡ����� �˳�,�쳣
	{
		return -1;
	}


	// ����copy
	int *require_vm_copy = new int[num_vm];
	for (int i = 0; i < num_vm; i++)
		require_vm_copy[i] = require_vm[i];



	/********************************************************************���ò���***********************************************************************/
	// ����������������¼��
	Server *server_remain = new Server[2000];
	// ���ݸ�ֵ(�������2000Ҫ�ͺ��������max_serve_py��Ӧ,�������Կ��Ƕ��һ��max_serve_py�ӿ�)
	for (int i = 0; i < 2000; i++)
	{
		server_remain[i].cpu_core_num = resource_pm.cpu_core_num;
		server_remain[i].mem_size = resource_pm.mem_size;
	}

	int server_num = 1;
	for (int i = 0; i < num_vm; i++)	// һ��һ�ַ���
	{
		// ��ǰ������������ʼ��������
		int j = 0;
		while (require_vm_copy[i] != 0)		// ÿһ��һ��һ������,ֱ������
		{
			// ��ǰ�˸���������ñ�־λ
			int flag_put = 0;
			// ��������
			for (; j < server_num; j++)
			{
				// ���ܷ���
				if (server_remain[j].cpu_core_num < inputFlavor[i].cpu_core_num || server_remain[j].mem_size < inputFlavor[i].mem_size)		// ������Դ����
				{
					continue;	// ֱ�ӽ�����һ�ε�����
				}
				else if (server_remain[j].cpu_core_num >= inputFlavor[i].cpu_core_num && server_remain[j].mem_size >= inputFlavor[i].mem_size)	// ��֤����ʱ��Դ����,�����
				{
					server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
					server_remain[j].mem_size -= inputFlavor[i].mem_size;
					flag_put = 1;
					result_save[j*num_vm + i]++;	// ���ü�¼
					break;	// ����������,����ѭ��
				}
			}
			// �������forѭ���п����Ǳ����궼�Ų���,Ҳ�п����Ƿ��ú���ֱ������
			if (flag_put == 0)
			{
				server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
				server_remain[j].mem_size -= inputFlavor[i].mem_size;
				result_save[j*num_vm + i]++;	// ���ü�¼
				server_num++;	// �¿�һ��������

			}

			require_vm_copy[i]--; // ���������������һ
		}
	}

	// ���л�ԭ������
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

	/***********************************************************���򲿷�*****************************************************************/
	/*if (opt_target == 0)
	{
	// ʵ������Ӵ�С
	for (int i = 0; i < num_vm - 1; i++)
	{
	for (int j = i + 1; j < num_vm; j++)
	{
	if (inputFlavor[j].cpu_core_num>inputFlavor[i].cpu_core_num) // ����cpu����ǰ��,����
	{
	Flavor tmp_flavor;
	// ���ܱ�λ�ý���
	tmp_flavor = inputFlavor[j];
	inputFlavor[j] = inputFlavor[i];
	inputFlavor[i] = tmp_flavor;

	int tmp_require;
	// ����λ�ý���
	tmp_require = require_vm[j];
	require_vm[j] = require_vm[i];
	require_vm[i] = tmp_require;
	}
	else if (inputFlavor[j].cpu_core_num == inputFlavor[i].cpu_core_num && inputFlavor[j].mem_size>inputFlavor[i].mem_size) // ����cpu���,mem����ǰ��,����
	{
	Flavor tmp_flavor;
	// ���ܱ�λ�ý���
	tmp_flavor = inputFlavor[j];
	inputFlavor[j] = inputFlavor[i];
	inputFlavor[i] = tmp_flavor;

	int tmp_require;
	// ����λ�ý���
	tmp_require = require_vm[j];
	require_vm[j] = require_vm[i];
	require_vm[i] = tmp_require;
	}
	}
	}
	}
	else if (opt_target == 1)
	{
	// ʵ������Ӵ�С
	for (int i = 0; i < num_vm - 1; i++)
	{
	for (int j = i + 1; j < num_vm; j++)
	{
	if (inputFlavor[j].mem_size>inputFlavor[i].mem_size) // ����mem����ǰ��,����
	{
	Flavor tmp_flavor;
	// ���ܱ�λ�ý���
	tmp_flavor = inputFlavor[j];
	inputFlavor[j] = inputFlavor[i];
	inputFlavor[i] = tmp_flavor;

	int tmp_require;
	// ����λ�ý���
	tmp_require = require_vm[j];
	require_vm[j] = require_vm[i];
	require_vm[i] = tmp_require;
	}
	else if (inputFlavor[j].mem_size == inputFlavor[i].mem_size && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // mem��� ����CPU����ǰ��,����
	{
	Flavor tmp_flavor;
	// ���ܱ�λ�ý���
	tmp_flavor = inputFlavor[j];
	inputFlavor[j] = inputFlavor[i];
	inputFlavor[i] = tmp_flavor;

	int tmp_require;
	// ����λ�ý���
	tmp_require = require_vm[j];
	require_vm[j] = require_vm[i];
	require_vm[i] = tmp_require;
	}
	}
	}
	}
	else    // Ŀ��ѡ����� �˳�,�쳣
	{
	return -1;
	}*/

	int *require_vm_temp = new int[num_vm];
	for (int i = 0; i < num_vm; i++)	require_vm_temp[i] = require_vm[vm_put_priority[i]];
	for (int i = 0; i < num_vm; i++)   require_vm[i] = require_vm_temp[i];
	// ����copy
	int *require_vm_copy = new int[num_vm];
	for (int i = 0; i < num_vm; i++)	require_vm_copy[i] = require_vm[i];

	/********************************************************************���ò���***********************************************************************/
	// ����������������¼��
	Server *server_remain = new Server[2000];
	// ���ݸ�ֵ(�������2000Ҫ�ͺ��������max_serve_py��Ӧ,�������Կ��Ƕ��һ��max_serve_py�ӿ�)
	for (int i = 0; i < 2000; i++)
	{
		server_remain[i].cpu_core_num = resource_pm.cpu_core_num;
		server_remain[i].mem_size = resource_pm.mem_size;
	}

	int server_num = 1;
	for (int i = 0; i < num_vm; i++)	// һ��һ�ַ���
	{
		// ��ǰ������������ʼ��������
		int j = 0;
		while (require_vm_copy[i] != 0)		// ÿһ��һ��һ������,ֱ������
		{
			// ��ǰ�˸���������ñ�־λ
			int flag_put = 0;
			// ��������
			for (; j < server_num; j++)
			{
				// ���ܷ���
				if (server_remain[j].cpu_core_num < inputFlavor[i].cpu_core_num || server_remain[j].mem_size < inputFlavor[i].mem_size)		// ������Դ����
				{
					continue;	// ֱ�ӽ�����һ�ε�����
				}
				else if (server_remain[j].cpu_core_num >= inputFlavor[i].cpu_core_num && server_remain[j].mem_size >= inputFlavor[i].mem_size)	// ��֤����ʱ��Դ����,�����
				{
					server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
					server_remain[j].mem_size -= inputFlavor[i].mem_size;
					flag_put = 1;
					result_save[j*num_vm + i]++;	// ���ü�¼
					break;	// ����������,����ѭ��
				}
			}
			// �������forѭ���п����Ǳ����궼�Ų���,Ҳ�п����Ƿ��ú���ֱ������
			if (flag_put == 0)
			{
				server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
				server_remain[j].mem_size -= inputFlavor[i].mem_size;
				result_save[j*num_vm + i]++;	// ���ü�¼
				server_num++;	// �¿�һ��������

			}

			require_vm_copy[i]--; // ���������������һ
		}
	}

	// ���л�ԭ������
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


/*  ����  */

// vm_put_priority ���õ����ȼ�������

int putVM_greedy_ratio_guided(int *require_vm, int server_num_pre, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int *vm_put_priority)

{
	int *require_vm_temp = new int[inputcontrol.flavorMaxnum];
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)	require_vm_temp[i] = require_vm[vm_put_priority[i]];
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)   require_vm[i] = require_vm_temp[i];
	// ����copy
	int *require_vm_copy = new int[inputcontrol.flavorMaxnum];
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)	require_vm_copy[i] = require_vm[i];

	/********************************************************************���ò���***********************************************************************/
	// ����������������¼��
	Server *server_remain = new Server[MAX_SERVER_NUM];
	// ���ݸ�ֵ(�������2000Ҫ�ͺ��������max_serve_py��Ӧ,�������Կ��Ƕ��һ��max_serve_py�ӿ�)
	for (int i = 0; i < MAX_SERVER_NUM; i++)
	{
		server_remain[i].cpu_core_num = resource_pm.cpu_core_num;
		server_remain[i].mem_size = resource_pm.mem_size;
	}
	int continue_count = 0;
	int flag_count = 0; // һ��С��server_num_pre
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  // һ��һ�ַ���
	{
		for (int j = 0; j < require_vm_copy[i]; j++)  // ÿһ���������Ӧ����������
		{
			int flag_put = 0; // �������϶�Ӧ����̨������Ƿ��п����λ��
			continue_count = 0;
			for (int m = 0; m < server_num_pre; m++)  // �ֱ���˳�������ӡ����ѭ���Ĵ���������
			{
				if (server_remain[(m + flag_count) % server_num_pre].cpu_core_num < inputFlavor[i].cpu_core_num || server_remain[(m + flag_count) % server_num_pre].mem_size < inputFlavor[i].mem_size)		// ������Դ����
				{
					continue_count++;
					continue;	// ֱ�ӽ�����һ�ε�����

				}
				else
					if (server_remain[(m + flag_count) % server_num_pre].cpu_core_num >= inputFlavor[i].cpu_core_num && server_remain[(m + flag_count) % server_num_pre].mem_size >= inputFlavor[i].mem_size)	// ��֤����ʱ��Դ����,�����
					{
						server_remain[(m + flag_count) % server_num_pre].cpu_core_num -= inputFlavor[i].cpu_core_num;
						server_remain[(m + flag_count) % server_num_pre].mem_size -= inputFlavor[i].mem_size;
						flag_put = 1; //�����
						flag_count = (flag_count + continue_count) % server_num_pre;
						continue_count = 0;
						result_save[(flag_count++)*inputcontrol.flavorMaxnum + i]++;	// ���ü�¼
						if (flag_count == server_num_pre)  flag_count = 0;
						//print_data(result_save, server_num_pre);
						//printf("  %d  %d  %d\n", i,j, flag_count);
						break;
					}

			}
			// ���ѭ����server_num_pre��Σ���û�з��ý�ȥ���Ǿ��ٿ�һ����������
			if (flag_put == 0)
			{
				// ���˵�j������
				server_remain[server_num_pre].cpu_core_num -= inputFlavor[i].cpu_core_num;
				server_remain[server_num_pre].mem_size -= inputFlavor[i].mem_size;
				result_save[server_num_pre*inputcontrol.flavorMaxnum + i]++;	// ���ü�¼
				server_num_pre++;	// �¿�һ��������

			}
		}

	}

	return server_num_pre;
}



/*
����Ŀ��:���������
����:	̰��FFD
����:	*require_vm		���������
num_vm			�������������
*inputFlavor	��������ܱ�
resource_pm		���������ܽṹ��
opt_target		�Ż�Ŀ�� 0:CPU 1:MEM
*result_save	���������ݱ�(���)
���:	�����ķ���������
*/
int putVM_greedy_without_seq(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	// ����copy
	int *require_vm_copy = new int[num_vm];
	for (int i = 0; i < num_vm; i++)
		require_vm_copy[i] = require_vm[i];
	/********************************************************************���ò���***********************************************************************/
	// ����������������¼��
	Server *server_remain = new Server[2000];
	// ���ݸ�ֵ(�������2000Ҫ�ͺ��������max_serve_py��Ӧ,�������Կ��Ƕ��һ��max_serve_py�ӿ�)
	for (int i = 0; i < 2000; i++)
	{
		server_remain[i].cpu_core_num = resource_pm.cpu_core_num;
		server_remain[i].mem_size = resource_pm.mem_size;
	}

	int server_num = 1;
	for (int i = 0; i < num_vm; i++)	// һ��һ�ַ���
	{
		// ��ǰ������������ʼ��������
		int j = 0;
		while (require_vm_copy[i] != 0)		// ÿһ��һ��һ������,ֱ������
		{
			// ��ǰ�˸���������ñ�־λ
			int flag_put = 0;
			// ��������
			for (; j < server_num; j++)
			{
				// ���ܷ���
				if (server_remain[j].cpu_core_num < inputFlavor[i].cpu_core_num || server_remain[j].mem_size < inputFlavor[i].mem_size)		// ������Դ����
				{
					continue;	// ֱ�ӽ�����һ�ε�����
				}
				else if (server_remain[j].cpu_core_num >= inputFlavor[i].cpu_core_num && server_remain[j].mem_size >= inputFlavor[i].mem_size)	// ��֤����ʱ��Դ����,�����
				{
					server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
					server_remain[j].mem_size -= inputFlavor[i].mem_size;
					flag_put = 1;
					result_save[j*num_vm + i]++;	// ���ü�¼
					break;	// ����������,����ѭ��
				}
			}
			// �������forѭ���п����Ǳ����궼�Ų���,Ҳ�п����Ƿ��ú���ֱ������
			if (flag_put == 0)
			{
				server_remain[j].cpu_core_num -= inputFlavor[i].cpu_core_num;
				server_remain[j].mem_size -= inputFlavor[i].mem_size;
				result_save[j*num_vm + i]++;	// ���ü�¼
				server_num++;	// �¿�һ��������

			}

			require_vm_copy[i]--; // ���������������һ
		}
	}

	// ���л�ԭ������
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
����Ŀ��:����
����:	*require_vm		���������
num_vm			�������������
*inputFlavor	��������ܱ�
resource_pm		���������ܽṹ��
opt_target		�Ż�Ŀ�� 0:CPU 1:MEM
*result_save	���������ݱ�(���)
���:	�����ķ���������
*/
void putVM_seq(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	/***********************************************************���򲿷�*****************************************************************/
	if (opt_target == 0)
	{
		// ʵ������Ӵ�С
		for (int i = 0; i < num_vm - 1; i++)
		{
			for (int j = i + 1; j < num_vm; j++)
			{
				if (inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // ����cpu����ǰ��,����
				{
					Flavor tmp_flavor;
					// ���ܱ�λ�ý���
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// ����λ�ý���
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
				else if (inputFlavor[j].cpu_core_num == inputFlavor[i].cpu_core_num && inputFlavor[j].mem_size > inputFlavor[i].mem_size) // ����cpu���,mem����ǰ��,����
				{
					Flavor tmp_flavor;
					// ���ܱ�λ�ý���
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// ����λ�ý���
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
			}
		}
	}
	else if (opt_target == 1)
	{
		// ʵ������Ӵ�С
		for (int i = 0; i < num_vm - 1; i++)
		{
			for (int j = i + 1; j < num_vm; j++)
			{
				if (inputFlavor[j].mem_size > inputFlavor[i].mem_size) // ����mem����ǰ��,����
				{
					Flavor tmp_flavor;
					// ���ܱ�λ�ý���
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// ����λ�ý���
					tmp_require = require_vm[j];
					require_vm[j] = require_vm[i];
					require_vm[i] = tmp_require;
				}
				else if (inputFlavor[j].mem_size == inputFlavor[i].mem_size && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // mem��� ����CPU����ǰ��,����
				{
					Flavor tmp_flavor;
					// ���ܱ�λ�ý���
					tmp_flavor = inputFlavor[j];
					inputFlavor[j] = inputFlavor[i];
					inputFlavor[i] = tmp_flavor;

					int tmp_require;
					// ����λ�ý���
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
	/***********************************************************���򲿷�*****************************************************************/

	// ���÷���һ������  ��vm_size_data��Ĵ�С��
	if (Flag_put_priority == 1)
	{
		if (opt_target == 0)
		{
			// ʵ������Ӵ�С
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (vm_size_data[j] > vm_size_data[i]) // ����cpu����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
					else if (vm_size_data[j] == vm_size_data[i] && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // ����cpu���,mem����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
			// ʵ������Ӵ�С
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (vm_size_data[j] > vm_size_data[i]) // ����mem����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
					else if (vm_size_data[j] == vm_size_data[i] && inputFlavor[j].mem_size > inputFlavor[i].mem_size) // mem��� ����CPU����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
	// ���÷�����������  ��������������ڴ�Ĵ�С��
	if (Flag_put_priority == 2)
	{
		if (opt_target == 0)
		{
			// ʵ������Ӵ�С
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // ����cpu����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;
					}
					else if (inputFlavor[j].cpu_core_num == inputFlavor[i].cpu_core_num && inputFlavor[j].mem_size > inputFlavor[i].mem_size) // ����cpu���,mem����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
			// ʵ������Ӵ�С
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (inputFlavor[j].mem_size > inputFlavor[i].mem_size) // ����mem����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;
					}
					else if (inputFlavor[j].mem_size == inputFlavor[i].mem_size && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // mem��� ����CPU����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
	// ���÷�����������  ��ratio_vm�Ĵ�С��
	if (Flag_put_priority == 3)
	{
		if (opt_target == 0)
		{
			// ʵ������Ӵ�С
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (ratio_vm_data[j] > ratio_vm_data[i]) // ����cpu�����ʴ���ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
					else if (ratio_vm_data[j] == ratio_vm_data[i] && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // ����cpu���,mem����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
			// ʵ������Ӵ�С
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (ratio_vm_data[j] < ratio_vm_data[i]) // ����mem����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
					else if (ratio_vm_data[j] == ratio_vm_data[i] && inputFlavor[j].mem_size > inputFlavor[i].mem_size) // mem��� ����CPU����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
	// ���÷����ĵ�����  ��ratio_vm_diff�Ĵ�С��
	if (Flag_put_priority == 4)
	{
		if (opt_target == 0)
		{
			// ʵ������Ӵ�С
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (ratio_vm_diff_data[j] < ratio_vm_diff_data[i]) // ����cpu�����ʴ���ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
					else if (ratio_vm_diff_data[j] == ratio_vm_diff_data[i] && vm_size_data[j] > vm_size_data[i]) // ����cpu���,mem����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
			// ʵ������Ӵ�С
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (ratio_vm_diff_data[j] < ratio_vm_diff_data[i]) // ����mem����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
					else if (ratio_vm_diff_data[j] == ratio_vm_diff_data[i] && vm_size_data[j] > vm_size_data[i]) // mem��� ����CPU����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
	// ���÷����������  ��vm_size_data��Ĵ�С��
	if (Flag_put_priority == 5)
	{
		if (opt_target == 0)
		{
			// ʵ������Ӵ�С
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (vm_size_data[j] > vm_size_data[i]) // ����cpu����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
					else if (vm_size_data[j] == vm_size_data[i] && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // ����cpu���,mem����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
			// ʵ������Ӵ�С
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (vm_size_data[j] > vm_size_data[i]) // ����mem����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
					else if (vm_size_data[j] == vm_size_data[i] && inputFlavor[j].mem_size > inputFlavor[i].mem_size) // mem��� ����CPU����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
	/***********************************************************���򲿷�*****************************************************************/

	if (Flag_put_priority == 2)
	{
		if (opt_target == 0)
		{
			// ʵ������Ӵ�С
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // ����cpu����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;
					}
					else if (inputFlavor[j].cpu_core_num == inputFlavor[i].cpu_core_num && inputFlavor[j].mem_size > inputFlavor[i].mem_size) // ����cpu���,mem����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
			// ʵ������Ӵ�С
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (inputFlavor[j].mem_size > inputFlavor[i].mem_size) // ����mem����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
						tmp_require = require_vm[j];
						require_vm[j] = require_vm[i];
						require_vm[i] = tmp_require;

						tmp_require = vm_put_priority[j];
						vm_put_priority[j] = vm_put_priority[i];
						vm_put_priority[i] = tmp_require;
					}
					else if (inputFlavor[j].mem_size == inputFlavor[i].mem_size && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // mem��� ����CPU����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
	// ���÷�����������  ��ratio_vm�Ĵ�С��
	if (Flag_put_priority != 2)
	{
		if (opt_target == 0)
		{
			// ʵ������Ӵ�С
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (ratio_vm_data[j] < ratio_vm_data[i]) // ����cpu�����ʴ���ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
					else if (ratio_vm_data[j] == ratio_vm_data[i] && inputFlavor[j].cpu_core_num > inputFlavor[i].cpu_core_num) // ����cpu���,mem����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
			// ʵ������Ӵ�С
			for (int i = 0; i < inputcontrol.flavorMaxnum - 1; i++)
			{
				for (int j = i + 1; j < inputcontrol.flavorMaxnum; j++)
				{
					if (ratio_vm_data[j] > ratio_vm_data[i]) // ����mem����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
					else if (ratio_vm_data[j] == ratio_vm_data[i] && inputFlavor[j].mem_size > inputFlavor[i].mem_size) // mem��� ����CPU����ǰ��,����
					{
						Flavor tmp_flavor;
						// ���ܱ�λ�ý���
						tmp_flavor = inputFlavor[j];
						inputFlavor[j] = inputFlavor[i];
						inputFlavor[i] = tmp_flavor;

						int tmp_require;
						// ����λ�ý���
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
�����㷨:˳�����
����:require_vm ���������������
num_vm �����������
resource_pm Ӳ������������
opt_target �Ż���Դ cpu��mem
inputFlavor ��������������� name �� cup �� mem
result_save �洢������ݵĵ�ַ
*/
int putVM_liner(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save) {
	int i, j;
	int resource_cpu_remain_now;  //��ǰ������ʣ��CPU��Դ
	int resource_mem_remain_now; //��ǰ������ʣ��MEM��Դ
	int pm_num = 0;
	//��ʼ��������������Դ
	resource_cpu_remain_now = resource_pm.cpu_core_num;
	resource_mem_remain_now = resource_pm.mem_size << 3;

	for (i = 0; i < num_vm; i++) {
		for (j = 0; j < require_vm[i]; j++) {
			if (resource_cpu_remain_now >= inputFlavor[i].cpu_core_num
				&& resource_mem_remain_now >= inputFlavor[i].mem_size) { //������Դ�㹻
				resource_cpu_remain_now = resource_cpu_remain_now - inputFlavor[i].cpu_core_num;
				resource_mem_remain_now = resource_mem_remain_now - inputFlavor[i].mem_size;
				result_save[pm_num*num_vm + i]++;
			}
			else { //������Դ����
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
����Ŀ��:���������
����:	��̬�滮
����:
*require_vm		���������
num_of_total_vm  �������������
num_vm			�������������
*inputFlavor	��������ܱ�
resource_pm		���������ܽṹ��
opt_target		�Ż�Ŀ�� 0:CPU 1:MEM
*result_save	���������ݱ�(���)
���:	�����ķ���������
*/
int putVM_dynamicpro(int *require_vm, int num_of_total_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save) {
	int server_num = 0;  //����������
	int *require_vm_copy = new int[num_vm];
	for (int i = 0; i < num_vm; i++) {
		require_vm_copy[i] = require_vm[i];
	}
	while (1) {
		/*������ʽ��������������г�һ��*/
		int num_of_total_vm_point = 1; //Ԥ��������������
		num_of_total_vm = 0;//ͳ�����������
		for (int i = 0; i < num_vm; i++) {
			for (int j = 0; j < require_vm_copy[i]; j++) {
				num_of_total_vm++;
			}
		}
		Flavor *list_danmicpro = new Flavor[num_of_total_vm + 1]; //��̬�滮��������������� һ��һ���Ž�ȥ,�����СΪ���������+1
		Flavor *list_danmicpro_ori = list_danmicpro;  //����ԭʼָ��
		list_danmicpro[0].cpu_core_num = 0;
		list_danmicpro[0].mem_size = 0;
		for (int i = 0; i < num_vm; i++) {
			for (int j = 0; j < require_vm_copy[i]; j++) {
				list_danmicpro[num_of_total_vm_point] = inputFlavor[i];
				num_of_total_vm_point++;
				//printf("\n");
				//printf("�����%d  cpu: %d   mem: %d", num_of_total_vm_point - 1, list_danmicpro[num_of_total_vm_point - 1].cpu_core_num, list_danmicpro[num_of_total_vm_point - 1].mem_size);
			}
		}
		/*��̬�滮��ʼ��*/
		int restict_cpu = resource_pm.cpu_core_num; //������cpu����
		int restict_mem = resource_pm.mem_size;   //������mem����
		int *f = new int[(restict_cpu + 1)*(restict_mem + 1)];
		char *chose_or_no = new char[(num_of_total_vm_point + 1)*(restict_cpu + 1)*(restict_mem + 1)];
		int *f_ori = f;   //����ԭʼָ��
		char *chose_or_no_ori = chose_or_no;   //����ԭʼָ��
		for (int i = 0; i < (num_of_total_vm_point + 1)*(restict_cpu + 1)*(restict_mem + 1); i++) {
			chose_or_no[i] = 0;
		}
		for (int i = 0; i < (restict_cpu + 1)*(restict_mem + 1); i++) {
			f[i] = 0;
		}
		/*�滮����*/
		for (int i = 1; i <= num_of_total_vm; i++) {
			for (int j = restict_cpu; j >= list_danmicpro[i].cpu_core_num; j--) {
				for (int k = restict_mem; k >= list_danmicpro[i].mem_size; k--) {
					int value1, value2;
					if (opt_target == 0) {  //�Ż�cpu
						value1 = f[(j - list_danmicpro[i].cpu_core_num)*(restict_mem + 1) + (k - list_danmicpro[i].mem_size)] + list_danmicpro[i].cpu_core_num;
					}
					else {   //�Ż��ڴ�
						value1 = f[(j - list_danmicpro[i].cpu_core_num)*(restict_mem + 1) + (k - list_danmicpro[i].mem_size)] + list_danmicpro[i].mem_size;
					}
					value2 = f[j*(restict_mem + 1) + k];
					if (value1 > value2) {   //ȡ��һ��
						f[j*(restict_mem + 1) + k] = value1;
						chose_or_no[i*(restict_cpu + 1)*(restict_mem + 1) + j * (restict_mem + 1) + k] = 1;
					}
					else {      //��ȡ��һ��
						f[j*(restict_mem + 1) + k] = value2;
						chose_or_no[i*(restict_cpu + 1)*(restict_mem + 1) + j * (restict_mem + 1) + k] = 0;
					}
				}
			}
		}
		/*�ܽ�ѡȡ*/
		int out_put[10000] = { 0 };  //ѡ���ļ�����������룬���10000�������
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
		//printf("ȡ��ȡ��\n");
		for (int j = 0; j < num_of_total_vm_point; j++) {
			//printf("%d ", out_put[j]);
		}
		//printf("\n�������ܣ�\n");
		for (int j = 0; j < num_of_total_vm_point; j++) {
			//printf("%d ", list_danmicpro[j].cpu_core_num);
		}

		/*һ�ֶ�̬�滮�������������result_save*/
		for (int i = 0; i < num_of_total_vm_point; i++) {
			if (out_put[i] == 1) {
				for (int j = 0; j < num_vm; j++) {
					if (list_danmicpro[i].flavor_name == inputFlavor[j].flavor_name) { //ƥ������
						require_vm_copy[j]--; //��ǰ����������
						result_save[server_num*num_vm + j]++;
						break;
					}
				}
			}
		}
		/*�����Ƿ����·�����*/
		char creat_new_py_flag = 0; //�����Ƿ����·�����
		for (int i = 0; i < num_vm; i++) {
			if (require_vm_copy[i] != 0) {
				creat_new_py_flag = 1;
				break;
			}
		}
		/*�ͷŶ��ڴ�*/
		delete[] list_danmicpro_ori;
		delete[] chose_or_no_ori;
		delete[] f_ori;
		/*�����·�����*/
		if (creat_new_py_flag == 1) {  //���������û�з�����
			server_num++;
		}
		else {  //���������break��while
			break;
		}
	}

	return server_num + 1;
}




int putVM_dynamicpro_ratio_guided(int *require_vm, int num_of_total_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int *vm_put_priority) {


	int *require_vm_temp = new int[inputcontrol.flavorMaxnum];
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)	require_vm_temp[i] = require_vm[vm_put_priority[i]];
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)   require_vm[i] = require_vm_temp[i];


	int server_num = 0;  //����������
	int *require_vm_copy = new int[num_vm];
	for (int i = 0; i < num_vm; i++) {
		require_vm_copy[i] = require_vm[i];
	}
	while (1) {
		/*������ʽ��������������г�һ��*/
		int num_of_total_vm_point = 1; //Ԥ��������������
		num_of_total_vm = 0;//ͳ�����������
		for (int i = 0; i < num_vm; i++) {
			for (int j = 0; j < require_vm_copy[i]; j++) {
				num_of_total_vm++;
			}
		}
		Flavor *list_danmicpro = new Flavor[num_of_total_vm + 1]; //��̬�滮��������������� һ��һ���Ž�ȥ,�����СΪ���������+1
		Flavor *list_danmicpro_ori = list_danmicpro;  //����ԭʼָ��
		list_danmicpro[0].cpu_core_num = 0;
		list_danmicpro[0].mem_size = 0;
		for (int i = 0; i < num_vm; i++) {
			for (int j = 0; j < require_vm_copy[i]; j++) {
				list_danmicpro[num_of_total_vm_point] = inputFlavor[i];
				num_of_total_vm_point++;
				//printf("\n");
				//printf("�����%d  cpu: %d   mem: %d", num_of_total_vm_point - 1, list_danmicpro[num_of_total_vm_point - 1].cpu_core_num, list_danmicpro[num_of_total_vm_point - 1].mem_size);
			}
		}
		/*��̬�滮��ʼ��*/
		int restict_cpu = resource_pm.cpu_core_num; //������cpu����
		int restict_mem = resource_pm.mem_size;   //������mem����
		int *f = new int[(restict_cpu + 1)*(restict_mem + 1)];
		char *chose_or_no = new char[(num_of_total_vm_point + 1)*(restict_cpu + 1)*(restict_mem + 1)];
		int *f_ori = f;   //����ԭʼָ��
		char *chose_or_no_ori = chose_or_no;   //����ԭʼָ��
		for (int i = 0; i < (num_of_total_vm_point + 1)*(restict_cpu + 1)*(restict_mem + 1); i++) {
			chose_or_no[i] = 0;
		}
		for (int i = 0; i < (restict_cpu + 1)*(restict_mem + 1); i++) {
			f[i] = 0;
		}
		/*�滮����*/
		for (int i = 1; i <= num_of_total_vm; i++) {
			for (int j = restict_cpu; j >= list_danmicpro[i].cpu_core_num; j--) {
				for (int k = restict_mem; k >= list_danmicpro[i].mem_size; k--) {
					int value1, value2;
					if (opt_target == 0) {  //�Ż�cpu
						value1 = f[(j - list_danmicpro[i].cpu_core_num)*(restict_mem + 1) + (k - list_danmicpro[i].mem_size)] + list_danmicpro[i].cpu_core_num;
					}
					else {   //�Ż��ڴ�
						value1 = f[(j - list_danmicpro[i].cpu_core_num)*(restict_mem + 1) + (k - list_danmicpro[i].mem_size)] + list_danmicpro[i].mem_size;
					}
					value2 = f[j*(restict_mem + 1) + k];
					if (value1 > value2) {   //ȡ��һ��
						f[j*(restict_mem + 1) + k] = value1;
						chose_or_no[i*(restict_cpu + 1)*(restict_mem + 1) + j * (restict_mem + 1) + k] = 1;
					}
					else {      //��ȡ��һ��
						f[j*(restict_mem + 1) + k] = value2;
						chose_or_no[i*(restict_cpu + 1)*(restict_mem + 1) + j * (restict_mem + 1) + k] = 0;
					}
				}
			}
		}
		/*�ܽ�ѡȡ*/
		int out_put[10000] = { 0 };  //ѡ���ļ�����������룬���10000�������
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
		//printf("ȡ��ȡ��\n");
		for (int j = 0; j < num_of_total_vm_point; j++) {
			//printf("%d ", out_put[j]);
		}
		//printf("\n�������ܣ�\n");
		for (int j = 0; j < num_of_total_vm_point; j++) {
			//printf("%d ", list_danmicpro[j].cpu_core_num);
		}

		/*һ�ֶ�̬�滮�������������result_save*/
		for (int i = 0; i < num_of_total_vm_point; i++) {
			if (out_put[i] == 1) {
				for (int j = 0; j < num_vm; j++) {
					if (list_danmicpro[i].flavor_name == inputFlavor[j].flavor_name) { //ƥ������
						require_vm_copy[j]--; //��ǰ����������
						result_save[server_num*num_vm + j]++;
						break;
					}
				}
			}
		}
		/*�����Ƿ����·�����*/
		char creat_new_py_flag = 0; //�����Ƿ����·�����
		for (int i = 0; i < num_vm; i++) {
			if (require_vm_copy[i] != 0) {
				creat_new_py_flag = 1;
				break;
			}
		}
		/*�ͷŶ��ڴ�*/
		delete[] list_danmicpro_ori;
		delete[] chose_or_no_ori;
		delete[] f_ori;
		/*�����·�����*/
		if (creat_new_py_flag == 1) {  //���������û�з�����
			server_num++;
		}
		else {  //���������break��while
			break;
		}
	}

	return server_num + 1;
}




void print_resource(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int num_py) {
	int i, j;
	/*��Դ��ʼ��*/
	double *py_ratio_cpu = new double[num_py];  //һ����������cpu��Դ������
	double *py_ratio_mem = new double[num_py];  //һ�����������ڴ���Դ������ 
	int *resource_rest_cpu = new int[num_py];   //һ����������cpuʣ����Դ
	int *resource_rest_mem = new int[num_py];   //һ�����������ڴ�ʣ����Դ
	int *flavor_num_in_py = new int[num_py];    //һ���������а��������������
	int flavor_totol_mun = 0;  //���������������
	int num_py_new = num_py; //�µ�����������
	for (i = 0; i < num_py; i++) {
		resource_rest_cpu[i] = resource_pm.cpu_core_num;
		resource_rest_mem[i] = resource_pm.mem_size;
		flavor_num_in_py[i] = 0;
	}
	for (i = 0; i < num_vm; i++) {
		flavor_totol_mun += require_vm[i];
	}


	printf("\n\n");
	printf("������\n\n");
	for (i = 0; i < num_vm; i++) {
		printf("���ƣ�flavor%d  ", inputFlavor[i].flavor_name);
		printf("�ڴ棺%d  ", inputFlavor[i].mem_size);
		printf("CPU�� %d  ", inputFlavor[i].cpu_core_num);
		printf("����%d  ", require_vm[i]);
		printf("\n");
	}


	/*����ÿ��Ӳ��������������*/
	for (i = 0; i < num_py; i++) {
		// ��i���������������ʺ�ʣ����Դ 
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
		printf("\n CPU%d�����ʣ�%f  ��ʣ��Դ��%d", i, py_ratio_cpu[i], resource_rest_cpu[i]);
	}
	printf("\n");
	for (i = 0; i < num_py; i++) {
		printf("\n MEM%d�����ʣ�%f  ��ʣ��Դ��%d", i, py_ratio_mem[i], resource_rest_mem[i]);
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
	printf("\n CPU�������ʣ�%f", ratio_cpu);
	printf("\n MEM�������ʣ�%f", ratio_mem);

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
����Ŀ��:�Է��÷������н���
����:	������֤100%������
����:	*require_vm		���������
num_vm			�������������
*inputFlavor	��������ܱ�
resource_pm		���������ܽṹ��
opt_target		�Ż�Ŀ�� 0:CPU 1:MEM
*result_save	���������ݱ�(���)
num_py      	�����ķ���������
���:   ���������������
*/
int putVM_correct(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save, int num_py) {
	int i, j;
	/*��Դ��ʼ��*/
	double *py_ratio_cpu = new double[num_py];  //һ����������cpu��Դ������
	double *py_ratio_mem = new double[num_py];  //һ�����������ڴ���Դ������ 
	int *resource_rest_cpu = new int[num_py];   //һ����������cpuʣ����Դ
	int *resource_rest_mem = new int[num_py];   //һ�����������ڴ�ʣ����Դ
	int *flavor_num_in_py = new int[num_py];    //һ���������а��������������
	int flavor_totol_mun = 0;  //���������������
	int num_py_new = num_py; //�µ�����������
	for (i = 0; i < num_py; i++) {
		resource_rest_cpu[i] = resource_pm.cpu_core_num;
		resource_rest_mem[i] = resource_pm.mem_size;
		flavor_num_in_py[i] = 0;
	}
	for (i = 0; i < num_vm; i++) {
		flavor_totol_mun += require_vm[i];
	}
	/*����ÿ��Ӳ��������������*/
	for (i = 0; i < num_py; i++) {
		// ��i���������������ʺ�ʣ����Դ 
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
		//printf("\n CPU%d�����ʣ�%f  ��ʣ��Դ��%d", i, py_ratio_cpu[i], resource_rest_cpu[i]);
	}
	//printf("\n");
	for (i = 0; i < num_py; i++) {
		//printf("\n MEM%d�����ʣ�%f  ��ʣ��Դ��%d", i, py_ratio_mem[i], resource_rest_mem[i]);
	}


	/*��ʼ����*/
	int opt_sub_timer;  //���Ż������������
	int opt_add_timer;  //���Ż������������
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

	//i = num_py - 1;  //ֻ�������һ��Ӳ�������� 
	i = 0;
	while (i < num_py) {
		opt_sub_timer = flavor_num_in_py[i];
		if (resource_rest_cpu[i] != 0 && resource_rest_mem[i] != 0)
		{ //ʣ����Դ��Ϊ0���ſɽ���
			for (j = 0; j < num_vm; j++) {
				while (inputFlavor[j].cpu_core_num <= resource_rest_cpu[i] && inputFlavor[j].mem_size <= resource_rest_mem[i]) {
					require_vm_copy[j]++;
					result_save_copy[i*num_vm + j]++;
					opt_add_timer++;
					resource_rest_cpu[i] -= inputFlavor[j].cpu_core_num;
					resource_rest_mem[i] -= inputFlavor[j].mem_size;
				}
			}
			if (opt_add_timer <= opt_sub_timer && opt_add_timer != 0) { //ѡ�� ��������Ż�
				for (int kk = 0; kk < num_py; kk++) {
					for (j = 0; j < num_vm; j++) {
						result_save[kk*num_vm + j] = result_save_copy[kk*num_vm + j];
					}
				}
				for (int kk = 0; kk < num_vm; kk++) {
					require_vm[kk] = require_vm_copy[kk];
				}
			}
			else {  //ѡ�� ��������Ż�
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
	/*��Դ��ʼ��*/
	double *py_ratio_cpu = new double[num_py];  //һ����������cpu��Դ������
	double *py_ratio_mem = new double[num_py];  //һ�����������ڴ���Դ������ 
	int *resource_rest_cpu = new int[num_py];   //һ����������cpuʣ����Դ
	int *resource_rest_mem = new int[num_py];   //һ�����������ڴ�ʣ����Դ
	int *flavor_num_in_py = new int[num_py];    //һ���������а��������������
	int flavor_totol_mun = 0;  //���������������
	int num_py_new = num_py; //�µ�����������
	for (i = 0; i < num_py; i++) {
		resource_rest_cpu[i] = resource_pm.cpu_core_num;
		resource_rest_mem[i] = resource_pm.mem_size;
		flavor_num_in_py[i] = 0;
	}
	for (i = 0; i < num_vm; i++) {
		flavor_totol_mun += require_vm[i];
	}
	/*����ÿ��Ӳ��������������*/
	for (i = 0; i < num_py; i++) {
		// ��i���������������ʺ�ʣ����Դ 
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
		//printf("\n CPU%d�����ʣ�%f  ��ʣ��Դ��%d", i, py_ratio_cpu[i], resource_rest_cpu[i]);
	}
	//printf("\n");
	for (i = 0; i < num_py; i++) {
		//printf("\n MEM%d�����ʣ�%f  ��ʣ��Դ��%d", i, py_ratio_mem[i], resource_rest_mem[i]);
	}


	/*��ʼ����*/
	int opt_sub_timer;  //���Ż������������
	int opt_add_timer;  //���Ż������������
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

	//i = num_py - 1;  //ֻ�������һ��Ӳ�������� 
	i = 0;
	while (i < num_py) {
		opt_sub_timer = flavor_num_in_py[i];
		if (resource_rest_cpu[i] != 0 && resource_rest_mem[i] != 0)
		{ //ʣ����Դ��Ϊ0���ſɽ���
			for (j = 0; j < num_vm; j++) {
				while (inputFlavor[j].cpu_core_num <= resource_rest_cpu[i] && inputFlavor[j].mem_size <= resource_rest_mem[i]) {
					require_vm_copy[j]++;
					result_save_copy[i*num_vm + j]++;
					opt_add_timer++;
					resource_rest_cpu[i] -= inputFlavor[j].cpu_core_num;
					resource_rest_mem[i] -= inputFlavor[j].mem_size;
				}
			}
			if (opt_add_timer <= opt_sub_timer && opt_add_timer != 0) { //ѡ�� ��������Ż�
				for (int kk = 0; kk < num_py; kk++) {
					for (j = 0; j < num_vm; j++) {
						result_save[kk*num_vm + j] = result_save_copy[kk*num_vm + j];
					}
				}
				for (int kk = 0; kk < num_vm; kk++) {
					require_vm[kk] = require_vm_copy[kk];
				}
			}
			else {  //ѡ�� ��������Ż�
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
����Ŀ��:��Ⱥʹ��̰�ľ�������
*/
int putVM_compet_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	// ���� require_vm ��������Ⱥ������
	int population_num = 16;

	// ����������
	int resize_begin = -2;
	int resize_end = 1;



	// һ��⵽�������ݱ�
	int **result_predict_super = new int *[population_num];
	for (int i = 0; i < population_num; i++)    result_predict_super[i] = new int[inputcontrol.flavorMaxnum];
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			result_predict_super[i][j] = require_vm[j];
		}
	}
	//�Ҽ�����
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

	// ��Զ����Ķ��ַ��÷���������
	int max_serve_py = 2000;  // ���������ĸ���!!!
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
		printf("case %d��result_serve_num: %d\n", i + 1, result_serve_num[i]);
	}



	double **py_ratio_cpu = new double *[population_num];  // һ����������cpu��Դ������
	double **py_ratio_mem = new double *[population_num];  // һ�����������ڴ���Դ������ 

	double *py_ratio_cpu_final = new double[population_num];  //
	double *py_ratio_mem_final = new double[population_num];  // 

	for (int i = 0; i < population_num; i++)
	{
		py_ratio_cpu[i] = new double[result_serve_num[i]];
		py_ratio_mem[i] = new double[result_serve_num[i]];

		for (int j = 0; j < result_serve_num[i]; j++)
		{
			// ��j���������������ʺ�ʣ����Դ 
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
			// �������ϵĵ���������
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

		// ���յ�������
		py_ratio_cpu_final[i] = sum_ratio_cpu / result_serve_num[i];
		py_ratio_mem_final[i] = sum_ratio_mem / result_serve_num[i];
		printf("case %d cpu ratio��%f\n", i + 1, py_ratio_cpu_final[i]);
		printf("case %d mem ratio��%f\n", i + 1, py_ratio_mem_final[i]);

	}

	int return_serve_num;

	// �Ż� cpu or mem��
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

		// ���и��ƣ�����
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = result_predict_super[temp][i];
		for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = result_save_super[temp][i];

		print_data(result_save_super[temp], result_serve_num[temp]);
		print_data(result_save, return_serve_num);

		printf("FINAL case %d ��", temp);
		// ��ӡģ��Ԥ����
		printf("[");
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
		printf("]\n");
		printf("FINAL case ratio��%f\n", py_ratio_cpu_final[temp]);

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

		// ���и��ƣ�����
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = result_predict_super[temp][i];
		for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = result_save_super[temp][i];

		// print_data(result_save_super[temp], result_serve_num[temp]);
		// print_data(result_save, return_serve_num);
		printf("FINAL case��");
		// ��ӡģ��Ԥ����
		printf("[");
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
		printf("]\n");
		printf("FINAL case ratio��%f\n", py_ratio_mem_final[temp]);
	}


	return return_serve_num;
}

/*
����Ŀ��: ���������� ��Ⱥʹ��̰�ľ�������
*/
int putVM_score_compet_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	// ���� require_vm ��������Ⱥ������
	int population_num = 16;

	// ����������
	int resize_begin = -2;
	int resize_end = 1;

	// һ��⵽�������ݱ�
	int **result_predict_super = new int *[population_num];
	for (int i = 0; i < population_num; i++)    result_predict_super[i] = new int[inputcontrol.flavorMaxnum];
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			result_predict_super[i][j] = require_vm[j];
		}
	}
	//�Ҽ�����
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

	// ��Զ����Ķ��ַ��÷���������
	int max_serve_py = 2000;  // ���������ĸ���!!!
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
		//����
		result_serve_num[i] = putVM_correct(result_predict_super[i], inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i], result_serve_num[i]);

		//�÷�
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
		printf("case %d��result_serve_num: %d\n", i + 1, result_serve_num[i]);
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

	// ���и��ƣ�����
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = result_predict_super[temp][i];
	for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = result_save_super[temp][i];

	//print_data(result_save_super[temp], result_serve_num[temp]);
	//print_data(result_save, return_serve_num);

	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf("%f \n", score_result[i]);

	printf("FINAL case %d ��", temp);
	// ��ӡģ��Ԥ����
	printf("[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
	printf("]\n");
	printf("FINAL case score��%f\n", score_result[temp]);


	return return_serve_num;
}


/*
����Ŀ��:��Ⱥʹ�ö��龺������
*/
int putVM_compet_dynamicpro(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	// ���� require_vm ��������Ⱥ������
	int population_num = 25;

	// ����������
	int resize_begin = -2;
	int resize_end = 2;



	// һ��⵽�������ݱ�
	int **result_predict_super = new int *[population_num];
	for (int i = 0; i < population_num; i++)    result_predict_super[i] = new int[inputcontrol.flavorMaxnum];
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			result_predict_super[i][j] = require_vm[j];
		}
	}
	//�Ҽ�����
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

	// ��Զ����Ķ��ַ��÷���������
	int max_serve_py = 2000;  // ���������ĸ���!!!
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
		// ����һ�ݳ�ʼ˳��
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
		//printf("case %d��result_serve_num: %d\n", i + 1, result_serve_num[i]);
	}



	double **py_ratio_cpu = new double *[population_num];  // һ����������cpu��Դ������
	double **py_ratio_mem = new double *[population_num];  // һ�����������ڴ���Դ������ 

	double *py_ratio_cpu_final = new double[population_num];  //
	double *py_ratio_mem_final = new double[population_num];  // 

	for (int i = 0; i < population_num; i++)
	{
		py_ratio_cpu[i] = new double[result_serve_num[i]];
		py_ratio_mem[i] = new double[result_serve_num[i]];

		for (int j = 0; j < result_serve_num[i]; j++)
		{
			// ��j���������������ʺ�ʣ����Դ 
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
			// �������ϵĵ���������
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

		// ���յ�������
		py_ratio_cpu_final[i] = sum_ratio_cpu / result_serve_num[i];
		py_ratio_mem_final[i] = sum_ratio_mem / result_serve_num[i];
		//printf("case %d cpu ratio��%f\n", i + 1, py_ratio_cpu_final[i]);
		//printf("case %d mem ratio��%f\n", i + 1, py_ratio_mem_final[i]);

	}

	int return_serve_num;

	// �Ż� cpu or mem��
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

		// ���и��ƣ�����
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = result_predict_super[temp][i];
		for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = result_save_super[temp][i];

		//print_data(result_save_super[temp], result_serve_num[temp]);
		//print_data(result_save, return_serve_num);

		printf("FINAL case %d ��", temp);
		// ��ӡģ��Ԥ����
		printf("[");
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
		printf("]\n");
		printf("FINAL case ratio��%f\n", py_ratio_cpu_final[temp]);

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

		// ���и��ƣ�����
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = result_predict_super[temp][i];
		for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = result_save_super[temp][i];

		// print_data(result_save_super[temp], result_serve_num[temp]);
		// print_data(result_save, return_serve_num);
		printf("FINAL case��");
		// ��ӡģ��Ԥ����
		printf("[");
		for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
		printf("]\n");
		printf("FINAL case ratio��%f\n", py_ratio_mem_final[temp]);
	}


	return return_serve_num;
}

/*
����Ŀ��:������������Ⱥʹ�ö��龺������
*/
int putVM_score_compet_dynamicpro(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	// ���� require_vm ��������Ⱥ������
	int population_num = 25;

	// ����������
	int resize_begin = -2;
	int resize_end = 2;



	// һ��⵽�������ݱ�
	int **result_predict_super = new int *[population_num];
	for (int i = 0; i < population_num; i++)    result_predict_super[i] = new int[inputcontrol.flavorMaxnum];
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			result_predict_super[i][j] = require_vm[j];
		}
	}
	//�Ҽ�����
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

	// ��Զ����Ķ��ַ��÷���������
	int max_serve_py = 2000;  // ���������ĸ���!!!
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
		// ����һ�ݳ�ʼ˳��
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

		//����
		result_serve_num[i] = putVM_correct(result_predict_super[i], inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i], result_serve_num[i]);

		//�÷�
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
		//printf("case %d��result_serve_num: %d\n", i + 1, result_serve_num[i]);
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

	// ���и��ƣ�����
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = result_predict_super[temp][i];
	for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = result_save_super[temp][i];

	//print_data(result_save_super[temp], result_serve_num[temp]);
	//print_data(result_save, return_serve_num);

	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf("%f \n", score_result[i]);

	printf("FINAL case %d ��", temp);
	// ��ӡģ��Ԥ����
	printf("[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
	printf("]\n");
	printf("FINAL case score��%f\n", score_result[temp]);


	return return_serve_num;
}

/*
require_vm :����Ԥ�ⲿ��Ԥ��ĸ��壬���ǽ�����Ϊ��ʼ�ĵ�һ��
*/

int gene_dynamicpro(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{


	//�Ŵ���������
	const int gene_total_num = 20;

	// ��Ⱥ��ģ������
	const int population_num = 25;

	// ѡ������֮��ʣ�����Ⱥ��������
	const int population_last = 12;


	// ����������������ݣ�
	const int ladder_rand = 20;


	// ��ʼ��һ��������������
	const int resize_begin = -1;
	const int resize_end = 3;

	// ��Ⱥ��ȫ������Ļ�����Ϣ
	GeneInfoUnit *geneinfounit = new GeneInfoUnit[population_num];
	// ��Ⱥ��ʣ�����Ļ�����Ϣ
	// GeneInfoUnit *geneinfounit_last = new GeneInfoUnit[population_last];

	// ��Ⱥ�и��������Ϣ�̳�
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			// ��ʾ��i������ĵ�j������  �ӳ�ʼ�̳�����
			geneinfounit[i].gene[j] = require_vm[j];
		}
	}

	//�Ҽ���������
	int max_count_a_num = 0;
	int max_count_b_num = 0;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_a_num] <= require_vm[i])  max_count_a_num = i;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_b_num] <= require_vm[i] && i != max_count_a_num)  max_count_b_num = i;

	// ͨ�������������������һ����Ⱥ
	for (int i = resize_begin; i <= resize_end; i++)
	{
		for (int j = resize_begin; j <= resize_end; j++)
		{
			geneinfounit[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)].gene[max_count_a_num] += i;
			geneinfounit[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)].gene[max_count_b_num] += j;
		}
	}

	//��ӡ��һ��
	for (int i = 0; i < population_num; i++)
	{
		print_data(geneinfounit[i].gene, 1);
	}

	// int *result_serve_num = new int[population_num];  //���صķ���������
	// double *score_result = new double[population_num];  //�÷ֵĽ������Ӧ�ȣ�
	Flavor *inputFlavor_data = new Flavor[inputcontrol.flavorMaxnum];  //�����ʼ����������ܱ���Ϣ

																	   //��ʼ�Ŵ����� 	 gene_temp_num : ��ǰ��Ⱥ���ڵڼ����� ��ʼΪ��1��
	for (int gene_temp_num = 1; gene_temp_num <= gene_total_num; )
	{
		//������Ⱥ����Ӧ����ֵ
		double sum_value_fit = 0;

		//��ʼ������Ⱥ��ÿ���������Ӧ��
		for (int i = 0; i < population_num; i++)
		{
			if (i == 0) // ��һ�ν���������ʼ����������ܱ���Ϣ
			{
				for (int j = 0; j<inputcontrol.flavorMaxnum; j++)
				{
					inputFlavor_data[j].cpu_core_num = inputFlavor[j].cpu_core_num;
					inputFlavor_data[j].mem_size = inputFlavor[j].mem_size;
					inputFlavor_data[j].flavor_name = inputFlavor[j].flavor_name;
				}
			}
			// �Ը���ķ��÷������г�ʼ��
			for (int j = 0; j < 16 * 2000; j++)  geneinfounit[i].result_save_gene[j] = 0;

			int num_of_Serve_vm = 0;
			//print_data(result_predict_super[i], 1);

			print_data(geneinfounit[i].gene, 1);
			// ����
			geneinfounit[i].result_serve_num = putVM_dynamicpro(geneinfounit[i].gene, num_of_Serve_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, geneinfounit[i].result_save_gene);
			print_data(geneinfounit[i].gene, 1);
			//����
			geneinfounit[i].result_serve_num = putVM_correct(geneinfounit[i].gene, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, geneinfounit[i].result_save_gene, geneinfounit[i].result_serve_num);
			print_data(geneinfounit[i].gene, 1);
			//������Ӧ������
			geneinfounit[i].value_fit = get_score(require_vm, geneinfounit[i].gene, inputFlavor, opt_target, geneinfounit[i].result_serve_num);

			//print_data(result_predict_super[i], 1);

			if ((i != population_num - 1) || (gene_temp_num != gene_total_num))
				for (int j = 0; j<inputcontrol.flavorMaxnum; j++)
				{
					inputFlavor[j].cpu_core_num = inputFlavor_data[j].cpu_core_num;
					inputFlavor[j].mem_size = inputFlavor_data[j].mem_size;
					inputFlavor[j].flavor_name = inputFlavor_data[j].flavor_name;
				}

			// ������Ⱥ��Ӧ�ȵ��ܺ�
			sum_value_fit += geneinfounit[i].value_fit;
		}
		// ��ӡ��Ӧ�ȵ�ֵ
		//for (int a = 0; a < population_num; a++)  printf("%f ", geneinfounit[a].value_fit);
		printf("��%d����Ⱥ��ȫ�������ƽ����Ӧ�� :%f\n", gene_temp_num, sum_value_fit / population_num);

		// ��һ����ʼ����������
		gene_temp_num++;

		//	��������һ�ε�������ô�Ͳ���Ҫ�����µĸ��壡��
		if (gene_temp_num != gene_total_num)
		{
			// �������и����������� ��ʼ��
			// int *last_count = new int[population_num];
			// for (int a = 0; a < population_num; a++)  last_count[a] = a;

			// �������֮�������ը�������֣�ɸѡ������壡����
			//����
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
			// ��ӡ����֮��ģ�����
			// for (int a = 0; a < population_num; a++)  printf("%f ", geneinfounit[a].value_fit);

			//��Ⱥ��������ķ���������
			printf("��%d����Ⱥ��������ĸ��� :%f\n", gene_temp_num - 1, geneinfounit[0].value_fit);

			// �����µĸ���
			/*  ע�⣺geneinfounit��Ⱥ��ǰpopulation_last�Ǳ��������ģ�����ĸ��彫��ȡ����������*/
			/*     ͨ��������������������������µĸ��壨������Ⱥ��    */
			double coeff_anneal = 0;

			// ��Ⱥ�Ĳ�������������
			for (int i = population_last; i < population_num; i++)
			{
				for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
				{
					//���������
					srand((unsigned)time(NULL));
					//ģ���˻�
					coeff_anneal = ((double)(rand() % ladder_rand) / ladder_rand - 0.5)*0.5*((double)(gene_total_num - gene_temp_num + 1) / gene_total_num);
					geneinfounit[i].gene[j] = (int)(geneinfounit[i%population_last].gene[j] + coeff_anneal * geneinfounit[i%population_last].gene[j]);
					if (geneinfounit[i].gene[j] < 0)  geneinfounit[i].gene[j] = 0;
				}
			}

			// ��Ⱥ�̶�������ȥ������

		}
		else
		{
			printf("��%d����Ⱥ��������ĸ��� :%f\n", gene_temp_num - 1, geneinfounit[0].value_fit);
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

	// ���и��ƣ�����
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = geneinfounit[temp].gene[i];
	for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = geneinfounit[temp].result_save_gene[i];

	//print_data(result_save_super[temp], result_serve_num[temp]);
	//print_data(result_save, return_serve_num);

	// for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf("%f \n", score_result[i]);

	printf("FINAL case %d ��", temp);
	// ��ӡģ��Ԥ����
	printf("[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
	printf("]\n");
	printf("FINAL case score��%f\n", geneinfounit[temp].value_fit);


	return return_serve_num;
}

int gene_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{


	//�Ŵ���������
	const int gene_total_num = 20;

	// ��Ⱥ��ģ������
	const int population_num = 25;

	// ѡ������֮��ʣ�����Ⱥ��������
	const int population_last = 12;


	// ����������������ݣ�
	const int ladder_rand = 20;


	putVM_seq(require_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);//ԭ����

																												   // ��ʼ��һ��������������
	const int resize_begin = -1;
	const int resize_end = 3;

	// ��Ⱥ��ȫ������Ļ�����Ϣ
	GeneInfoUnit *geneinfounit = new GeneInfoUnit[population_num];
	// ��Ⱥ��ʣ�����Ļ�����Ϣ
	// GeneInfoUnit *geneinfounit_last = new GeneInfoUnit[population_last];

	// ��Ⱥ�и��������Ϣ�̳�
	for (int i = 0; i < population_num; i++)
	{
		for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
		{
			// ��ʾ��i������ĵ�j������  �ӳ�ʼ�̳�����
			geneinfounit[i].gene[j] = require_vm[j];
		}
	}

	//�Ҽ���������
	int max_count_a_num = 0;
	int max_count_b_num = 0;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_a_num] <= require_vm[i])  max_count_a_num = i;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)
		if (require_vm[max_count_b_num] <= require_vm[i] && i != max_count_a_num)  max_count_b_num = i;

	// ͨ�������������������һ����Ⱥ
	for (int i = resize_begin; i <= resize_end; i++)
	{
		for (int j = resize_begin; j <= resize_end; j++)
		{
			geneinfounit[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)].gene[max_count_a_num] += i;
			geneinfounit[(i - resize_begin) * (resize_end - resize_begin + 1) + (j - resize_begin)].gene[max_count_b_num] += j;
		}
	}

	//��ӡ��һ��
	for (int i = 0; i < population_num; i++)
	{
		print_data(geneinfounit[i].gene, 1);
	}

	// int *result_serve_num = new int[population_num];  //���صķ���������
	// double *score_result = new double[population_num];  //�÷ֵĽ������Ӧ�ȣ�
	Flavor *inputFlavor_data = new Flavor[inputcontrol.flavorMaxnum];  //�����ʼ����������ܱ���Ϣ

																	   //��ʼ�Ŵ����� 	 gene_temp_num : ��ǰ��Ⱥ���ڵڼ����� ��ʼΪ��1��
	for (int gene_temp_num = 1; gene_temp_num <= gene_total_num; )
	{
		//������Ⱥ����Ӧ����ֵ
		double sum_value_fit = 0;

		//��ʼ������Ⱥ��ÿ���������Ӧ��
		for (int i = 0; i < population_num; i++)
		{
			if (i == 0) // ��һ�ν���������ʼ����������ܱ���Ϣ
			{
				for (int j = 0; j<inputcontrol.flavorMaxnum; j++)
				{
					inputFlavor_data[j].cpu_core_num = inputFlavor[j].cpu_core_num;
					inputFlavor_data[j].mem_size = inputFlavor[j].mem_size;
					inputFlavor_data[j].flavor_name = inputFlavor[j].flavor_name;
				}
			}
			// �Ը���ķ��÷������г�ʼ��
			for (int j = 0; j < 16 * 2000; j++)  geneinfounit[i].result_save_gene[j] = 0;

			int num_of_Serve_vm = 0;
			//print_data(result_predict_super[i], 1);

			print_data(geneinfounit[i].gene, 1);


			int *temp_gene = new int[inputcontrol.flavorMaxnum];
			// ����Ԥ����Ϣ���ı䣬��������п�����
			for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
			{
				temp_gene[j] = geneinfounit[i].gene[j];
			}

			//̰��
			// geneinfounit[i].result_serve_num = putVM_greedy(temp_gene, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, geneinfounit[i].result_save_gene);
			putVM_greedy_without_seq(geneinfounit[i].gene, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, geneinfounit[i].result_save_gene);//ԭ����

			print_data(geneinfounit[i].gene, 1);
			//print_data(result_predict_super[i], 1);


			//����
			geneinfounit[i].result_serve_num = putVM_correct(geneinfounit[i].gene, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, geneinfounit[i].result_save_gene, geneinfounit[i].result_serve_num);




			print_data(geneinfounit[i].gene, 1);



			//������Ӧ������
			geneinfounit[i].value_fit = get_score(require_vm, geneinfounit[i].gene, inputFlavor, opt_target, geneinfounit[i].result_serve_num);
			print_data(geneinfounit[i].gene, 1);

			if ((i != population_num - 1) || (gene_temp_num != gene_total_num))
				for (int j = 0; j<inputcontrol.flavorMaxnum; j++)
				{
					inputFlavor[j].cpu_core_num = inputFlavor_data[j].cpu_core_num;
					inputFlavor[j].mem_size = inputFlavor_data[j].mem_size;
					inputFlavor[j].flavor_name = inputFlavor_data[j].flavor_name;
				}

			// ������Ⱥ��Ӧ�ȵ��ܺ�
			sum_value_fit += geneinfounit[i].value_fit;
		}
		// ��ӡ��Ӧ�ȵ�ֵ
		//for (int a = 0; a < population_num; a++)  printf("%f ", geneinfounit[a].value_fit);
		printf("��%d����Ⱥ��ȫ�������ƽ����Ӧ�� :%f\n", gene_temp_num, sum_value_fit / population_num);

		// ��һ����ʼ����������
		gene_temp_num++;

		//	��������һ�ε�������ô�Ͳ���Ҫ�����µĸ��壡��
		if (gene_temp_num != gene_total_num)
		{
			// �������и����������� ��ʼ��
			// int *last_count = new int[population_num];
			// for (int a = 0; a < population_num; a++)  last_count[a] = a;

			// �������֮�������ը�������֣�ɸѡ������壡����
			//����
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
			// ��ӡ����֮��ģ�����
			// for (int a = 0; a < population_num; a++)  printf("%f ", geneinfounit[a].value_fit);

			//��Ⱥ��������ķ���������
			printf("��%d����Ⱥ��������ĸ��� :%f\n", gene_temp_num - 1, geneinfounit[0].value_fit);

			// �����µĸ���
			/*  ע�⣺geneinfounit��Ⱥ��ǰpopulation_last�Ǳ��������ģ�����ĸ��彫��ȡ����������*/
			/*     ͨ��������������������������µĸ��壨������Ⱥ��    */
			double coeff_anneal = 0;
			for (int i = population_last; i < population_num; i++)
			{
				for (int j = 0; j < inputcontrol.flavorMaxnum; j++)
				{
					//���������
					srand((unsigned)time(NULL));
					//ģ���˻�
					coeff_anneal = ((double)(rand() % ladder_rand) / ladder_rand - 0.5)*0.5*((double)(gene_total_num - gene_temp_num + 1) / gene_total_num);
					geneinfounit[i].gene[j] = (int)(geneinfounit[i%population_last].gene[j] + coeff_anneal * geneinfounit[i%population_last].gene[j]);
					if (geneinfounit[i].gene[j] < 0)  geneinfounit[i].gene[j] = 0;
				}
			}
		}
		else
		{
			printf("��%d����Ⱥ��������ĸ��� :%f\n", gene_temp_num - 1, geneinfounit[0].value_fit);
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

	// ���и��ƣ�����
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)    require_vm[i] = geneinfounit[temp].gene[i];
	for (int i = 0; i < return_serve_num * inputcontrol.flavorMaxnum; i++)   result_save[i] = geneinfounit[temp].result_save_gene[i];

	//print_data(result_save_super[temp], result_serve_num[temp]);
	//print_data(result_save, return_serve_num);

	// for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf("%f \n", score_result[i]);

	printf("FINAL case %d ��", temp);
	// ��ӡģ��Ԥ����
	printf("[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
	printf("]\n");
	printf("FINAL case score��%f\n", geneinfounit[temp].value_fit);


	return return_serve_num;
}

/*
����Ŀ��:ģ���˻��㷨SAA ���������
����:	ģ���˻�
����:	*require_vm		���������
num_vm			�������������
resource_pm     ����������
opt_target		�Ż�Ŀ�� 0:CPU 1:MEM
*inputFlavor	��������ܱ�
*result_save	���������ݱ�(���)
���:	�����ķ���������
*/
int putVM_score_SAA_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save) {
	srand((unsigned)time(NULL));

	double max_score = 0;

	int result_serve_num; //���շ�����������
	int max_serve_py = 2000;  //������������

	putVM_seq(require_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);//ԭ����

	double score_final;  //���յ÷�
	int *require_vm_iter = new int[num_vm];
	int *require_vm_copy = new int[num_vm];
	int *require_vm_copy_temp = new int[num_vm];

	/*printf("\npredict final result : [");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
	printf("]\n");*/

	for (int i = 0; i < num_vm; i++) {
		require_vm_iter[i] = require_vm[i];
	}

	float Temperature_first = 1.5;       //��ʼ�¶�
	int Num_of_inner_first = 40;         //��Ȧѭ����ʼ�Ĵ������¶�Խ�͵�������������
	float Step = 0.1;                    //�¶�Temperature�Ĳ���
	float Temperature_end = 0.0001;               //�����¶�
	int *result_save_temp1 = new int[max_serve_py*num_vm]; //ԭ����
	int *result_save_temp2 = new int[max_serve_py*num_vm]; //������
	int result_serve_num1; //ԭ����
	int result_serve_num2; //������
	double score_result1;   //ԭ���е÷�
	double score_result2;   //�����е÷�

	int time_iter_saa = 20; //�ظ��˻����
	for (int iter_i = 0; iter_i < time_iter_saa; iter_i++) {
		/*-----------------------�˻�ʼ---------------------------------------------------*/
		for (int i = 0; i < num_vm; i++) {
			require_vm_copy[i] = require_vm[i];
		}
		float Temperature = Temperature_first;  //��ǰ�¶�
												/*--------------------��ѭ�������¶�----------------------------*/
		int range_rand;  //������ķ�Χ-range_rand��+range_rand
		float range_rand_start = 10; //��ʼ�������Χ
		while (Temperature > Temperature_end) {
			int Num_of_inner = Num_of_inner_first + (int)((Temperature_first - Temperature) * 1 / Step) * 2; //��ѭ������,���¶Ƚ��Ͷ��½�
																											 //int Num_of_inner = Num_of_inner_first;
			int range_rand = (int)(range_rand_start * (Temperature / Temperature_first));
			if (range_rand <= 1) {
				range_rand = 2;
			}
			//��ѭ����ʼ
			for (int i = 0; i < Num_of_inner; i++) {
				//���������
				int num_change = rand() % num_vm;       //�ı�ڼ��������
				int predict_change = rand() % (2 * range_rand) - range_rand;   //-range_rand��+range_rand�������
				while (predict_change == 0) {
					predict_change = rand() % (2 * range_rand) - range_rand;   //ȡ��0����ȡһ��
				}

				//�ı�ԭ����
				for (int ii = 0; ii < num_vm; ii++) {
					require_vm_copy_temp[ii] = require_vm_copy[ii];
				}
				require_vm_copy_temp[num_change] += predict_change;
				if (require_vm_copy_temp[num_change] < 0) {  //�������������С��0
					require_vm_copy_temp[num_change] = 0;
				}
				//��������
				for (int zreo_i = 0; zreo_i < max_serve_py*num_vm; zreo_i++) {
					result_save_temp1[zreo_i] = 0;
					result_save_temp2[zreo_i] = 0;
				}

				/*printf("\n [");
				for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm_copy[i]);
				printf("]\n");*/

				result_serve_num1 = putVM_greedy_without_seq(require_vm_copy, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_temp1);//ԭ����
				result_serve_num2 = putVM_greedy_without_seq(require_vm_copy_temp, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_temp2);//������

																																												  //��ʼ����
				score_result1 = get_score(require_vm, require_vm_copy, inputFlavor, opt_target, result_serve_num1);
				score_result2 = get_score(require_vm, require_vm_copy_temp, inputFlavor, opt_target, result_serve_num2);

				//�Ƿ��������
				if (score_result1 < score_result2) {  //�����������ţ����滻ԭ����
					score_final = score_result2;
					result_serve_num = result_serve_num2;
					for (int ii = 0; ii < num_vm; ii++) {
						require_vm_copy[ii] = require_vm_copy_temp[ii];
					}
				}
				else {  //���������ϲ������һ�����ʴ���ԭ����
					double derta_s = (score_result1 - score_result2) * 100; //��ֵ�鵽0-100
					double D_saa = exp(-derta_s / Temperature);
					int D_saa_int = (int)(D_saa * 100);
					int rand_saa = rand() % 100;
					if (D_saa_int>rand_saa) { //�滻
						score_final = score_result2;
						result_serve_num = result_serve_num2;
						for (int ii = 0; ii < num_vm; ii++) {
							require_vm_copy[ii] = require_vm_copy_temp[ii];
						}
					}
					else { //���滻
						score_final = score_result1;
						result_serve_num = result_serve_num1;
					}
				}
			}
			Temperature -= Step;
			//��ӡ
			//printf("\n��ǰ�¶ȣ�%f  �������÷֣�%f", Temperature, score_final);
		}

		printf("\n��ǰ�¶ȣ�%f  �������÷֣�%f", Temperature, score_final);
		if (score_final > max_score) {
			max_score = score_final;
			for (int i = 0; i < num_vm; i++) {  //һ���˻�����ֵ
				require_vm_iter[i] = require_vm_copy[i];
			}
		}
		/*------------------------------�˻����--------------------------------------------*/
	}


	for (int i = 0; i < num_vm; i++) {  //���ս����ֵ
		require_vm[i] = require_vm_iter[i];
	}
	result_serve_num = putVM_greedy(require_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);//ԭ����


	return result_serve_num;
}

/*  �ϰ�SAA  */
/*
int putVM_score_SAA_greedy(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save) {
int result_serve_num; //���շ�����������
int max_serve_py = 2000;  //������������

putVM_seq(require_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);//ԭ����

double score_final;  //���յ÷�
int *require_vm_copy = new int[num_vm];
int *require_vm_copy_temp = new int[num_vm];

//printf("\npredict final result : [");
//for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
//printf("]\n");

for (int i = 0; i < num_vm; i++) {
require_vm_copy[i] = require_vm[i];
}

float Temperature_first = 100;       //��ʼ�¶�
int Num_of_inner_first = 30;         //��Ȧѭ����ʼ�Ĵ������¶�Խ�͵�������������
float Step = 1;                    //�¶�Temperature�Ĳ���
float Temperature_end = 5;               //�����¶�
float Temperature = Temperature_first;  //��ǰ�¶�

int *result_save_temp1 = new int[max_serve_py*num_vm]; //ԭ����
int *result_save_temp2 = new int[max_serve_py*num_vm]; //������
int result_serve_num1; //ԭ����
int result_serve_num2; //������
double score_result1;   //ԭ���е÷�
double score_result2;   //�����е÷�
//--------------------��ѭ�������¶�----------------------------
int range_rand;  //������ķ�Χ-range_rand��+range_rand
float range_rand_start = 10; //��ʼ�������Χ
while (Temperature > Temperature_end) {
int Num_of_inner = Num_of_inner_first + Temperature_first - Temperature; //��ѭ������,���¶Ƚ��Ͷ��½�
int range_rand = (int)(range_rand_start * (Temperature / Temperature_first));
if (range_rand <= 1) {
range_rand = 2;
}
//��ѭ����ʼ
for (int i = 0; i < Num_of_inner; i++) {
//���������
int num_change = rand() % num_vm;       //�ı�ڼ��������
int predict_change = rand() % (2 * range_rand) - range_rand;   //-range_rand��+range_rand�������
while (predict_change == 0) {
predict_change = rand() % (2 * range_rand) - range_rand;   //ȡ��0����ȡһ��
}

//�ı�ԭ����
for (int ii = 0; ii < num_vm; ii++) {
require_vm_copy_temp[ii] = require_vm_copy[ii];
}
require_vm_copy_temp[num_change] += predict_change;
if (require_vm_copy_temp[num_change] < 0) {  //�������������С��0
require_vm_copy_temp[num_change] = 0;
}
//��������
for (int zreo_i = 0; zreo_i < max_serve_py*num_vm; zreo_i++) {
result_save_temp1[zreo_i] = 0;
result_save_temp2[zreo_i] = 0;
}

//printf("\n [");
//for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm_copy[i]);
//printf("]\n");

result_serve_num1 = putVM_greedy_without_seq(require_vm_copy, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_temp1);//ԭ����
result_serve_num2 = putVM_greedy_without_seq(require_vm_copy_temp, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_temp2);//������

//��ʼ����
score_result1 = get_score(require_vm, require_vm_copy, inputFlavor, opt_target, result_serve_num1);
score_result2 = get_score(require_vm, require_vm_copy_temp, inputFlavor, opt_target, result_serve_num2);

//�Ƿ��������
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
//��ӡ
printf("\n��ǰ�¶ȣ�%f  �������÷֣�%f", Temperature, score_final);
}

for (int i = 0; i < num_vm; i++) {  //���ս����ֵ
require_vm[i] = require_vm_copy[i];
}

result_serve_num = putVM_greedy(require_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);//ԭ����


return result_serve_num;
}
*/


/*   ���ݷ������������ܺ�����������������Ż����÷���    */

/*
����֪��
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

���������������ܣ�
���� 56  128           2       0.5
����ǰ�CPU�Ż���56/128 = 0.5 ��Щ = 0.5������� ����������     > 0.5������� ���Ż����ֶ���    < 0.5������� �ǳ��ֶ���
����ǰ�MEM�Ż���56/128 = 0.5 ��Щ = 0.5������� ����������     < 0.5������� ���Ż����ֶ���    > 0.5������� �ǳ��ֶ���
// ����ǰ�MEM�Ż���128/56 = 2.0 ��Щ = 2.0������� ����������     > 2.0������� ���Ż����ֶ���    < 2.0������� �ǳ��ֶ���


inputServer.cpu_core_num/inputServer.mem_size�����������Ż�cpu��ϵ����
// inputServer.mem_size/inputServer.cpu_core_num�����������Ż�mem��ϵ����

inputFlavor.cpu_core_num/inputFlavor.mem_size����������Ż�cpu��ϵ����
// inputFlavor.mem_size/inputFlavor.cpu_core_num����������Ż�mem��ϵ����

��������Ż�cpu��ϵ�� = ���������Ż�cpu��ϵ����   ����������          �����Ϊ���ֵ��������أ����ӻ��߼��ٶ�OK
��������Ż�cpu��ϵ�� > ���������Ż�cpu��ϵ����   �����ֶ���          ��Ϊ���ֵ�Ψһ�������أ���Զ��Ծ����ܼ��ٻ�õ�
��������Ż�cpu��ϵ�� < ���������Ż�cpu��ϵ����   ���Ż����ֶ���      ���ã������Ż����ֶ������ӻ��߼��ٶ�OK��������ֶ�������أ���ô��������Ż����ֶ���ͻỺ�ⳬ������


���е�����ۺϿ��ǣ�
��1���ȿ���cpu���Ż���
A�����������Ż�cpu��ϵ�� :(0,0.25] �������ܳ��ֳ��ֵ������ ----------- �����н�ϵ����1��
B�����������Ż�cpu��ϵ�� :(1,+max) ���ض����ֳ��ֵ������ ----------- �����н�ϵ����2��
C�����������Ż�cpu��ϵ�� :(0.25,1] �����ܳ��ֳ��ֵ������(�ٸ�����������ܽ����б�) ----------- �����н�ϵ����3��
note�����������Ż�mem��ϵ��Խ�󣬳��ֵĳ��ֵĿ�����Խ��
��2���ȿ���mem���Ż���
A�����������Ż�mem��ϵ�� :[1,+max) �������ܳ��ֳ��ֵ������ ----------- �����н�ϵ����1��
B�����������Ż�mem��ϵ�� :(0,0.25) ���ض����ֳ��ֵ������ ----------- �����н�ϵ����2��
C�����������Ż�mem��ϵ�� :[0.25,1) �����ܳ��ֳ��ֵ������(�ٸ�����������ܽ����б�) ----------- �����н�ϵ����3��
note�����������Ż�mem��ϵ��ԽС�����ֵĳ��ֵĿ�����Խ��

�����н�ϵ����1������Ϊ�ض������֣�һ������£��������Ѿ��ܸߣ�Ԥ���������¸�����ok��
�����н�ϵ����2������Ϊ�ض�������û���κη������Խ����һ������£��������Ѿ��ܵͣ��ԡ����ֶ�����������Ż�cpu��ϵ������Ԥ�����ݾ����ܼ��ٺõ㡣
***  �ص㿼�Ƕ���  ***
�����н�ϵ����3�������ܳ��ֳ��������������Ҫ�����ܽ�����ⲿ�ֳ�����������������ֶ��󡿺͡��Ż����ֶ��󡿾����ܻ�����ɡ�
��ô����� ���磺Ԥ��������a,b,c,d,e,f,g,h�� ����Ϊ3�ࡾ�������󡿡����ֶ��󡿡��Ż����ֶ���
���绮�����£����������� e��f�� �����ֶ���a��b��h�� ���Ż����ֶ���c��d��g��

�����͵Ĵ�С��ģ������
��������Ĵ�С��ģ�����������ģ��û�ж��Ӱ�졣
�����ֿ�Ĵ�С��ģ������������ģ��ͺ����ء� �ӵĴ�С��������������С�ӣ�
���Ż����ֿ�Ĵ�С��ģ�������ר�ã�����������ȥ�Ӵ����ȣ�
�����ֿ�Ĵ�С��ģ���͡��Ż����ֿ�Ĵ�С��ģ����ô�����Ծ������Ż�������

���������󡿣������ӹ����У���������ִ�������������������OK��
�����ֶ��󡿣������ӹ����У�һ��������ִ����������������ٻ�õ㡣
���Ż����ֶ��󡿣������ӹ����У��Ỻ�ⳬ�ֵĸ����������ܶ��õ㡣


*/

/*
�����������������
����:	*require_vm		���������
num_vm			�������������
resource_pm     ����������
opt_target		�Ż�Ŀ�� 0:CPU 1:MEM
*inputFlavor	��������ܱ�
*result_save	���������ݱ�(���)
���:	�����ķ���������
*/
int putVM_directTraversal(int *require_vm, int num_vm, Server resource_pm, int opt_target, Flavor *inputFlavor, int *result_save)
{
	//��������
	double ratio_pm = (double)(resource_pm.cpu_core_num) / resource_pm.mem_size;
	double *ratio_vm = new double[inputcontrol.flavorMaxnum];

	// �϶���Ҫ���ķ�����������
	int SERVER_NUM_PREDICT = 0;

	//ʹ�÷����ķ���
	int flag_method = 1;  // Ĭ���Ƿ���1
						  // �������ܳ��ֳ��ֵ������  ------  1
						  // ���ض����ֳ��ֵ������    ------  2
						  // �����ܳ��ֳ��ֵ������    ------  3


	int *vm_master = new int[inputcontrol.flavorMaxnum];  // ����������������
	int *vm_good = new int[inputcontrol.flavorMaxnum]; // ����Ż����ֶ��������
	int *vm_bad = new int[inputcontrol.flavorMaxnum]; // ��ų��ֶ��������

	int count_master = 0; // ��������ļ�����
	int count_good = 0;  // �Ż����ֶ���ļ�����
	int count_bad = 0; // ���ֶ���ļ�����

	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
		ratio_vm[i] = (double)inputFlavor[i].cpu_core_num / inputFlavor[i].mem_size;

		printf("%f  %f\n", ratio_vm[i], ratio_pm);
		//��Ϊ3��
		//����ǰ�CPU�Ż���56 / 128 = 0.5 ��Щ = 0.5������� ����������     > 0.5������� ���Ż����ֶ���    < 0.5������� �ǳ��ֶ���
		//����ǰ�MEM�Ż���56 / 128 = 0.5 ��Щ = 0.5������� ����������     < 0.5������� ���Ż����ֶ���    > 0.5������� �ǳ��ֶ���
		if (opt_target == 0) //cpu
		{
			// �����Ż�cpu ratio_vmԽ��Խ�á�
			if (ratio_vm[i] == ratio_pm)  vm_master[count_master++] = i;
			if (ratio_vm[i] > ratio_pm)  vm_good[count_good++] = i;
			if (ratio_vm[i] < ratio_pm)  vm_bad[count_bad++] = i;
		}
		if (opt_target == 1) //mem
		{
			// �����Ż�mem ratio_vmԽСԽ�á�
			if (ratio_vm[i] == ratio_pm)  vm_master[count_master++] = i;
			if (ratio_vm[i] < ratio_pm)  vm_good[count_good++] = i;
			if (ratio_vm[i] > ratio_pm)  vm_bad[count_bad++] = i;
		}

	}

	long *vm_size = new long[inputcontrol.flavorMaxnum]; // ���Ϳ�Ĵ�С
	long *vm_size_temp = new long[inputcontrol.flavorMaxnum]; // ���Ϳ�Ĵ�С
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
	// ������vm_size�������Ż����Լ۱ȣ��Ż����Լ۱�Խ�ߣ�����ǰ����ǰ�Ż���



	//��vm_size��������
	int *vm_size_count = new int[inputcontrol.flavorMaxnum]; // ���Ϳ�Ĵ�С�������ţ�����
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  vm_size_count[i] = i;
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  vm_size_temp[i] = vm_size[i];
	printf("��ԭʼ���У�");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)   printf(" %d ", vm_size[i]);	// ��ӡ������
	printf("\n");

	/*
	// ��ӡ������
	for (int i = 0; i < count_good; i++)
	{
	printf(" %d ", vm_size[i]);
	}

	printf("\n");
	// ��ӡ������
	for (int i = 0; i < count_good; i++)
	{
	printf(" %d ", vm_good[i]);
	}
	printf("\n");
	*/

	// ���򣬴�ķ���ǰ��
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

	printf("���������У�");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  	printf(" %d ", vm_size[i]);
	printf("\n");

	printf("�黹ԭ���У�");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  vm_size[i] = vm_size_temp[i];
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  	printf(" %d ", vm_size[i]);
	printf("\n");

	// ��ӡ������
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


	int *vm_opt_priority = new int[inputcontrol.flavorMaxnum]; // ������ȷ�������Ż���Ŀ�꣬����ţ�����ǰ�棡������Ĭ�������Ż�
	int *vm_put_priority = new int[inputcontrol.flavorMaxnum]; // ���ò��ֵ����ȷ���˳�򣡣���
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  vm_put_priority[i] = i;   //���ò��ֵĳ�ʼ����

																				   // ����һ ������ good -- bad -- master
	for (int i = 0; i < count_good; i++)  vm_opt_priority[i] = vm_good[i];
	for (int i = 0; i < count_bad; i++)  vm_opt_priority[i + count_good] = vm_bad[i];
	for (int i = 0; i < count_master; i++)  vm_opt_priority[i + count_bad + count_good] = vm_master[i];
	print_data(vm_opt_priority, 1);

	// ������������ bad -- good -- master
	/*for (int i = 0; i < count_bad; i++)  vm_opt_priority[i] = vm_bad[i];
	for (int i = 0; i < count_good; i++)  vm_opt_priority[i + count_bad] = vm_good[i];
	for (int i = 0; i < count_master; i++)  vm_opt_priority[i + count_bad + count_good] = vm_master[i];*/

	//������������

	print_data(vm_opt_priority, 1);

	int population_max_num = 330000;	 // require_vm ������н��������������ó�ʱ.ʵ����3000
	int population_num = 1; //ʵ�ʿ��н������
	int *vm_opt_range = new int[inputcontrol.flavorMaxnum]; // �Ż��ķ�Χ����Ӧ�Ż����ȼ���
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  vm_opt_range[i] = 1;  // ��ʼ��
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

	int *vm_opt_bit = new int[inputcontrol.flavorMaxnum];  // �Ż���Χȷ��֮��
	vm_opt_bit[0] = 1;
	for (int i = 1; i < inputcontrol.flavorMaxnum; i++)  vm_opt_bit[i] = vm_opt_bit[i - 1] * vm_opt_range[i - 1];
	print_data(vm_opt_bit, 1);

	// һ����н⵽������н���ݱ�
	int **result_predict_super = new int *[population_num];
	for (int i = 0; i < population_num; i++)    result_predict_super[i] = new int[inputcontrol.flavorMaxnum];

	int AAAA = 0;  //�Ե�ǰֵAAAA�Ľ���������
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


	// ��Զ����Ķ��ַ��÷���������
	int max_serve_py = MAX_SERVER_NUM;  // ���������ĸ���!!!
	int EPISODE_MAX_NUM = 5000;
	int **result_save_super = new int *[EPISODE_MAX_NUM];  //ÿһ��ִ��5000��
	for (int i = 0; i < EPISODE_MAX_NUM; i++)    result_save_super[i] = new int[max_serve_py*inputcontrol.flavorMaxnum];
	
	//���ٵķ��õ����ռ�
	for (int i = 0; i < EPISODE_MAX_NUM; i++)
	{
		for (int j = 0; j < max_serve_py*inputcontrol.flavorMaxnum; j++)
		{
			result_save_super[i][j] = 0;
		}
	}

	// flag_method ����˳�򷽷���ѡ��


	// �������ܳ��ֳ��ֵ������ ������2��(��������Ҫ�Ż���������������ͷ�������������)
	if (count_bad == 0)  flag_method = 2;

	// ���ض����ֳ��ֵ������ ������3��(������������ͷ���������������������ڿ����Ż���)
	if (count_bad != 0 && count_good == 0)  flag_method = 3;

	// if (count_bad != 0 && count_good != 0)  flag_method = 4;
	// for (int i = 0; i < inputcontrol.flavorMaxnum;i++)   printf("\n %f ", ratio_vm[i]);
	// ̰������-˳���ѡ��  ������ɺ�require_vm��inputFlavor ��˳�򶼸ı��ˣ�����vm_put_priority��¼������ô�ı�ģ�����
	// putVM_seq_vmsize(require_vm, resource_pm, opt_target, inputFlavor,  vm_size, ratio_vm, ratio_vm_diff,vm_put_priority, flag_method);

	// ����ratio��������򣡣���
	putVM_seq_ratio_guided(require_vm, resource_pm, opt_target, inputFlavor, ratio_vm, vm_put_priority, flag_method);


	int *result_serve_num = new int[EPISODE_MAX_NUM];  // ���淵�صķ�����������
	double *score_result = new double[EPISODE_MAX_NUM]; // ����÷����
	Flavor *inputFlavor_data = new Flavor[inputcontrol.flavorMaxnum];
	int return_serve_num; // ���շ��صķ��������� 
	double FINAL_score_result = 0;  //��߷���

    // ���ŵ�
	for (int AAA = 0; AAA < population_num / EPISODE_MAX_NUM; AAA++)
	{
		for (int i = 0; i < EPISODE_MAX_NUM; i++)
		{

			//����������Ҫ���ķ�����������
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

			//����
			//result_serve_num[i] = putVM_correct_vmsize(result_predict_super[i + EPISODE_MAX_NUM*AAA], inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i], result_serve_num[i]);
			//result_serve_num[i] = putVM_correct(result_predict_super[i], inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save_super[i], result_serve_num[i]);

			//print_data(require_vm, 1);
			//print_data(result_predict_super[i], 1);
			//�÷�
			score_result[i] = get_score(require_vm, result_predict_super[i + EPISODE_MAX_NUM*AAA], inputFlavor, opt_target, result_serve_num[i]);

		}
		
		//��ɵ���֮�󣬿�����û�кõķ���
		for (int i = 0; i < EPISODE_MAX_NUM; i++)
		{
			if (score_result[i] > FINAL_score_result)
			{
				FINAL_score_result = score_result[i];
				return_serve_num = result_serve_num[i]; //���صķ�����������													  
				for (int m = 0; m < inputcontrol.flavorMaxnum; m++)    require_vm[m] = result_predict_super[i + EPISODE_MAX_NUM*AAA][m];// ���и��ƣ�����
				for (int m = 0; m < return_serve_num * inputcontrol.flavorMaxnum; m++)   result_save[m] = result_save_super[i][m];
			}
		}

		// �������֮�󽫽����գ�����
		for (int m = 0; m < EPISODE_MAX_NUM; m++)
		{
			for (int n = 0; n < max_serve_py*inputcontrol.flavorMaxnum; n++)
			{
				result_save_super[m][n] = 0;  //�������
			}
			score_result[m] = 0.0; //�÷ֽ�����
			result_serve_num[m] = 0;  //����������
		}

		//end
	}

	// ��ӡģ��Ԥ����
	printf("[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", require_vm[i]);
	printf("]\n");
	printf("FINAL case score��%f\n", FINAL_score_result);

	return return_serve_num;

}



/*
���ֹ�ʽ
*/
double get_score(int *result_predict_real, int *result_predict_temp, Flavor *inputFlavor_temp, int cpuormem, int serve_num)
{
	// ���
	double sum_result_predict_real = 0;  // ��ĸ�ұ�
	double sum_result_predict_temp = 0;  // ��ĸ���
	double sum_result_predict_diff = 0;  // �ϱ߷���

										 //�ұ�
	double sum_source_fenzi_num = 0; //�ұ߷��ӣ�����
	double sum_source_fenmu_num = 0; //�ұ߷�ĸ������

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
�����������
result_file_local:������������ַ
vm_predict �����Ԥ����
vm_require_num �������������
result_py_save�����������洢����
py_require_num ��������������
inputFlavor �����ļ�������������ܽṹ��:����,cpu,mem
vm_class_num �������������
*/
void write_output_to_result(char *result_file_local, int *vm_predict, int vm_require_num, int *result_py_save, int py_require_num, Flavor *inputFlavor, int vm_class_num) {
	int result_point = 0;
	/*��������*/
	result_point = input_a_int_num(result_file_local, result_point, vm_require_num); //��һ�����������
	result_file_local[result_point++] = '\n';   //��һ�лس�
	for (int i = 0; i < vm_class_num; i++) {
		result_file_local[result_point++] = 'f';
		result_file_local[result_point++] = 'l';
		result_file_local[result_point++] = 'a';
		result_file_local[result_point++] = 'v';
		result_file_local[result_point++] = 'o';
		result_file_local[result_point++] = 'r';
		result_point = input_a_int_num(result_file_local, result_point, inputFlavor[i].flavor_name); //��������
		result_file_local[result_point++] = ' ';
		result_point = input_a_int_num(result_file_local, result_point, vm_predict[i]); //��������
		result_file_local[result_point++] = '\n';
	}
	/*���������*/
	result_file_local[result_point++] = '\n'; //����
	result_point = input_a_int_num(result_file_local, result_point, py_require_num); //��һ������������
	result_file_local[result_point++] = '\n';   //��һ�лس�
	for (int i = 0; i < py_require_num; i++) {
		result_point = input_a_int_num(result_file_local, result_point, (i + 1)); //���������
		for (int j = 0; j < vm_class_num; j++) {
			if (result_py_save[i*vm_class_num + j] != 0) { //��i���������е�j����������䲻Ϊ0,�����
				result_file_local[result_point++] = ' ';
				result_file_local[result_point++] = 'f';
				result_file_local[result_point++] = 'l';
				result_file_local[result_point++] = 'a';
				result_file_local[result_point++] = 'v';
				result_file_local[result_point++] = 'o';
				result_file_local[result_point++] = 'r';
				result_point = input_a_int_num(result_file_local, result_point, inputFlavor[j].flavor_name); //��������
				result_file_local[result_point++] = ' ';
				result_point = input_a_int_num(result_file_local, result_point, result_py_save[i*vm_class_num + j]); //��������
			}
		}
		//result_file_local[result_point++] = '\n';
		if (i < py_require_num - 1) { //�������һ�в��û���
			result_file_local[result_point++] = '\n';
		}
	}
}



// �������һ���������������ˣ�����
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


//��Ҫ��ɵĹ��������
void predict_server(char * info[MAX_INFO_NUM], char * data[MAX_DATA_NUM], int data_num, char * filename)
{
	// ��Ҫ���������

	char result_file[20000] = "0";
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	printf("\n**************************************************************************\n\n");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/************   ģ�����벿�ֿ�ʼ  ************/
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// if(data_num < 5000) CYC_SAMPLE = 7;


	/****************  ����˾�  *******************/

	int is_valid_div = 0; // �Ƿ�ȥ����Ч����������Ч�ָ 0--��ȥ��   1--ȥ��

	int is_noise_oneday = 1; // �Ƿ���ȥ����  0--��ȥ��   1--ȥ��

	int is_noise_happyday = 1; // �Ƿ���ȥ����  0--��ȥ��   1--ȥ��
							   //���ײ�ֲ���
	double diff_parse[4] = { 0, 1,1,-1.80 };
	/*************************************************/
	//�·ݵĳ�ʼ��
	month_init();
	// for (int i = 0; i< 13; i++)   printf("%d %d %d \n", month[i].month_name, month[i].month_day_num, month[i].month_day_total_num);

	//ѵ���ļ�����Ч�ָ���Ϣ
	int valid_div_data[200] = { 0 };
	////////////////////////////  �����ļ��Ĵ��� /////////////////////////////////////////////
	traincontrol.trainfileLinenum = data_num;
	int info_num = 0;
	for (int i = 0; i < MAX_INFO_NUM; i++) {
		if (info[i][4] == '-') {
			info_num = i + 2;
			break;
		}
	}
	inputcontrol.inputfileLinenum = info_num;

	read_flavor_class_num(info); //��ȡ�������������

	Flavor *inputFlavor = new Flavor[inputcontrol.flavorMaxnum];
	do_input_file(info, inputFlavor);  //��ȡ�����ļ�����
									   // CYC_SAMPLE = inputcontrol.intervalTime + 1;


									   /************************************  �Է���������������ķ�������  *************************************/





									   /**********************************  END  ******************************************************************/




									   ////////////////////////////  ѵ���ļ��Ĵ��� //////////////////////////

	read_time_diff(data, data_num); // ��ȡѵ���ļ�����


	if (is_valid_div)
	{
		traincontrol.sample_num = do_train_file_valid_div(data, valid_div_data);  // ���ȶ�ѵ���ļ�������Ч�ָ��ȥ�������ڵ���������
		printf("traincontrol.sample_num: %d\n", traincontrol.sample_num);
		for (int i = 0; i < traincontrol.sample_num; i++)
		{
			printf("valid_div_data[%d]: %d\n", i, valid_div_data[i]);
		}
	}
	else
	{
		traincontrol.sample_num = (traincontrol.endTime - traincontrol.startTime + 1) / CYC_SAMPLE;  //�����ĸ���������
		traincontrol.sample_num_noise = (traincontrol.endTime - traincontrol.startTime + 1) / CYC_SAMPLE_NOISE;  //�����ĸ���������
																												 // printf("traincontrol.sample_num: %d\n", traincontrol.sample_num);
		for (int i = 0; i < traincontrol.sample_num_noise; i++)
		{
			valid_div_data[i] = (traincontrol.endTime - i * CYC_SAMPLE_NOISE);
			printf("valid_div_data[%d]: %d\n", i, valid_div_data[i]);
		}
	}

	// ��̬������������
	int *trainfileFlavordata_temp = new int[inputcontrol.flavorMaxnum * traincontrol.sample_num_noise];

	int *trainfileFlavordata = new int[inputcontrol.flavorMaxnum * traincontrol.sample_num];
	int *trainfileFlavordata_diff = new int[inputcontrol.flavorMaxnum * (traincontrol.sample_num - 1)];
	int *trainfileFlavordata_diff_diff = new int[inputcontrol.flavorMaxnum * (traincontrol.sample_num - 2)];
	int *trainfileFlavordata_diff_diff_DIFF = new int[inputcontrol.flavorMaxnum * (traincontrol.sample_num - 3)];
	//���ݳ�ʼ��
	for (int i = 0; i < inputcontrol.flavorMaxnum * traincontrol.sample_num; i++) {
		trainfileFlavordata[i] = 0;
	}
	for (int i = 0; i < inputcontrol.flavorMaxnum * traincontrol.sample_num_noise; i++) {
		trainfileFlavordata_temp[i] = 0;
	}

	do_train_file(data, inputFlavor, trainfileFlavordata_temp, valid_div_data);  //ͳ��ѵ���ļ�


	print_data(trainfileFlavordata_temp, traincontrol.sample_num_noise); // ��ӡ�����ȡ������
																		 // �ڼ���ȥ����������
	if (is_noise_happyday)
	{
		do_noise_happyday(trainfileFlavordata_temp, valid_div_data, traincontrol.sample_num_noise);

	}

	print_data(trainfileFlavordata_temp, traincontrol.sample_num_noise); // ��ӡ�����ȡ������

	if (is_noise_oneday)
	{
		do_noise_midfilter(trainfileFlavordata_temp, inputFlavor);
		// print_data(trainfileFlavordata_temp, traincontrol.sample_num_noise); // ��ӡ�����ȡ������
	}

	// ȥ��֮�󣡽����ݻ�ԭΪ���ڵ�����
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
	/************   ģ��ȥ�벿�ֿ�ʼ  ************/
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	print_data(trainfileFlavordata, traincontrol.sample_num);

	print_data(trainfileFlavordata_diff, traincontrol.sample_num - 1);

	print_data(trainfileFlavordata_diff_diff, traincontrol.sample_num - 2);

	// print_data(trainfileFlavordata);
	// do_noise_avgfilter(trainfileFlavordata);


	// print_data(trainfileFlavordata);

	// printf("do_train_file function has been passed.\n");


	/////////////////////////////��ʾͳ�ƽ��
	/*for (int i = 0; i < traincontrol.sample_num; i++) {
	cout << "\n";
	for (int j = 0; j < inputcontrol.flavorMaxnum; j++) {
	cout << trainfileFlavordata[i*inputcontrol.flavorMaxnum + j] << " ";
	}
	}
	cout << "\n";*/
	////////////////////////////////// ȥ�ﲿ�ֽ��� //////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/************   ģ��Ԥ�ⲿ�ֿ�ʼ  ************/
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int *result_predict = new int[inputcontrol.flavorMaxnum];
	int preidct_sample_num = inputcontrol.endTime - inputcontrol.startTime + 1;

	/////////////////////////////// Wmean Ԥ�ⲿ�ֿ�ʼ ////////////////////////////////////////////////////////////////////////////////

	//result_predict = predict_run_Wmean(trainfileFlavordata, inputcontrol.flavorMaxnum, traincontrol.sample_num, preidct_sample_num);

	/////////////////////////////// Wmean Ԥ�ⲿ�ֿ�ʼ ////////////////////////////////////////////////////////////////////////////////


	/////////////////////////////// ����Ԥ�ⲿ�ֿ�ʼ ////////////////////////////////////////////////////////////////////////////////

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


	// ��һ�� ������
	result_predict_double = predict_run_liner(trainfileFlavordata, inputcontrol.flavorMaxnum, traincontrol.sample_num, preidct_sample_num);
	// ��ӡԤ����
	printf("predict result : \n[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %f ", result_predict_double[i]);
	printf("]\n");

	// �ڶ��� ������
	result_predict_diff_double = predict_run_liner(trainfileFlavordata_diff, inputcontrol.flavorMaxnum, traincontrol.sample_num - 1, preidct_sample_num);
	// ��ӡԤ����
	printf("predict diff result : \n[");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %f ", result_predict_diff_double[i]);
	printf("]\n");

	// ������ ������
	result_predict_diff_diff_double = predict_run_liner(trainfileFlavordata_diff_diff, inputcontrol.flavorMaxnum, traincontrol.sample_num - 2, preidct_sample_num);
	// ��ӡԤ����

	// ��4��
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


	//���һ������������!!!!!
	result_ori_double = predict_get_ori_data(trainfileFlavordata, inputcontrol.flavorMaxnum, traincontrol.sample_num, preidct_sample_num);
	printf("the last time data : [");
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %f ", result_ori_double[i]);
	printf("]\n");



	// ����Ԥ����
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++)
	{
		// +1ԭ�� �����ܶ����һ�������������ͻ������󣡣���
		result_predict[i] = (int)round_my(result_ori_double[i] * diff_parse[0] + result_predict_double[i] * diff_parse[1] + result_predict_diff_double[i] * diff_parse[2] + result_predict_diff_diff_double[i] * diff_parse[3] +
			result_predict_diff_diff_DIFF_double[i] * diff_parse1 + 0.2);


		if (result_predict[i]<0) {
			result_predict[i] = 0;
		}

	}


	//////////////////////////////// ����Ԥ�ⲿ�ֽ��� ///////////////////////////////////////////////////////


	// ��ӡģ��Ԥ����
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

	//////////////////////////////// ģ��Ԥ�ⲿ�ֽ��� /////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/************   ģ�ͷ��ò��ֿ�ʼ  ************/
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//��̬�Ĵ������������������������

	int max_serve_py = MAX_SERVER_NUM;
	int *result_save = new int[max_serve_py*inputcontrol.flavorMaxnum];
	for (int i = 0; i < max_serve_py*inputcontrol.flavorMaxnum; i++) {
		result_save[i] = 0;
	}
	int num_of_Serve_py;//������������������
	int num_of_Serve_vm = 0;  //Ԥ����������������


							  /***********************  �˻��㷨(Simulate Anneal Arithmetic)+ ̰�ķ��� + �����÷� ����   *************************/
							  /*if (inputcontrol.flavorMaxnum > 1)
							  {
							  num_of_Serve_py = putVM_score_SAA_greedy(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }
							  else
							  {
							  num_of_Serve_py = putVM_greedy(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }

							  // �Ƿ��������С������
							  num_of_Serve_py = putVM_correct(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save, num_of_Serve_py);
							  */
							  // ��ӡģ��Ԥ����
							  /*printf("\npredict final result : [");
							  for (int i = 0; i < inputcontrol.flavorMaxnum; i++)  printf(" %d ", result_predict[i]);
							  printf("]\n");*/
							  /********************************  END  ****************************************************/




							  /***********************  ��Ⱥ���� + ̰�� + ���� + �����÷� ����   *************************/
							  /*if (inputcontrol.flavorMaxnum > 1)
							  {
							  num_of_Serve_py = putVM_score_compet_greedy(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }
							  else
							  {
							  num_of_Serve_py = putVM_greedy(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }*/
							  /********************************  END  ****************************************************/



							  /*****************************  ̰�� + ���� ����   *****************************************/
							  /*num_of_Serve_py = putVM_greedy(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  num_of_Serve_py = putVM_correct(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save, num_of_Serve_py);*/
							  /********************************  END  ****************************************************/


							  /***********************  ��Ⱥ���� + ���� + ���� + �����÷� ����   *************************/
							  /*if (inputcontrol.flavorMaxnum > 1)
							  {
							  num_of_Serve_py = putVM_score_compet_dynamicpro(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }
							  else
							  {
							  num_of_Serve_py = putVM_dynamicpro(result_predict, num_of_Serve_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  }*/
							  /********************************  END  ****************************************************/



							  /*****************************  ���� + ���� ����   *****************************************/
							  /*num_of_Serve_py = putVM_dynamicpro(result_predict, num_of_Serve_vm, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  num_of_Serve_py = putVM_correct(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save, num_of_Serve_py); */
							  /********************************  END  ****************************************************/



							  /***********************  �Ŵ� + ���飨̰�ģ� + ���� + �����÷� ����   *************************/

							  // num_of_Serve_py = gene_dynamicpro(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
							  // num_of_Serve_py = gene_greedy(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);

							  /********************************  END  ****************************************************/


							  /***********************   ��Ⱥ���� + ���� ����   *************************/
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



							  /***********************   ��Ⱥ���� + ̰�� ����   *************************/
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



							  /***********************  ������� + ���� + �����÷� ����   *************************/
	num_of_Serve_py = putVM_directTraversal(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save);
	//num_of_Serve_py = putVM_correct(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save, num_of_Serve_py);

	/********************************  END  ****************************************************/




	/*************************  END  **************************************************************************************************/

	num_of_Serve_vm = 0;  //�������ٴ�ͳ��Ԥ����������������
	for (int i = 0; i < inputcontrol.flavorMaxnum; i++) {
		num_of_Serve_vm += result_predict[i];
	}
	print_resource(result_predict, inputcontrol.flavorMaxnum, inputServer, inputcontrol.cpuOrmem, inputFlavor, result_save, num_of_Serve_py);
	write_output_to_result(result_file, result_predict, num_of_Serve_vm, result_save, num_of_Serve_py, inputFlavor, inputcontrol.flavorMaxnum);
	printf("\n**************************************************************************\n");

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// ֱ�ӵ�������ļ��ķ��������ָ���ļ���(ps��ע���ʽ����ȷ��,����н�,��һ��ֻ��һ������;�ڶ���Ϊ��;�����п�ʼ���Ǿ��������,����֮����һ���ո�ָ���)
	write_result(result_file, filename);
}




