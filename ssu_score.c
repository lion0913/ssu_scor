#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ssu_score.h"
#include "blank.h"

extern struct ssu_scoreTable score_table[QNUM];//scoretable을 정렬
extern char id_table[SNUM][10];

struct ssu_scoreTable score_table[QNUM];
char id_table[SNUM][10];

char stuDir[BUFLEN];
char ansDir[BUFLEN];
char errorDir[BUFLEN];
char threadFiles[ARGNUM][FILELEN];
char IDs[ARGNUM][FILELEN];

int eOption = false;
int tOption = false;
int mOption = false;
int iOption = false;


void ssu_score(int argc, char *argv[])//인자값 가져옴
{
	char saved_path[BUFLEN];
	int i;

	for(i = 0; i < argc; i++){
		if(!strcmp(argv[i], "-h")){//-h :  사용법 출력함수
			print_usage();
			return;
		}
	}

	memset(saved_path, 0, BUFLEN);//배열의 값을 0으로 초기화
	if(argc >= 3 && strcmp(argv[1], "-i") != 0){//STD_DIR ,ANS_DIR이 주어진 상태에서 -i옵션이 들어간 경우
		strcpy(stuDir, argv[1]);//인자대로 디렉토리에 copy함
		strcpy(ansDir, argv[2]);
	}

	if(!check_option(argc, argv))
		exit(1);

	if(!eOption && !tOption && !mOption && iOption){//다른 옵션 없이 -i옵션만 입력된경우
	do_iOption(IDs);
		return;
	}

	getcwd(saved_path, BUFLEN);//현재 작업중인 디렉토리 경로 saved_path에 얻어옴

	if(chdir(stuDir) < 0){// 작업디렉토리를 stuDir로 변경
		fprintf(stderr, "%s doesn't exist\n", stuDir);
		return;
	}
	getcwd(stuDir, BUFLEN);//작업중인 디렉토리경로  stuDir에 복사

	chdir(saved_path);//현재작업디렉토리 변경
	if(chdir(ansDir) < 0){//디렉토리변경 에러시
		fprintf(stderr, "%s doesn't exist\n", ansDir);
		return;
	}
	getcwd(ansDir, BUFLEN);

	chdir(saved_path);

	set_scoreTable(ansDir);//점수테이블 파일생성함수
	set_idTable(stuDir);//학생테이블(학번 디렉터리 저장)
	if(mOption){
		do_mOption();//문제를 수정하는 함수를 수행
		set_scoreTable(ansDir);//새로 바뀐 score_table.csv파일의 값에 맞는 score.csv파일 내용 수정
	}
	printf("grading student's test papers..\n");
	score_students();//학생들의 답안을 채점해서 score.csv함수에 저장하는 함수

	if(iOption)
		do_iOption(IDs);

	return;
}

int check_option(int argc, char *argv[])//옵션여부를 체크하고 그에 필요한 인자를 받아들이는 함수
{
	int i, j;
	int c;

	while((c = getopt(argc, argv, "e:thmi")) != -1)
	{
		switch(c){
			case 'e'://디렉토리파일에 qname_error.txt파일의 형태로 저장
				eOption = true;
				strcpy(errorDir, optarg);//인자를 받아서 errorDir로 복사함

				if(access(errorDir, F_OK) < 0)//errorDir파일을 오픈함
					mkdir(errorDir, 0755);
				else{//이미 있는경우 지우고 재생성
					rmdirs(errorDir);
					mkdir(errorDir, 0755);
				}
				break;
			case 't'://-t옵션인 경우(-lpthread옵션)
				tOption = true;
				i = optind;//-t의 뒤에 들어가는 인자의 개수 저장
				j = 0;//-lpthread로 컴파일할 문제들을 저장하기 위한 변수

				while(i < argc && argv[i][0] != '-'){

					if(j >= ARGNUM)//가변인자가 5개를 넘은 경우
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else
						strcpy(threadFiles[j], argv[i]);//-lpthread로 컴파일할 문제들의 리스트를 배열에 저장
					i++; 
					j++;
				}
				break;
			case 'm'://문제의 점수를 수정하는 함수
				mOption = true;
				break;
			case 'i'://-i옵션인 경우(각 학생들이 틀린 문제번호를 출력하는 옵션)
				iOption = true;
				i = optind;//-i의 뒤에 들어가는 인자의 개수 저장
				j = 0;

				while(i < argc && argv[i][0] != '-'){//인자의 개수 하나씩 받아들여서 IDs배열에 저장

					if(j >= ARGNUM)//가변인자가 5개를 넘은 경우
						printf("Maximum Number of Argument Exceeded.  :: %s\n", argv[i]);
					else
						strcpy(IDs[j], argv[i]);//입력한 학번을 받아서 저장
					i++; 
					j++;
				}
				break;
			case '?'://옵션을 알 수 없을때
				printf("Unkown option %c\n", optopt);
				return false;
		}
	}

	return true;
}
void do_mOption(){
	FILE *fp;
	int line=0;//수정할 줄 수
	double score;//할당할 점수
	char input[BUFLEN];//수정하고자하는 문제제목
	char qname[BUFLEN];//score_table.csv에서 찾을 문제의 제목
	char *qnum;
	
	//int cnt=0;
	//char *p, *saved;
	while(1){

		if((fp = fopen("score_table.csv", "r")) == NULL){//score_table.csv파일 오픈
			fprintf(stderr, "file open error for score.csv\n");
			return;
		}//오픈에러처리
		memset(input,0,strlen(input));
		fseek(fp,0,SEEK_SET);//문제 수정을 반복적으로 하기 때문에 오프셋을 처음으로 이동해주어야함
		printf("Input question's number to modify >> ");
		scanf("%s",input);
		getchar();
		line=0;//라인을 다시 초기화해주어야지 rewrite함수를 쓸 때 수정할 줄의 수를 제대로 읽어들일 수 있음.
		while((fscanf(fp,"%[^,],%lf\n",qname,&score))!=EOF){//QNAME, score을 차례로 채움
			qnum=strtok(qname,".");//문제 번호만 가져옴 ex>1-1,11-1
			if(strcmp(qnum,input)==0){//입력한 문제번호와 읽어온 문제번호가 같을때(=찾았을 때)
				printf("Current score: %.1lf\n",score);
				printf("New score : ");
				scanf("%lf",&score);//새롭게 할당할 점수를 입력받음
				rewrite(line,score);
				//printf("%.1lf점수에 대한 재작성이 완료되었습니다.\n",score);
			}
			line++;
			memset(qname,0,strlen(qname));
		}
		if(strcmp(input,"no")==0) break;
	
	//	if(strcmp(q,
     // 	fscanf(fp, "%s\n", f_line);//첫번째 줄 (문제번호 모음)읽어들이기

	}
	fclose(fp);
}

