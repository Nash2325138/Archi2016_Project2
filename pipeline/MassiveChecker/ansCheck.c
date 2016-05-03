#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
    need to create a directory in same level named judgeAns to store correct answers
    i.e.

    simulator---|--(this file or the file include this code)
                |
                |
                |---workspace-----snapshot.rpt(answer)
                |             |
                |             |
                |             |--error_dump.rpt(answer)
                |
                |
                |---snapshot.rpt(yours)
                |
                |
                |---error_dump.rpt(yours)
**/

int main(int argc, char* argv[]){

	FILE* judgeSnapshot = fopen("snapshot.rpt","r");
	FILE* judgeError = fopen("error_dump.rpt","r");

	FILE* ansSnapshot = fopen("./workspace/snapshot.rpt","r");
	FILE* ansError = fopen("./workspace/error_dump.rpt","r");

	FILE* log = fopen("result.log","a");

	char c[200]={0},ans[200]={0};

	int cycle = 0;

	int fail1=1;
	int fail2=1;

	if(argc>1){
		fprintf(log,"judge result of: %s\n",argv[1]);
	}

	while(fgets(ans,200,ansSnapshot) ){
		int reg = 0;
		if( !fgets(c,200,judgeSnapshot) ){
			printf("snapshot FAILED at %d...\n",cycle);
			printf("ans = %s",ans);
			printf("you = %s",c);
			fprintf(log,"snapshot FAILED at %d...\n",cycle);
			fprintf(log,"ans = %s",ans);
			fprintf(log,"you = %s",c);
			goto end;
		}
		if( strcmp(c,ans) ){
			printf("snapshot FAILED at %d...\n",cycle);
			printf("ans = %s",ans);
			printf("you = %s",c);
			fprintf(log,"snapshot FAILED at %d...\n",cycle);
			fprintf(log,"ans = %s",ans);
			fprintf(log,"you = %s",c);
			goto end;
		}
		// $0 ~ PC total 32 +5
		for(reg=0;reg<=37;reg++){
			fgets(ans,200,ansSnapshot);
			if( !fgets(c,200,judgeSnapshot) ){
				printf("snapshot FAILED at %d...\n",cycle);
				printf("ans = %s",ans);
				printf("you = %s",c);
				fprintf(log,"snapshot FAILED at %d...\n",cycle);
				fprintf(log,"ans = %s",ans);
				fprintf(log,"you = %s",c);
				goto end;
			}
			if( strcmp(c,ans) ){
				printf("snapshot FAILED at %d...\n",cycle);
				printf("ans = %s",ans);
				printf("you = %s",c);
				fprintf(log,"snapshot FAILED at %d...\n",cycle);
				fprintf(log,"ans = %s",ans);
				fprintf(log,"you = %s",c);
				goto end;
			}
		}
		char trash[2] ;
		fread(trash,2,sizeof(char),ansSnapshot);
		fread(trash,2,sizeof(char),judgeSnapshot);
		//printf("pass %d cycle\n",cycle);

		cycle++;
	}
	fail1=0;
	printf("snapshot.rpt:	 PASS!!!\n");
	fprintf(log,"snapshot.rpt:	 PASS!!!\n");
end:

	cycle = 0;
	while(fgets(ans,200,ansError) ){
		if( !fgets(c,200,judgeError) ){
			printf("error_dump FAILED at %d...\n",cycle);
			printf("ans = %s",ans);
			printf("you = %s",c);
			printf("\n\n\n");
			fprintf(log,"error_dump FAILED at %d...\n",cycle);
			fprintf(log,"ans = %s",ans);
			fprintf(log,"you = %s",c);
			fprintf(log,"\n\n\n");
			goto ret;
		}
		if( strcmp(c,ans) ){
			printf("error_dump FAILED at %d...\n",cycle);
			printf("ans = %s",ans);
			printf("you = %s",c);
			printf("\n\n\n");
			fprintf(log,"error_dump FAILED at %d...\n",cycle);
			fprintf(log,"ans = %s",ans);
			fprintf(log,"you = %s",c);
			fprintf(log,"\n\n\n");
			goto ret;
		}

		//char trash ; scanf("%c",&trash); scanf("%c",&trash);
		//printf("pass %d cycle\n",cycle);

		cycle++;
	}
	fail2=0;
	printf("error_dump.rpt:	 PASS!!!\n\n\n");
	fprintf(log,"error_dump.rpt:	 PASS!!!\n\n\n");

	fclose(log);
	fclose(judgeSnapshot);
	fclose(judgeError);
	fclose(ansSnapshot);
	fclose(ansError);

ret:
	if(fail1&&fail2) return 3;
	else if(fail1) return 1;
	else if(fail2) return 2;
	return 0;

}



