#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

// used to call "mkdir" function, mkdir can also be done with "system()"
#include <sys/stat.h>

DIR* dir;
DIR* innerDir;
FILE* file;
FILE* iimage;
FILE* dimage;
struct dirent* iter;
struct dirent* inner;

int re;
int pivot=0;
int allPass=1;

char a[1000][500]={{0}};

char path[1000]={0};
char fileAddr[1000]={0};
char cmd[1000]={0};
char buff[100]={0};


void concatDir(char* re, char* in1, char* in2){
	strcpy(re,in1);
	if(re[strlen(re)-1]!='/'){
		int len = strlen(re);
		re[len] = '/';
		re[len+1] = '\0';
	}
	strcat(re,in2);
	if(re[strlen(re)-1]!='/'){
		int len = strlen(re);
		re[len] = '/';
		re[len+1] = '\0';
	}
}

void changeAns(void){

	while( (iter = readdir(dir)) ){
		if(!strcmp(iter->d_name,".")) continue;
		if(!strcmp(iter->d_name,"..")) continue;

		if(iter->d_type!=DT_DIR) continue; //is not a directory

		printf("processing file:	%s\n",iter->d_name);
		concatDir(path,"./testcases/", iter->d_name);

		// copy image to current dir for executing pipeline
		strcpy(cmd,"cp ");
		strcat(cmd,path);
		strcat(cmd,"iimage.bin .");
		re = system(cmd);
		if(re!=0) printf("copy iimage or dimage fail!\n");

		strcpy(cmd,"cp ");
		strcat(cmd,path);
		strcat(cmd,"dimage.bin .");
		re = system(cmd);
		if(re!=0) printf("copy iimage or dimage fail!\n");

		// execute pipeline
		re = system("./golden_simulator/pipeline");
		if(re!=0) printf("Something wrong while executing pipeline!\n");
		
		// copy files back to each directory
		// i.e. change answer
		strcpy(cmd,"cp ./error_dump.rpt ");
		strcat(cmd,path);
		system(cmd);
		strcpy(cmd,"cp ./snapshot.rpt ");
		strcat(cmd,path);
		system(cmd);
	}

}	

void usingDiff(void){
		// diff snapshot
		printf("	snapshot:\n");
		strcpy(cmd,"diff ./snapshot.rpt ");
		strcat(cmd,path);
		strcat(cmd,"snapshot.rpt");
		re = system(cmd);
		if(re!=0) printf("cannot execute diff on snapshot!\n");

		// diff error_dump
		printf("	error_dump:\n");
		strcpy(cmd,"diff ./error_dump.rpt ");
		strcat(cmd,path);
		strcat(cmd,"error_dump.rpt");
		re = system(cmd);
		if(re!=0) printf("cannot execute diff on error_dump!\n");

		//close all pass detect because I can't deal with diff output
		allPass=0;
}

void usingMyAnsCheckProgram(void){
	
		//execute ans check
		// this require answer snapshot and error_dump in workspace/ directory
		strcpy(cmd,"./ansCheck ");
		strcat(cmd,iter->d_name);
		re = system(cmd);
		strcpy(a[pivot],iter->d_name);
		strcat(a[pivot],"   	-----	");
		if(re==256){ // i.e. return 1
			strcat(a[pivot],"Failed at snapshot.rpt\n");
			allPass=0;
		}
		else if(re==512){
			strcat(a[pivot],"Failed at error_dump.rpt\n");
			allPass=0;
		}
		else if(re==768){
			strcat(a[pivot],"Failed at snapshot.rpt and error_dump.rpt\n");
			allPass=0;
		}
		else{
			strcat(a[pivot],"[ALL PASS !!!!!]\n");
		}

		pivot++;

	
}
	
void checkAns(void){
	while( (iter = readdir(dir)) ){
		if(!strcmp(iter->d_name,".")) continue;
		if(!strcmp(iter->d_name,"..")) continue;

		if(iter->d_type!=DT_DIR) continue; //is not a directory

		printf("judging file: %s\n",iter->d_name);
		concatDir(path,"./testcases/", iter->d_name);

		// copy image to current dir for executing pipeline
		strcpy(cmd,"cp ");
		strcat(cmd,path);
		strcat(cmd,"iimage.bin .");
		re = system(cmd);
		if(re!=0) printf("copy iimage or dimage fail!\n");
		strcpy(cmd,"cp ");
		strcat(cmd,path);
		strcat(cmd,"dimage.bin .");
		re = system(cmd);
		if(re!=0) printf("copy iimage or dimage fail!\n");

		// copy ans to workspace
		// for executing my "ansCheck" program, if you use diff command, this is useless
		strcpy(cmd,"cp ");
		strcat(cmd,path);
		strcat(cmd,"snapshot.rpt ./workspace");
		re = system(cmd);
		if(re!=0) printf("copy answer fail!\n");
		strcpy(cmd,"cp ");
		strcat(cmd,path);
		strcat(cmd,"error_dump.rpt ./workspace");
		re = system(cmd);
		if(re!=0) printf("copy answer fail!\n");

		// execute pipeline
		re = system("./pipeline");
		if(re!=0) printf("Something wrong while executing pipeline!\n");

		usingMyAnsCheckProgram();
		
		//usingDiff();
	
	}
}

int main(int argc, char* argv[]){
	mkdir("./workspace",0777);

	if( NULL==(dir = opendir("./testcases/") ) ){
		printf("Require a directory named 'testcases' and filled it with testcases\n");
		return 0;
	}

	re = system("rm result.log");
	if(re!=0) printf("Can't remove result.log, the result will append to previous result.log\n");

	// any additional argument on execution will enter change answer mode
	if(argc>1){
		printf("You want to update all testcases answer with new golden simulator?\n");
		printf("Please check you already copy the golden simulator to current directory: (Y/N)");
		char c;
re:
		scanf("%c",&c);
		if(c=='N'||c=='n'){
			printf("Process terminated ...\n");
			return 0;
		}
		else if(c=='Y'||c=='y'){
			printf("Processing ...\n");
			changeAns();
			printf("End of process!\n");
			return 0;
		}
		else{
			printf("please enter Y/N:");
			goto re;
		}
	}

	checkAns();

	// print judge result in single lines
	int i;
	for(i=0;i<pivot;i++){
		printf("%s",a[i]);
	}
	
	// congrat if all pass
	if(allPass){
		char line[1000];
		FILE* a = fopen("allpass","r");
		printf("\n\n");
		while(fgets(line,1000,a)!=NULL){
			printf("%s",line);
		}
		printf("\n\n");
	}
	
	return 0;

}