void rewrite(int line,double score){
	FILE *temp;//임시 저장할 파일(나중에 옮겨적을꺼다)
	FILE *old;//원래 score_table.csv파일
	char old_pth[BUFLEN]={0};//파일의 경로를 저장하기 위한 배열
	char temp_pth[BUFLEN]={0};
	char filedelete[BUFLEN]={0};
	char filecpy[BUFLEN]={0};
	double old_score;
	char qname[BUFLEN]={0};
	int cnt=0;
	char tmp[BUFLEN]={0};
	if((temp=fopen("temp.csv","w"))==NULL){
		fprintf(stderr,"file open error for %s\n","temp.csv");
	}
	if((old=fopen("score_table.csv","r"))==NULL){
		fprintf(stderr,"file open error for %s\n","score_table.csv");
	}
	cnt=0;
	memset(tmp,0,strlen(tmp));
//printf("%d\n",line);
	while(fscanf(old,"%[^,],%lf\n",qname,&old_score)!=EOF){//원래파일의 문제번호와 점수 차례로 받아옴
		
		//memset(tmp,0,strlen(tmp));
		if(cnt==line){//수정하고자 하는 라인에 도달한 경우
			sprintf(tmp,"%s,%.2f\n",qname,score);//임시저장공간에 문제번호와 새로 입력한 점수 형식맞춰서 저장
			fwrite(tmp,strlen(tmp),1,temp);
		//	printf("%s문제 번호를 %.1lf점수로 할당하였습니다\n",qname,score);

//printf("%s\n",tmp);
		}
		else{//기존의 old파일의 동일라인 그대로 복사
			sprintf(tmp,"%s,%.2f\n",qname,old_score);
			fwrite(tmp,strlen(tmp),1,temp);
//printf("%s\n",tmp);
		}
		cnt++;
		memset(tmp,0,strlen(tmp));
	}
	fclose(old);
	fclose(temp);

	sprintf(old_pth,"%s","score_table.csv");
	sprintf(temp_pth,"%s","temp.csv");
	sprintf(filedelete,"rm %s",old_pth);
	system(filedelete);
	sprintf(filecpy,"cp %s %s",temp_pth,old_pth);
	system(filecpy);
	sprintf(filedelete,"rm %s",temp_pth);
	system(filedelete);
}

void do_iOption(char (*ids)[FILELEN])//i옵션 수행함수
{
	FILE *fp;
	char tmp[BUFLEN];
	char f_line[BUFLEN];
	int i = 0;
	int cnt=0;
	char *p, *saved;
	if((fp = fopen("score.csv", "r")) == NULL){//score.csv파일 오픈
		fprintf(stderr, "file open error for score.csv\n");
		return;
	}//오픈에러처리

	fscanf(fp, "%s\n", f_line);//첫번째 줄 (문제번호 모음)읽어들이기

	while(fscanf(fp, "%s\n", tmp) != EOF)//줄단위로 읽어들임(파일이 끝날 때까지)
	{
		p = strtok(tmp, ",");


		if(!is_exist(ids, tmp))
			continue;

		//char *remain=strtok(NULL,"\n");

		printf("%s's wrong answer :\n ", tmp);
	//	while((p=strtok(NULL,","))!=NULL){
	//	while((p=strtok(NULL,","))!=NULL){
		while((p=strtok(NULL,","))!=NULL){
			cnt++;
			if(strcmp(p,"0")==0){
				saved=get_qnumber(f_line,get_index(f_line,cnt));
				printf("%s ",saved);
	//	find_wrong_answer(remain,f_line);
			}	
		}
		printf("\n");
		cnt=0;
	}
	memset(tmp,0,strlen(tmp));
	//printf("\n");
	fclose(fp);
}

