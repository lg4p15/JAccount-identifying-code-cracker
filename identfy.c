#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#define LIMIT 188
#define PERMIT 128
#define SETBEND(X) opt=pdt+(X)*pwid;optmp=opt+letter->start;opt+=letter->stop;
#define NEWLETTER {crt->next=(Letter*)malloc(sizeof(Letter));crt=crt->next;crt->start=length;crt->stop=i;crt->next=NULL;flag=0;}
int phgt,pwid,i,j,dnm;
unsigned char *pdt,*opt,*optmp,*res;
FILE *fp;
typedef struct Letter{int start,stop;struct Letter *next;}Letter;
char getL(Letter *letter){
	int up=0,down=phgt-1,cur=j=0,diff,curdif=PERMIT;
	do{	SETBEND(up)
		while(optmp<opt&&*optmp>=LIMIT&&++optmp);
	}while(optmp==opt&&++up);
	do{	SETBEND(down)
		while(optmp<opt&&*optmp>=LIMIT&&++optmp);
	}while(optmp==opt&&--down);
	unsigned char bb=0,mask[32]={},r=++down-up,c=letter->stop-letter->start,fr,fc;
	for(i=up;i<down;++i){
		SETBEND(i)
		do{	if(*optmp<LIMIT)++bb;
			if(++j==8){
				mask[cur++]=bb;
				bb=j=0;
				if(cur>31)return 0;
			}
			else bb<<=1;
		}while(++optmp<opt);
	}
	unsigned curlet=0,cprbf;
	for(mask[cur]=bb<<=(7-j),i=1;i<=dnm;++i){
		fseek(fp,64*i,SEEK_SET);
		fread(&fr,1,1,fp);
		fread(&fc,1,1,fp);
		if(r==fr&&c==fc){
			for(diff=0,j=0;j<8;++j){
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
	return curlet;
}
char* identfy(char *argv){
    struct jpeg_decompress_struct cif;
    struct jpeg_error_mgr jerr;
    if ((fp=fopen(argv, "rb"))==NULL)return 0;
    cif.err=jpeg_std_error(&jerr);
    jpeg_create_decompress(&cif);
    jpeg_stdio_src(&cif, fp);
    jpeg_read_header(&cif, TRUE);
    jpeg_start_decompress(&cif);
	phgt=cif.output_height;
	pwid=cif.output_width;
    int flag=pwid * cif.output_components,length=pwid*phgt;
    optmp=opt=malloc(flag*phgt);
    while (cif.output_scanline < phgt)
		jpeg_read_scanlines(&cif, &optmp,1),optmp += flag;
    for(optmp=opt,res=pdt=malloc(length),j=0;j<length;++j)
        *res++=(*optmp+*(optmp+1)+*(optmp+2))/3,optmp+=3;
    free(opt);
    jpeg_finish_decompress(&cif);
    jpeg_destroy_decompress(&cif);
    fclose(fp);
	Letter home ={0,0,NULL},*crt=&home;
	for(i=0,flag=0;i<pwid;i++){
		for(res=pdt+i,j=0;j<phgt&&*res>=LIMIT;j++,res+=pwid);
		if(j==phgt&&flag)NEWLETTER
		else if(j<phgt&&(!flag))length=i,flag=1;
	}
	if(flag)NEWLETTER
	Letter *letter = home.next;
	char rst[8]={},*rs=rst;
	fp=fopen("SJL.dat","r");
	fseek(fp,2,SEEK_SET);
	fread(&dnm,sizeof(int),1,fp);
	while(letter!=NULL)
		*rs++=getL(letter),letter=letter->next;
	free(pdt);
	fclose(fp);
	while(letter!=NULL){
		crt=letter->next;
		free(letter);
		letter=crt;
	}
	return rst;
}
