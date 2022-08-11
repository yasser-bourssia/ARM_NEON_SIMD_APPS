#include <jni.h>
#include <string>
#include <math.h>
#include <arm_neon.h>

#define BLACK   0x00
#define WHITE   0xFF

#define NIV_GRIS_SIMD
#define LAPLACE_SIMD
// TP7 BOURSSIA_SAADA
extern "C"
{
JNIEXPORT void
JNICALL Java_com_example_tp7ti_MyIntentService_nativeNiveauxGris(
        JNIEnv *env,
        jobject /* this */, jbyteArray data, jint nbPixels) {
    unsigned long i;
    jboolean  isCopy = true;
    unsigned char* pixels = (unsigned char*) env->GetByteArrayElements(data,&isCopy);

#ifndef NIV_GRIS_SIMD
    unsigned char grey;

    nbPixels <<= 2;
    for(i = 0; i < nbPixels; i+=4)
    {
        grey = (pixels[i]+pixels[i+1]+pixels[i+2])/3;
        pixels[i]   = grey;
        pixels[i+1] = grey;
        pixels[i+2] = grey;
        pixels[i+3] = 0xFF;
    }

#else

    uint8x16x4_t pix;
    uint8x16_t gray,tmp1,tmp2,tmp3;

    uint8x16_t FFs = vdupq_n_u8(WHITE);


    for(i=0;i<nbPixels*4;i+=64){

        pix = vld4q_u8(pixels+i);

        tmp1 = vshrq_n_u8(pix.val[0],2);
        tmp1 = vaddq_u8(tmp1,vshrq_n_u8(tmp1,2));

        tmp2 = vshrq_n_u8(pix.val[1],2);
        tmp2 = vaddq_u8(tmp2,vshrq_n_u8(tmp1,2));

        tmp3 = vshrq_n_u8(pix.val[2],2);
        tmp3 = vaddq_u8(tmp3,vshrq_n_u8(tmp1,2));


        gray = vaddq_u8(tmp1,tmp2);

        gray = vaddq_u8(gray,tmp3);

        pix.val[0] = gray;
        pix.val[1] = gray;
        pix.val[2] = gray;
        pix.val[3] = FFs;

        vst4q_u8(pixels+i,pix);


    }



#endif

    env->ReleaseByteArrayElements(data, (jbyte*)pixels, 0);
}

JNIEXPORT void
JNICALL Java_com_example_tp7ti_MyIntentService_nativeLaplace(

        JNIEnv *env,
        jobject /* this */, jbyteArray data, jint Width, jint Height, jbyteArray result) {
    unsigned long i, j;
    jboolean  isCopy = true;
    unsigned char* pixels = (unsigned char*) env->GetByteArrayElements(data,&isCopy);
    unsigned char* laplace  = (unsigned char*) env->GetByteArrayElements(result,&isCopy);

#ifndef LAPLACE_SIMD
    short Tmp;

    Width <<= 2;
    for(i = 0; i < Width; i+=4)
    {
        // Coloration des deux lignes supérieure et inférieure en noir
        laplace[i  ] = laplace[i+1] = laplace[i+2] = BLACK;
        laplace[i+3] = 0xFF;

        laplace[(Height - 1) * Width + i    ] = laplace[(Height - 1) * Width + i + 1] = laplace[(Height - 1) * Width + i + 2] = BLACK;
        laplace[(Height - 1) * Width + i + 3] = 0xFF;
    }
    for(j = 1; j < (Height -1); j++)
    {
        // Coloration des deux colonnes gauche et droite en noir
        laplace[j * Width    ] = laplace[j * Width + 1] = laplace[j * Width + 2] = BLACK;
        laplace[j * Width + 3] = 0xFF;

        laplace[(j + 1) * Width - 4] = laplace[(j + 1) * Width - 3] = laplace[(j + 1) * Width - 2] = BLACK;
        laplace[(j + 1) * Width - 1] = 0xFF;
    }


    for(j = 1; j < (Height-1); j++)
        for(i = 4; i < (Width - 4); i+=4)
        {
            Tmp =   (pixels[j*Width + i] << 3)
                    - pixels[(j-1)*Width + i - 4]- pixels[(j-1)*Width + i]- pixels[(j-1)*Width + i + 4]
                    - pixels[(j+1)*Width + i - 4]- pixels[(j+1)*Width + i]- pixels[(j+1)*Width + i + 4]
                    - pixels[j*Width + i - 4] - pixels[j*Width + i + 4];
            Tmp = abs(Tmp)>>3; // Pour eviter les overflow

            laplace[j * Width + i] = laplace[j * Width + i + 1] = laplace[j * Width + i + 2] = Tmp;
            laplace[j * Width + i + 3] = 0xFF;
        }

#else

    uint8x16x4_t pix,pix2;
    uint8x16x4_t convcc,convcl,convcr;
    uint8x16x4_t convhc,convhl,convlr;
    uint8x16x4_t convlc,convll,convhr;
    uint8x16x4_t finalresult;
    uint8x16_t temp1,temp2,temp3,temp;
    uint8x16_t newresult;

    uint8x16_t black = vdupq_n_u8(BLACK);
    uint8x16_t FFs = vdupq_n_u8(WHITE);
    uint8x16_t FFx = vdupq_n_u8(0xEE);
    Width <<= 2;
    for (i=0;i<Width;i+=64){

        pix = vld4q_u8(pixels+i);

        pix2 = vld4q_u8(pixels+ (Height - 1) * Width + i);

        pix.val[0] = pix.val[1] = pix.val[2] = black;
        pix.val[3] = FFs;
        pix2.val[0] = pix2.val[1] = pix2.val[2] = black;
        pix2.val[3] = FFs;

        vst4q_u8(laplace+i,pix);

        vst4q_u8(laplace+(Height - 1) * Width + i,pix2);



    }

    for (j=1;j<Height;j+=1){

        pix = vld4q_u8(pixels+ j * Width);
        pix2 = vld4q_u8(pixels+ (j + 1) * Width - 64);

        pix.val[0] = pix.val[1] = pix.val[2] = black;
        pix.val[3] = FFs;
        pix2.val[0] = pix2.val[1] = pix2.val[2] = black;
        pix2.val[3] = FFs;

        vst4q_u8(laplace + j * Width , pix);

        vst4q_u8(laplace + (j + 1) * Width - 64,pix2);
    }


   for (j = 1; j < (Height-1); j++){

       for (i = 16; i < (Width - 16); i+=64){

            convcc = vld4q_u8(pixels + j*Width + i);
            convcl = vld4q_u8(pixels + j*Width + i - 4);
            temp1 = vextq_u8(convcl.val[0],convcc.val[0],15);
            convcr = vld4q_u8(pixels + j*Width + i + 4);
            temp2 = vextq_u8(convcc.val[0],convcr.val[0],15);
            temp = vshlq_n_u8(convcc.val[0],3);
            newresult = temp;
            newresult = vsubq_u8(newresult,temp1);
            newresult = vsubq_u8(newresult,temp2);
            convhc = vld4q_u8(pixels + (j-1)*Width + i);
            convhl = vld4q_u8(pixels + (j-1)*Width + i - 4);
            temp1 = vextq_u8(convhl.val[0],convhc.val[0],15);
            convhr = vld4q_u8(pixels + (j-1)*Width + i + 4);
            temp2 = vextq_u8(convhc.val[0],convhr.val[0],15);
            newresult = vsubq_u8(newresult,convhc.val[0]);
            newresult = vsubq_u8(newresult,temp1);
            newresult = vsubq_u8(newresult,temp2);
            convlc = vld4q_u8(pixels + (j+1)*Width + i);
            convll = vld4q_u8(pixels + (j+1)*Width + i - 4);
            temp1 = vextq_u8(convll.val[0],convlc.val[0],15);
            convlr = vld4q_u8(pixels + (j+1)*Width + i + 4);
            temp2 = vextq_u8(convlc.val[0],convlr.val[0],15);
            newresult = vsubq_u8(newresult,convlc.val[0]);
            newresult = vsubq_u8(newresult,temp1);
            newresult = vsubq_u8(newresult,temp2);
           newresult = vshrq_n_u8(newresult,3);


            finalresult.val[0] = finalresult.val[1] = finalresult.val[2] = newresult;
            finalresult.val[3] = FFs;


            vst4q_u8(laplace+j * Width + i,finalresult);






       }




   }











#endif

    env->ReleaseByteArrayElements(data, (jbyte*)pixels, 0);
    env->ReleaseByteArrayElements(result, (jbyte*)laplace, 0);
}
}