int get_index(char * f_line,int cnt){
	//char qname[BUFLEN]={0};//문제번호를 받을 버퍼
	//char * token;
	int k,i;
	for(int idx=0;idx<strlen(f_line);idx++){
		if(f_line[idx]==',') k++;
		if(k==cnt){
			i=idx+1;
			break;
		}

	}
	//printf("%d",k+1);
	return i;
/*
	char *p;
	token=strtok(tmp,",");
	int count=0;
	while((token = strtok(NULL, ",")) != NULL){
	//	count++;
	//	printf("%s\n",token);
		if(strcmp(token,"0")==0){
		//	printf("%d\n",count);
		      p= get_qnumber(count,qname);
			printf("%s ",p);
		}
	       count++;	
	}*/
}

char* get_qnumber(char * f_line,int idx){
	char *temp=(char *)calloc(BUFLEN,sizeof(char));
	int k=0;
	while(f_line[idx]!=',')
		temp[k++]=f_line[idx++];
	return temp;
	/*int cnt=0;
	int index;
	for(int k=0;k<BUFLEN;k++){
		if(qname[i]==',')
			cnt++;
		if(cnt==count){
			index=k+1;
			break;
		}
	}*/
	

}
int is_exist(char (*src)[FILELEN], char *target)
{
	int i = 0;

	while(1)
	{
		if(i >= ARGNUM)
			return false;
		else if(!strcmp(src[i], ""))
			return false;
		else if(!strcmp(src[i++], target))
			return true;
	}
	return false;
}

void set_scoreTable(char *ansDir)//점수테이블 파일 설정함수
{
	char filename[FILELEN];

	
	sprintf(filename, "%s", "score_table.csv");//파일에 답안디렉터리 넣기

	if(access(filename, F_OK) == 0)//파일의 접근권한 확인
		read_scoreTable(filename);//점수테이블 읽어들이는 함수
	else{//score_table.csv 파일이 없을 경우 
		make_scoreTable(ansDir);//scoreTable파일 생성
		write_scoreTable(filename);//score_table.csv에 값 넣는 함수
	}
}

void read_scoreTable(char *path)//점수테이블 읽어들이는 함수
{
	FILE *fp;
	char qname[FILELEN];
	char score[BUFLEN];
	int idx = 0;

	if((fp = fopen(path, "r")) == NULL){//파일오픈+ 오픈에러처리
		fprintf(stderr, "file open error for %s\n", path);
		return ;
	}

	while(fscanf(fp, "%[^,],%s\n", qname, score) != EOF){//답안파일에 있는 문제번호, 점수 읽어서 score table에 넣기
		strcpy(score_table[idx].qname, qname);
		score_table[idx++].score = atof(score);//인덱스증가(double형으로)
	}

	fclose(fp);
}

void make_scoreTable(char *ansDir)//점수테이블 생성  함수
{
	int type, num;
	double score, bscore, pscore;//bscore : 빈칸 점수, pscore : 프로그램 점수
	struct dirent *dirp, *c_dirp;
	DIR *dp, *c_dp;//c_dp이거 지금은 필요없음
	char buf[BUFLEN];//ㅅ래뫈든거
	char tmp[BUFLEN];
	int idx = 0;
	int i;
	int rd_size;//새로
	struct dirent* filename;
	num = get_create_type();//점수생성테이블의  유형받아옴

	if(num == 1)//배점 입력(빈 칸, 프로그램 문제)
	{
		printf("Input value of blank question : ");
		scanf("%lf", &bscore);
		printf("Input value of program question : ");
		scanf("%lf", &pscore);
	}

	if((dp = opendir(ansDir)) == NULL){//ansDir열기
		fprintf(stderr, "open dir error for %s\n", ansDir);
		return;
	}//파일오픈에러처리

	while((dirp = readdir(dp)) != NULL)//dirp로 정답 디렉토리 읽음
	{
		//printf("directory name:%s\n\n",dirp->d_name);//확인용 다 지워야됨
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))//.과 ..을 무시함
			continue;
		if(type=get_file_type(dirp->d_name)<0){//파일 타입 알아내고 
			continue;
		}
		strcpy(score_table[idx++].qname,dirp->d_name);//디렉토리에 있는 파일이름을 점수테이블에 입력+index++
		//sprintf(tmp, "%s/%s", ansDir, dirp->d_name);//임시저장공간 tmp에' 디렉토리이름/파일이름' 저장 
	}

