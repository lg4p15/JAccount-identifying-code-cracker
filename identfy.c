#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

#define LIMIT 188
#define PERMIT 128
#define SETBEND(X) opt=pdt+(X)*pwid;optmp=opt+letter->start;opt+=letter->stop;
int phgt;
int pwid;
unsigned char* pdt;
unsigned char* opt;
unsigned char* optmp;
int i,j,dnm;
FILE *fp;
typedef struct Letter{
	int start;
	int stop;
	struct Letter *next;
}Letter;
char getL(Letter *letter){
	int up=0,down=phgt-1;
	do{	SETBEND(up)
		while(optmp<opt&&*optmp>=LIMIT&&++optmp);
	}while(optmp==opt&&++up);
	do{	SETBEND(down)
		while(optmp<opt&&*optmp>=LIMIT&&++optmp);
	}while(optmp==opt&&--down);
	++down;
	unsigned char bb=0;
	int cur=0;
	j=0;
	unsigned char mask[32]={};
	for(i=up;i<down;++i){
		SETBEND(i)
		do{	if(*optmp<LIMIT)
				++bb;
			if(++j==8){
				mask[cur++]=bb;
				bb=0;
				j=0;
				if(cur>31)return 0;
			}
			else
				bb<<=1;
		}while(++optmp<opt);
	}
	bb<<=(7-j);
	mask[cur]=bb;

	unsigned char r=down-up;
	unsigned char c=letter->stop-letter->start;
	unsigned char fr,fc;

	int diff;
	int curdif=PERMIT;
	unsigned curlet=0;
	unsigned cprbf;
	for(i=1;i<=dnm;++i){
		fseek(fp,64*i,SEEK_SET);
		fread(&fr,1,1,fp);
		fread(&fc,1,1,fp);
		if(r==fr&&c==fc){
			diff=0;
			for(j=0;j<8;++j){
				fread(&cprbf,4,1,fp);
				cprbf^=*((unsigned*)mask+j);
				while(cprbf)
					cprbf&=cprbf-1,diff++;
			}
			if(diff<curdif){
				curdif=diff;
				fseek(fp,64*i+62,SEEK_SET);
				fread(&curlet,1,1,fp);
			}
		}
	}
	//if need,attend i/l extra there
	return curlet;
}

char* identfy(char *argv){
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    if ((fp = fopen(argv, "rb")) == NULL)
		return 0;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, fp);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
	phgt=cinfo.output_height;
	pwid=cinfo.output_width;
    int flag=pwid * cinfo.output_components;
    int length=pwid*phgt;
    opt = malloc(flag*phgt);
    optmp=opt;
    while (cinfo.output_scanline < phgt)
		jpeg_read_scanlines(&cinfo, &optmp,1),optmp += flag;
    optmp=opt;
    pdt = malloc(length);
	unsigned char *res =pdt;
    for(j=0;j<length;++j)
        *res++=(*optmp+*(optmp+1)+*(optmp+2))/3,optmp+=3;
    free(opt);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(fp);
	flag=0;
	Letter home ={0,0,NULL};
	Letter *current = &home;
	for(i = 0;i<pwid;i++){
		res=pdt+i;
		for(j=0;j<phgt&&*res>=LIMIT;j++,res+=pwid);
		if(j==phgt&&flag){
			current->next = (Letter*)malloc(sizeof(Letter));
			current=current->next;
			current->start=length;
			current->stop=i;
			current->next=NULL;
			flag=0;
		}
		else if(j<phgt&&(!flag)){
			length=i;
			flag=1;
		}
	}
	if(flag){	
		current->next = (Letter*)malloc(sizeof(Letter));
		current=current->next;
		current->start=length;
		current->stop=pwid;
		current->next=NULL;
	}
	Letter *letter = home.next;
	char rst[8];
	char *rs=rst;
	fp=fopen("SJL.dat","r");
	fseek(fp,2,SEEK_SET);
	fread(&dnm,sizeof(int),1,fp);
	while(letter!=NULL)
		*rs++=getL(letter),letter=letter->next;
	*rs=0;

	free(pdt);
	fclose(fp);
	while(letter!=NULL){
		current=letter->next;
		free(letter);
		letter=current;
	}
	return rst;
}
