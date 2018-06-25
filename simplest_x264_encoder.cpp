#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "x264.h"

int main(int argc, char** argv) {
	int ret;
	FILE* fp_src = fopen("roger_yuv420p.yuv", "rb");
	FILE* fp_dst = fopen("roger.h264", "wb");

	int frame_num = 50;
	int csp = X264_CSP_I420;
	int width = 852, height = 480;

	     int iNal   = 0;  
         x264_nal_t* pNals = NULL;  
         x264_t* pHandle   = NULL;  
         x264_picture_t* pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));  
         x264_picture_t* pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));  
         x264_param_t* pParam = (x264_param_t*)malloc(sizeof(x264_param_t));  
          
         //Check  
         if(fp_src==NULL||fp_dst==NULL){  
                   printf("Error open files.\n");  
                   return -1;  
         }

         x264_param_default(pParam);  
         pParam->i_width   = width;  
         pParam->i_height  = height;  
         pParam->i_csp=csp;  
         x264_param_apply_profile(pParam, x264_profile_names[5]);  

         pHandle = x264_encoder_open(pParam);  
     
         x264_picture_init(pPic_out);  
         x264_picture_alloc(pPic_in, csp, pParam->i_width, pParam->i_height); 

         int y_size = pParam->i_width * pParam->i_height;
         if(frame_num==0){  
                   fseek(fp_src,0,SEEK_END);  
                   switch(csp){  
                   case X264_CSP_I444:frame_num=ftell(fp_src)/(y_size*3);break;  
                   case X264_CSP_I420:frame_num=ftell(fp_src)/(y_size*3/2);break;  
                   default:printf("Colorspace Not Support.\n");return -1;  
                   }  
                   fseek(fp_src,0,SEEK_SET); 
         }    
	int i = 0, j = 0;
         //Loop to Encode  
         for( i=0;i<frame_num;i++){  
                   switch(csp){  
                   case X264_CSP_I444:{  
                            fread(pPic_in->img.plane[0],y_size,1,fp_src);         //Y  
                            fread(pPic_in->img.plane[1],y_size,1,fp_src);         //U  
                            fread(pPic_in->img.plane[2],y_size,1,fp_src);         //V  
                            break;}  
                   case X264_CSP_I420:{  
                            fread(pPic_in->img.plane[0],y_size,1,fp_src);         //Y  
                            fread(pPic_in->img.plane[1],y_size/4,1,fp_src);     //U  
                            fread(pPic_in->img.plane[2],y_size/4,1,fp_src);     //V  
                            break;}  
                   default:{  
                            printf("Colorspace Not Support.\n");  
                            return -1;}  
                   }  
                   pPic_in->i_pts = i;  
   
                   ret = x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out);  
                   if (ret< 0){  
                            printf("Error.\n");  
                            return -1;  
                   }  
   
                   printf("Succeed encode frame: %5d\n",i);  
   
                   for ( j = 0; j < iNal; ++j){  
                             fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);  
                   }  
         }       

         i=0;  
         //flush encoder  
         while(1){  
                   ret = x264_encoder_encode(pHandle, &pNals, &iNal, NULL, pPic_out);  
                   if(ret==0){  
                            break;  
                   }  
                   printf("Flush 1 frame.\n");  
                   for (j = 0; j < iNal; ++j){  
                            fwrite(pNals[j].p_payload, 1, pNals[j].i_payload, fp_dst);  
                   }  
                   i++;  
         }  
         x264_picture_clean(pPic_in);  
         x264_encoder_close(pHandle);  
         pHandle = NULL;  
   
         free(pPic_in);  
         free(pPic_out);  
         free(pParam);  
   
         fclose(fp_src);  
         fclose(fp_dst);    
    return 0;
}