//		closedir(c_dp);

	/*if((c_dp = opendir(tmp)) == NULL){//파일 열기
			fprintf(stderr, "open dir error for %s\n", tmp);
			return;
		}

		while((c_dirp = read(c_dp,buf)) != NULL)//연 정답 파일 읽기,원랜 readdir이었음.
		{
			if(!strcmp(c_dirp->d_name, ".") || !strcmp(c_dirp->d_name, ".."))//..과 .을 무시함
				continue;

			if((type = get_file_type(c_dirp)) < 0)//.txt인지 .c파일인지 구분함,c_dirp->d_name
				continue;

			strcpy(score_table[idx++].qname, c_dirp->d_name);//scoretable에 파일 이름 쓰기
		}

		closedir(c_dp);*/
	



	closedir(dp);
	sort_scoreTable(idx);//정답테이블을 정렬함

	for(i = 0; i < idx; i++)
	{
		
		type = get_file_type(score_table[i].qname);//인덱스번호대로 차례로 파일타입 받아와서 저장

		if(num == 1)//문제의 종류별로 한번에 점수를 입력받는 경우
		{
			if(type == TEXTFILE)//빈칸문제, 프로그램 문제의 각 점수 저장
				score = bscore;
			else if(type == CFILE)
				score = pscore;
		}
		else if(num == 2)//문제 선택 후 점수를 지정하는 경우
		{
			printf("Input of %s: ", score_table[i].qname);//문제번호 입력받고 점수 저장
			scanf("%lf", &score);
		}

		score_table[i].score = score;
	}
}

void write_scoreTable(char *filename)//score_table.csv파일에 값 쓰는 함수
{
	int fd;
	char tmp[BUFLEN];
	int i;
	int num = sizeof(score_table) / sizeof(score_table[0]);

	if((fd = creat(filename, 0666)) < 0){
		fprintf(stderr, "creat error for %s\n", filename);
		return;
	}

	for(i = 0; i < num; i++)
	{
		if(score_table[i].score == 0)
			break;
		//printf("%s,%.2f\n",score_table[i].qname, score_table[i].score);//이거 지워야됨
		sprintf(tmp, "%s,%.2f\n", score_table[i].qname, score_table[i].score);//형식에 맞춰서 .csv파일에 출력
		write(fd, tmp, strlen(tmp));
	}

	close(fd);
}


void set_idTable(char *stuDir)//학번테이블 생성 함수
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	char tmp[BUFLEN];
	int num = 0;

	if((dp = opendir(stuDir)) == NULL){//stuDir파일 오픈
		fprintf(stderr, "opendir error for %s\n", stuDir);
		exit(1);
	}//오픈에러처리

	while((dirp = readdir(dp)) != NULL){//디렉토리 파일 하나씩 읽기
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))
			continue;

		sprintf(tmp, "%s/%s", stuDir, dirp->d_name);
		stat(tmp, &statbuf);//tmp경로의 정보 statbuf에 저장 std_dir/20200001 이런식으로 저장되임

		if(S_ISDIR(statbuf.st_mode)){//이 파일이 디렉토리인지확인하고 학번테이블에 넣음

			strcpy(id_table[num++], dirp->d_name);
	//		printf("%s\n\n",dirp->d_name);
		}
		else
			continue;
	}

	sort_idTable(num);
}

void sort_idTable(int size)//
{
	int i, j;
	char tmp[10];

	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 -i; j++){
			if(strcmp(id_table[j], id_table[j+1]) > 0){
				strcpy(tmp, id_table[j]);
				strcpy(id_table[j], id_table[j+1]);
				strcpy(id_table[j+1], tmp);
			}
		}
	}
}

void sort_scoreTable(int size)//scoretable을 정렬
{
	int i, j;
	struct ssu_scoreTable tmp;
	int num1_1, num1_2;
	int num2_1, num2_2;

	for(i = 0; i < size - 1; i++){
		for(j = 0; j < size - 1 - i; j++){

			get_qname_number(score_table[j].qname, &num1_1, &num1_2);//정답 파일에서 문제번호 받아옴.
			get_qname_number(score_table[j+1].qname, &num2_1, &num2_2);


			if((num1_1 > num2_1) || ((num1_1 == num2_1) && (num1_2 > num2_2))){//비교 후 정렬->메모리에까지 저장

				memcpy(&tmp, &score_table[j], sizeof(score_table[0]));
				memcpy(&score_table[j], &score_table[j+1], sizeof(score_table[0]));
				memcpy(&score_table[j+1], &tmp, sizeof(score_table[0]));
			}
		}
	}
}

