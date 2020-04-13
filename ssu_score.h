#ifndef MAIN_H_
#define MAIN_H_

#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif
#ifndef STDOUT
	#define STDOUT 1
#endif
#ifndef STDERR
	#define STDERR 2
#endif
#ifndef TEXTFILE
	#define TEXTFILE 3
#endif
#ifndef CFILE
	#define CFILE 4
#endif
#ifndef OVER
	#define OVER 5
#endif
#ifndef WARNING
	#define WARNING -0.1
#endif
#ifndef ERROR
	#define ERROR 0
#endif
//버퍼의 기본 크기 설정
#define FILELEN 64
#define BUFLEN 1024
#define SNUM 100
#define QNUM 100
#define ARGNUM 5//최대 허용 가능한 인자수

struct ssu_scoreTable{//점수 테이블 구조체 선언(문제번호, 점수)
	char qname[FILELEN];
	double score;
};

void ssu_score(int argc, char *argv[]);//옵션을 확인하고 점수테이블을 생성하는 함수
int check_option(int argc, char *argv[]);//옵션여부를 체크하고 그에 필요한 인자를 받아들이는 함수
void print_usage();

void score_students();
double score_student(int fd, char *id);
void write_first_row(int fd);

char *get_answer(int fd, char *result);
int score_blank(char *id, char *filename);
double score_program(char *id, char *filename);
double compile_program(char *id, char *filename);
int execute_program(char *id, char *filname);
pid_t inBackground(char *name);
double check_error_warning(char *filename);
int compare_resultfile(char *file1, char *file2);
void do_mOption(void);
int  rewrite(int line,double score);
void do_iOption(char (*ids)[FILELEN]);//i옵션수행함수(학번 찾아서 틀린 답 출력)
int get_index(char *f_line,int cnt);
char* get_qnumber(char *f_line,int idx);
int is_exist(char (*src)[FILELEN], char *target);

int is_thread(char *qname);
void redirection(char *command, int newfd, int oldfd);
int get_file_type(char *filename);
void rmdirs(const char *path);
void to_lower_case(char *c);

void set_scoreTable(char *ansDir);
void read_scoreTable(char *path);
void make_scoreTable(char *ansDir);
void write_scoreTable(char *filename);
void set_idTable(char *stuDir);
int get_create_type();

void sort_idTable(int size);
void sort_scoreTable(int size);
void get_qname_number(char *qname, int *num1, int *num2);

#endif