void get_qname_number(char *qname, int *num1, int *num2)//정답 디렉토리에서 문제번호를 가져오는 함수
{
	char *p;
	char dup[FILELEN];

	strncpy(dup, qname, strlen(qname));//파일이름을 복사
	*num1 = atoi(strtok(dup, "-."));//-.앞의 값을 num1에 저장
	
	p = strtok(NULL, "-.");//뒤의 값을 p에 저장함
	
	if(p == NULL)
		*num2 = 0;
	else
		*num2 = atoi(p);
}

int get_create_type()//점수생성테이블의 유형선택
{
	int num;

	while(1)
	{
		printf("score_table.csv file doesn't exist!\n");
		printf("1. input blank question and program question's score. ex) 0.5 1\n");
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n");
		printf("select type >> ");
		scanf("%d", &num);

		if(num != 1 && num != 2)
			printf("not correct number!\n");
		else
			break;
	}

	return num;
}

void score_students()//score.csv파일을 채우는 함수(학생들의 채점결과, 평균을 입력)
{
	double score = 0;
	int num;
	int fd;
	char tmp[BUFLEN];
	int size = sizeof(id_table) / sizeof(id_table[0]);

	if((fd = creat("score.csv", 0666)) < 0){//score파일 생성
		fprintf(stderr, "creat error for score.csv");
		return;
	}
	write_first_row(fd);//파일의 첫번째줄 작성

	for(num = 0; num < size; num++)//문제수만큼 반복
	{
		//printf("%s\n",id_table[num]);
		if(!strcmp(id_table[num], ""))//학번을 받아들임 (없을 시 종료)
			break;
		sprintf(tmp, "%s,", id_table[num]);
		write(fd, tmp, strlen(tmp)); //score.csv 파일에 학번 입력
		score += score_student(fd, id_table[num]);//학번별 채점결과 출력
	}

	
	printf("Total average : %.2f\n", score / num);

	close(fd);
}

double score_student(int fd, char *id)//학번 별 채점결과 출력해주는 함수(score.csv파일을 채움)
{
	int type;
	double result;
	double score = 0;
	int i;
	char tmp[BUFLEN];
	int size = sizeof(score_table) / sizeof(score_table[0]);

	for(i = 0; i < size ; i++)
	{
		if(score_table[i].score == 0)
			break;

		//sprintf(tmp, "%s/%s", stuDir, id);
		
		sprintf(tmp,"%s/%s/%s",stuDir,id,score_table[i].qname);
	//printf("%s\n",tmp);//출력확인용
	//	sprintf(tmp,"%s/%s/%s",stuDir,id,score_table[i].qname);
	//	printf("%s",tmp);
		//	sprintf(tmp, "%s/%s/%s", stuDir, id, score_table[i].qname);
	//	printf("tmp출력: %s\n",tmp);//확인용
		if(access(tmp, F_OK) < 0)//학생 답안파일 열기
			result = false;
		else
		{
			if((type = get_file_type(score_table[i].qname)) < 0)//파일 타입 읽어오기
				continue;
			
			if(type == TEXTFILE)//텍스트파일이면 score_blank의 결과로
				result = score_blank(id, score_table[i].qname);//
			else if(type == CFILE)//c파일이면 score_program의 결과값을 받아옴
				result = score_program(id, score_table[i].qname);
		}//result에는 두 답안파일을 비교 후 정답유무를 비교한 최종값이 저장되어있음

		if(result == false)//다른 결과가 나왔으므로 점수는 0
			write(fd, "0,", 2);
		else{
			if(result == true){//프로그램 채점 결과가 정답인 경우
				score += score_table[i].score;//점수를 score변수에 저장하고 
				sprintf(tmp, "%.2f,", score_table[i].score);//scoretable에 그 점수를 기록
			}
			else if(result < 0){//감점할 점수가 있다면
				score = score + score_table[i].score + result;//score에 그 값을 더함(전체 점수)
				sprintf(tmp, "%.2f,", score_table[i].score + result);//score_table에도 그 점수를 기록(그 문제에 대한 점수)
			}
			write(fd, tmp, strlen(tmp));//score.csv에 이 값을 기록
		}
	}

	//if(mOption)
		printf("%s is finished.. score : %.2f\n", id, score); 
//	else
//		printf("%s is finished..\n", id);

	sprintf(tmp, "%.2f\n", score);
	write(fd, tmp, strlen(tmp));

	return score;
}

void write_first_row(int fd)//score.csv의 첫번째 줄 작성(1-1.txt…,sum)형식
{
	int i;
	char tmp[BUFLEN];
	
	int size = sizeof(score_table) / sizeof(score_table[0]);//문제 개수를 저장

	write(fd, ",", 1);

	for(i = 0; i < size; i++){
		if(score_table[i].score == 0)
			break;
		
		sprintf(tmp, "%s,", score_table[i].qname);//문제번호 입력
		write(fd, tmp, strlen(tmp));//파일에 작성
	}
	write(fd, "sum\n", 4);
}

char *get_answer(int fd, char *result)//학생이 입력한 문제에 대한 정답을 가져오는 함수(fd:오픈한 파일의 디스크립터,result : 리턴할 정답)
{
	char c;
	int idx = 0;

	memset(result, 0, BUFLEN);
	while(read(fd, &c, 1) > 0)//정답파일을 한 글자씩 읽음
	{
		if(c == ':')//:로 복수정답을 구분
			break;
		
		result[idx++] = c;//:전까지의 값을 result값에 차례로 넣음
	}
	if(result[strlen(result) - 1] == '\n')//만약 마지막 문자에 개행이 들어간다면 제거해준다
		result[strlen(result) - 1] = '\0';

	return result;//정답값 리턴
}

int score_blank(char *id, char *filename)//빈칸 문제(한 문제)에 대한 정답유무를 리턴하는 함수
{
	char tokens[TOKEN_CNT][MINLEN];
	node *std_root = NULL, *ans_root = NULL;
	int idx, start;
	char tmp[BUFLEN];
	char s_answer[BUFLEN], a_answer[BUFLEN];
	char qname[FILELEN];
	int fd_std, fd_ans;
	int result = true;
	int has_semicolon = false;

	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));

	sprintf(tmp, "%s/%s/%s", stuDir, id, filename);//빈칸 파일의 경로 받아옴
	//printf("%s\n",tmp);//출력확인용
	fd_std = open(tmp, O_RDONLY);//파일오픈
	strcpy(s_answer, get_answer(fd_std, s_answer));//정답값을 s_answer에 집어넣음

	if(!strcmp(s_answer, "")){//정답값이 아무것도 없다면 파일 닫고 끝냄
		close(fd_std);
		return false;
	}

	if(!check_brackets(s_answer)){//정답값에 괄호의 개수가 다르면 파일닫고 끝냄.
		close(fd_std);
		return false;
	}

	strcpy(s_answer, ltrim(rtrim(s_answer)));//문자열 오른쪽, 왼쪽 공백 제거

	if(s_answer[strlen(s_answer) - 1] == ';'){//문자열끝에있는 ';' 제거
		has_semicolon = true;
		s_answer[strlen(s_answer) - 1] = '\0';
	}

	if(!make_tokens(s_answer, tokens)){//문자열을 언어요소(token)으로  만들기 
		close(fd_std);
		return false;
	}

	idx = 0;
	std_root = make_tree(std_root, tokens, &idx, 0);

	
	sprintf(tmp, "%s/%s", ansDir, filename);//답지 파일을 오픈
	//sprintf(tmp, "%s/%s/%s", ansDir, qname, filename);
	fd_ans = open(tmp, O_RDONLY);

	while(1)
	{
		ans_root = NULL;
		result = true;

		for(idx = 0; idx < TOKEN_CNT; idx++)
			memset(tokens[idx], 0, sizeof(tokens[idx]));

		strcpy(a_answer, get_answer(fd_ans, a_answer));

		if(!strcmp(a_answer, ""))
			break;

		strcpy(a_answer, ltrim(rtrim(a_answer)));

		if(has_semicolon == false){
			if(a_answer[strlen(a_answer) -1] == ';')
				continue;
		}

		else if(has_semicolon == true)
		{
			if(a_answer[strlen(a_answer) - 1] != ';')
				continue;
			else
				a_answer[strlen(a_answer) - 1] = '\0';
		}

		if(!make_tokens(a_answer, tokens))
			continue;

		idx = 0;
		ans_root = make_tree(ans_root, tokens, &idx, 0);

		compare_tree(std_root, ans_root, &result);

		if(result == true){
			close(fd_std);
			close(fd_ans);

			if(std_root != NULL)
				free_node(std_root);
			if(ans_root != NULL)
				free_node(ans_root);
			return true;

		}
	}
	
	close(fd_std);
	close(fd_ans);

	if(std_root != NULL)
		free_node(std_root);
	if(ans_root != NULL)
		free_node(ans_root);

	return false;
}

double score_program(char *id, char *filename)//프로그램문제를 채점해서 정답유무를 리턴하는 프로그램
{
	double compile;
	int result;

	compile = compile_program(id, filename);//파일을 차례로 컴파일 해줌(compile : warning으로 인한 감점점수/ERROR 유무)

	if(compile == ERROR || compile == false)//만약 컴파일이 안됐거나 에러가 났다면 그대로 리턴
		return false;
	
	result = execute_program(id, filename);//파일의 실행값을 비교해서 같은지 다른지 리턴하는 함수(return=두 답안이 같은지 다른지를 저장한 함수)

	if(!result)//오답이면 false 리턴
		return false;

	if(compile < 0)//감점할 점수가 있다면 그 값을 리턴
		return compile;

	return true;//정답이므로 true를 리턴
}

int is_thread(char *qname)//스레드인지 아닌지 확인하는 함수
{
	int i;
	int size = sizeof(threadFiles) / sizeof(threadFiles[0]);

	for(i = 0; i < size; i++){
		if(!strcmp(threadFiles[i], qname))
			return true;
	}
	return false;
}

double compile_program(char *id, char *filename)//프로그램을 컴파일하는 함수
{
	int fd;
	char tmp_f[BUFLEN], tmp_e[BUFLEN];
	char command[BUFLEN];
	char qname[FILELEN];
	int isthread;
	off_t size;
	double result;

	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));
	
	isthread = is_thread(qname);

	sprintf(tmp_f, "%s/%s", ansDir,  filename);
	
	sprintf(tmp_e, "%s/%s.exe", ansDir, qname);
	sprintf(tmp_e, "%s/%s.exe", ansDir, qname);
	if(tOption && isthread)//-t옵션이나 스레드가 붙은경우 -lpthread옵션을 이용해서 컴파일함 
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);//답안 컴파일
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);

	sprintf(tmp_e, "%s/%s_error.txt", ansDir, qname);//답안 컴파일 시의 에러파일생성
	fd = creat(tmp_e, 0666);

	redirection(command, fd, STDERR);
	size = lseek(fd, 0, SEEK_END);
	close(fd);
	unlink(tmp_e);

	if(size > 0)
		return false;

	sprintf(tmp_f, "%s/%s/%s", stuDir, id, filename);
	sprintf(tmp_e, "%s/%s/%s.stdexe", stuDir, id, qname);

	if(tOption && isthread)//학생들의 답안파일 컴파일
		sprintf(command, "gcc -o %s %s -lpthread", tmp_e, tmp_f);
	else
		sprintf(command, "gcc -o %s %s", tmp_e, tmp_f);

	sprintf(tmp_f, "%s/%s/%s_error.txt", stuDir, id, qname);//에러파일 생성
	fd = creat(tmp_f, 0666);

	redirection(command, fd, STDERR);//리다이렉션 실시
	size = lseek(fd, 0, SEEK_END);
	close(fd);

	if(size > 0){
		if(eOption)//-e옵션을 입력했다면
		{
			sprintf(tmp_e, "%s/%s", errorDir, id);
			if(access(tmp_e, F_OK) < 0)//에러디렉토리 생성
				mkdir(tmp_e, 0755);

			sprintf(tmp_e, "%s/%s/%s_error.txt", errorDir, id, qname);//문제번호 별 에러텍스트 생성
			rename(tmp_f, tmp_e);//에러디렉토리를 채우는 함수로 넘어가기 위해 파일 명 변경

			result = check_error_warning(tmp_e);//에러디렉토리 안 _error.txt 파일 채우기
		}
		else{ 
			result = check_error_warning(tmp_f);//학번디렉토리 속의 _error.txt파일 채우기
			unlink(tmp_f);
		}

		return result;//각 파일을 컴파일했을 때  났던 warning횟수를 리턴함
	}

	unlink(tmp_f);//언링크해줌
	return true;
}

double check_error_warning(char *filename)//error.txt파일 채우기
{
	FILE *fp;
	char tmp[BUFLEN];
	double warning = 0;//warning가 몇 번 떴는지 확인하고 그에 따른 감점을 부여하기 위한 변수

	if((fp = fopen(filename, "r")) == NULL){//파일읽기(권한 : 읽기)
		fprintf(stderr, "fopen error for %s\n", filename);
		return false;
	}

	while(fscanf(fp, "%s", tmp) > 0){//파일 안 값 읽어들이기
		if(!strcmp(tmp, "error:"))//error가 나면 즉시 ERROR 리턴
			return ERROR;
		else if(!strcmp(tmp, "warning:"))//warning 발생시 감점 점수를 더함
			warning += WARNING;
	}

	return warning;//감점값 반환
}

int execute_program(char *id, char *filename)//프로그램을을 실행하는 함수
{
	char std_fname[BUFLEN], ans_fname[BUFLEN];
	char tmp[BUFLEN];
	char qname[FILELEN];
	time_t start, end;
	pid_t pid;
	int fd;

	memset(qname, 0, sizeof(qname));
	memcpy(qname, filename, strlen(filename) - strlen(strrchr(filename, '.')));//qname에 문제번호만 입력되도록 메모리에서 복사함

	sprintf(ans_fname, "%s/%s.stdout", ansDir,  qname);//정답번호.stdout파일 생성
	fd = creat(ans_fname, 0666);

	sprintf(tmp, "%s/%s.exe", ansDir, qname);//정답번호.exe파일 생성
	redirection(tmp, fd, STDOUT);
	close(fd);

	sprintf(std_fname, "%s/%s/%s.stdout", stuDir, id, qname);//정답번호.stdout파일 생성
	fd = creat(std_fname, 0666);

	sprintf(tmp, "%s/%s/%s.stdexe &", stuDir, id, qname);//redirection을 위한 tmp name 설정

	start = time(NULL);
	redirection(tmp, fd, STDOUT);//redirection
	
	sprintf(tmp, "%s.stdexe", qname);
	while((pid = inBackground(tmp)) > 0){
		end = time(NULL);
		if(difftime(end, start) > OVER){//처리 시간이 OVER초 이상 소요되면 false로 리턴하고 프로세스를 중지시킴.
			kill(pid, SIGKILL);
			close(fd);
			return false;
		}
	}

	close(fd);

	return compare_resultfile(std_fname, ans_fname);//학생답안,정답지의 stdout파일을 분석 후 같을 시 true, 다를 시 false를 리턴하는 함수로 이동
}

pid_t inBackground(char *name)
{
	pid_t pid;
	char command[64];
	char tmp[64];
	int fd;
	off_t size;
	
	memset(tmp, 0, sizeof(tmp));
	fd = open("background.txt", O_RDWR | O_CREAT | O_TRUNC, 0666);//background.txt파일 생성

	sprintf(command, "ps | grep %s", name);
	redirection(command, fd, STDOUT);//프로세스 목록 가져와서 커맨드 창에 입력하기

	lseek(fd, 0, SEEK_SET);
	read(fd, tmp, sizeof(tmp));

	if(!strcmp(tmp, "")){
		unlink("background.txt");
		close(fd);
		return 0;
	}

	pid = atoi(strtok(tmp, " "));
	close(fd);

	unlink("background.txt");
	return pid;
}

int compare_resultfile(char *file1, char *file2)//학생파일, 정답파일의 결과 비교(같을 시 true, 다를 시 false를 리턴함)
{
	int fd1, fd2;
	char c1, c2;
	int len1, len2;

	fd1 = open(file1, O_RDONLY);//학생파일 오픈
	fd2 = open(file2, O_RDONLY);//정답 파일 오픈

	while(1)
	{
		while((len1 = read(fd1, &c1, 1)) > 0){//공백이 나오기 전까지 한글자씩 읽어들여서 저장
			if(c1 == ' ') 
				continue;
			else 
				break;
		}
		while((len2 = read(fd2, &c2, 1)) > 0){
			if(c2 == ' ') 
				continue;
			else 
				break;
		}
		
		if(len1 == 0 && len2 == 0)//읽을 데이터가 없다면 break;
			break;

		to_lower_case(&c1);//읽은 값 소문자로 전부 변환
		to_lower_case(&c2);

		if(c1 != c2){//받아들인 두 실행파일의 stdout파일이 다르다면 파일을 닫은 후 false리턴
			close(fd1);
			close(fd2);
			return false;
		}
	}
	close(fd1);
	close(fd2);
	return true;
}
void redirection(char *command, int new, int old)//프로그램 redirection -> 출력 디스크립터를 변경
{
	int saved;
	saved = dup(old);
	dup2(new, old);

	system(command);

	dup2(saved, old);
	close(saved);
}

int get_file_type(char *filename)//답안파일의 종류 알아오는 함수
{
	char *extension = strrchr(filename, '.');//.이 포함된 가장 오른쪽 위치 찾아옴

	if(!strcmp(extension, ".txt"))//txt파일인지
		return TEXTFILE;
	else if (!strcmp(extension, ".c"))//c파일인지 확인 후 리턴
		return CFILE;
	else
		return -1;
}

void rmdirs(const char *path)//경로를 주고 그 위치에 있는 디렉토리를 삭제하는 함수
{
	struct dirent *dirp;
	struct stat statbuf;
	DIR *dp;
	char tmp[50];
	
	if((dp = opendir(path)) == NULL)//삭제할 디렉토리를 오픈
		return;

	while((dirp = readdir(dp)) != NULL)//디렉토리안 파일을 차례로 읽음
	{
		if(!strcmp(dirp->d_name, ".") || !strcmp(dirp->d_name, ".."))//.나 ..은 넘어감
			continue;

		sprintf(tmp, "%s/%s", path, dirp->d_name);

		if(lstat(tmp, &statbuf) == -1)//파일에 대한 정보를 읽어옴
			continue;//정보가 없으면 넘어감

		if(S_ISDIR(statbuf.st_mode))//디렉토리 안에 디렉토리가 있다면 그 디렉토리를 삭제(재귀)
			rmdirs(tmp);
		else//그렇지 않다면 그 파일과 언링크함
			unlink(tmp);
	}

	closedir(dp);
	rmdir(path);//경로에 위치한 디렉토리를 삭제
}

void to_lower_case(char *c)//대문자->소문자로 만들어주는 함수
{
	if(*c >= 'A' && *c <= 'Z')
		*c = *c + 32;
}

void print_usage()//사용법 출력함수(-h)
{
	printf("Usage : ssu_score <STD_DIR> <ANS_DIR> [OPTION]\n");
	printf("Option : \n");
	printf(" -m                modify question's score \n");
	printf(" -e <DIRNAME>      print error on 'DIRNAME/ID/qname_error.txt' file\n");
	printf(" -t <QNAMES>       compile QNAME.c with -lpthread option\n");
	printf(" -i <IDS>          print ID's wrong questions\n");
	printf(" -h                print usage\n");
}
