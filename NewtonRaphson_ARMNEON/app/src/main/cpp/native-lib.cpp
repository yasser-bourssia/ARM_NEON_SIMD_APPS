#include <jni.h>
#include <string>
#include <stdlib.h>
#include <arm_neon.h>
#include <omp.h>
#include <android/log.h>

#define	ARRAY_LENGTH	8
#define EPSILON         0.01

// TP 6 BOURSSIA SAADA
extern "C"
{
    void Newton_Raphson_Scalaire(float* A, float* B, float* C, float* D, float* E, float* F, float* X, float Epsilon, unsigned short Length)
    {

        int i ;

        float pow2,pow3,pow4,pow5;
        float px,pxd;



        for (i = 0; i < Length; i++){
            pow5 = X[i]*X[i]*X[i]*X[i]*X[i];
            pow4 = X[i]*X[i]*X[i]*X[i];
            pow3 = X[i]*X[i]*X[i];
            pow2 = X[i]*X[i];
            px = A[i] * pow5 + B[i] * pow4 + C[i] * pow3 + D[i] * pow2 + E[i] * X[i] + F[i];
            pxd = 5*A[i] * pow4 + 4*B[i] * pow3 + 3*C[i] * pow2 + 2*D[i] * X[i] + E[i];

            while(abs(px) > Epsilon) {

                X[i] = X[i] - (px/pxd);
                pow5 = X[i]*X[i]*X[i]*X[i]*X[i];
                pow4 = X[i]*X[i]*X[i]*X[i];
                pow3 = X[i]*X[i]*X[i];
                pow2 = X[i]*X[i];
                px = A[i] * pow5 + B[i] * pow4 + C[i] * pow3 + D[i] * pow2 + E[i] * X[i] + F[i];
                pxd = 5*A[i] * pow4 + 4*B[i] * pow3 + 3*C[i] * pow2 + 2*D[i] * X[i] + E[i];
            }

        }
    }

    void Newton_Raphson_NEON(float* A, float* B, float* C, float* D, float* E, float* F, float* X, float Epsilon, unsigned short Length)
    {


        float32x4_t a,b,c,d,e,f,x;
        float32x4_t px,pxd;
        float32x4_t pow5,pow4,pow3,pow2;
        float32x4_t temp;
        float32x4_t five=vdupq_n_f32(5),four =vdupq_n_f32(4),three=vdupq_n_f32(3),two=vdupq_n_f32(2);
        float32x4_t conditions;
        float32x4_t epsilons = vdupq_n_f32(Epsilon);
        float32x4_t mulpxpxd,newx;

        for (int i = 0;i<Length;i+=4){
            a = vld1q_f32(A + i);
            b = vld1q_f32(B + i);
            c = vld1q_f32(C + i);
            d = vld1q_f32(D + i);
            e = vld1q_f32(E + i);
            f = vld1q_f32(F + i);
            x = vld1q_f32(X + i);

            px = f;
            conditions=vcageq_f32(px,epsilons);

            while (vgetq_lane_f32(conditions,0)!=0 || vgetq_lane_f32(conditions,1)!=0 || vgetq_lane_f32(conditions,2)!=0 || vgetq_lane_f32(conditions,3)!=0  ) {
                px = vdupq_n_f32(0);
                pxd = vdupq_n_f32(0);
                // 5th Power
                pow5 = vmulq_f32(x, x);
                pow5 = vmulq_f32(pow5, pow5);
                pow5 = vmulq_f32(pow5, x);
                //4th Power
                pow4 = vmulq_f32(x, x);
                pow4 = vmulq_f32(pow4, pow4);
                //3rd Power
                pow3 = vmulq_f32(x, x);
                pow3 = vmulq_f32(pow3, x);
                //2nd Power
                pow2 = vmulq_f32(x, x);
                // calcul P(x)

                temp = vmulq_f32(a, pow5);
                px = vaddq_f32(px, temp);
                temp = vmulq_f32(b, pow4);
                px = vaddq_f32(px, temp);
                temp = vmulq_f32(c, pow3);
                px = vaddq_f32(px, temp);
                temp = vmulq_f32(d, pow2);
                px = vaddq_f32(px, temp);
                temp = vmulq_f32(e, x);
                px = vaddq_f32(px, temp);
                px = vaddq_f32(px, f);
                // Calcul dérivée P(x)
                temp = vmulq_f32(a, pow4);
                temp = vmulq_f32(temp, five);
                pxd = vaddq_f32(pxd, temp);
                temp = vmulq_f32(b, pow3);
                temp = vmulq_f32(temp, four);
                pxd = vaddq_f32(pxd, temp);
                temp = vmulq_f32(c, pow2);
                temp = vmulq_f32(temp, three);
                pxd = vaddq_f32(pxd, temp);
                temp = vmulq_f32(d, x);
                temp = vmulq_f32(temp, two);
                pxd = vaddq_f32(pxd, temp);
                pxd = vaddq_f32(pxd, e);
                // Comparaison entre px et epsilon

                conditions=vcageq_f32(px,epsilons);

                // Affectation des X

                pxd = vrecpeq_f32(pxd);
                mulpxpxd = vmulq_f32(px,pxd);

                newx = vsubq_f32(x,mulpxpxd);
                x = vbslq_f32(conditions,newx,x);

            }
            vst1q_f32(X+i,x);
        }









    }

    void Newton_Raphson_Scalaire_Full(float* A, float* B, float* C, float* D, float* E, float* F, float* X1, float* X2, float* X3, float* X4, float* X5, float Epsilon, unsigned short Length)
    {

        int i,j,k;

        float pow2,pow3,pow4,pow5;
        float px,pxd;
        bool flag;
        float temp;

        float** X = new float*[5];
        X[0] = X1;
        X[1] = X2;
        X[2] = X3;
        X[3] = X4;
        X[4] = X5;

        for (i = 0; i < Length; i++){

            for (j = 0; j<5;j++) {
                flag = true;
                while (flag == true) {
                    X[j][i] = rand() % 21 - 10;
                    temp = X[j][i];
                    pow5 = temp * temp * temp * temp * temp;
                    pow4 = temp * temp * temp * temp;
                    pow3 = temp * temp * temp;
                    pow2 = temp * temp;
                    px = A[i] * pow5 + B[i] * pow4 + C[i] * pow3 + D[i] * pow2 + E[i] * temp + F[i];
                    pxd = 5 * A[i] * pow4 + 4 * B[i] * pow3 + 3 * C[i] * pow2 + 2 * D[i] * temp +
                          E[i];

                    while (abs(px) > Epsilon) {
                        flag = false;
                        temp = X[j][i];
                        temp = temp - (px / pxd);
                        pow5 = temp * temp * temp * temp * temp;
                        pow4 = temp * temp * temp * temp;
                        pow3 = temp * temp * temp;
                        pow2 = temp * temp;
                        px = A[i] * pow5 + B[i] * pow4 + C[i] * pow3 + D[i] * pow2 + E[i] * temp +
                             F[i];
                        pxd = 5 * A[i] * pow4 + 4 * B[i] * pow3 + 3 * C[i] * pow2 +
                              2 * D[i] * temp +
                              E[i];

                        for (k = 0; k < j; k++) {
                            if (abs(temp - X[k][i]) < 0.03) flag = true;
                        }
                        X[j][i] = temp;
                    }

                }
            }
        }

    }






    void Newton_Raphson_NEON_Full(float* A, float* B, float* C, float* D, float* E, float* F, float* X1, float* X2, float* X3, float* X4, float* X5, float Epsilon, unsigned short Length)
    {



    }

    #define NB_CARACTERS_BY_LINE	8
    void float_PrintArray(const char* debut, const char* str, float* Array, unsigned short Length)
    {
        unsigned short	i;
        unsigned char	j;
        char tmp[1024];

        for(i=0;i<Length;i+=NB_CARACTERS_BY_LINE)
        {
            tmp[0] = 0x00;
            sprintf(tmp, "%s",debut);
            for(j=0;(j<NB_CARACTERS_BY_LINE)&&((i+j)<Length);j++)
                sprintf(tmp, "%s %s[%3d]=%8.2f",tmp,str,i+j,Array[i+j]);
            __android_log_print(ANDROID_LOG_ERROR, "PRINTARRAY", "%s\n", tmp);
        }
    }

    JNIEXPORT void JNICALL Java_com_example_tp_1newtonraphson_MainActivity_NewtonRaphson(
            JNIEnv *env, jobject /* this */) {
        float A[ARRAY_LENGTH] = {0.49,-4.79,-3.09,0.44,3.95,-0.62,4.05,-0.4};
        float B[ARRAY_LENGTH] = {-0.9408,1.2933,-20.3322,-1.2892,47.795,12.7224,-52.65,0.552};
        float C[ARRAY_LENGTH] = {-30.577617,74.994156,24.455805,-16.651756,144.965395,-71.395294,177.207345,18.66968};
        float D[ARRAY_LENGTH] = {-56.56516684,19.13685472,92.6178369,44.84375852,-35.8869824,18.79171764,177.4614339,-5.0737488};
        float E[ARRAY_LENGTH] = {-1.978922996,-141.5916244,-48.30611288,-13.34153265,-249.796004934,412.8538946,-1547.472852,-71.74255551};
        float F[ARRAY_LENGTH] = {0.426203815,-8.548211889,-36.39114834,-10.355313,-75.70514142576,-24.82362688,1329.792296,-19.92802863};
        float X1[ARRAY_LENGTH], X2[ARRAY_LENGTH], X3[ARRAY_LENGTH], X4[ARRAY_LENGTH], X5[ARRAY_LENGTH];

        bzero(X1, sizeof(float)*ARRAY_LENGTH);
        Newton_Raphson_Scalaire(A,B,C,D,E,F,X1,EPSILON,ARRAY_LENGTH);
        float_PrintArray("NR_Scalaire      :"," X",X1,ARRAY_LENGTH);
        __android_log_print(ANDROID_LOG_ERROR, "PRINTARRAY", "\n");

        bzero(X1, sizeof(float)*ARRAY_LENGTH);
        Newton_Raphson_NEON(A,B,C,D,E,F,X1,EPSILON,ARRAY_LENGTH);
        float_PrintArray("NR_NEON          :"," X",X1,ARRAY_LENGTH);
        __android_log_print(ANDROID_LOG_ERROR, "PRINTARRAY", "\n");

        memset(X1,0xFF,sizeof(float)*ARRAY_LENGTH);
        memset(X2,0xFF,sizeof(float)*ARRAY_LENGTH);
        memset(X3,0xFF,sizeof(float)*ARRAY_LENGTH);
        memset(X4,0xFF,sizeof(float)*ARRAY_LENGTH);
        memset(X5,0xFF,sizeof(float)*ARRAY_LENGTH);
        Newton_Raphson_Scalaire_Full(A,B,C,D,E,F,X1,X2,X3,X4,X5,EPSILON,ARRAY_LENGTH);
        float_PrintArray("NR_Scalaire_FUll :", "X1",X1,ARRAY_LENGTH);
        float_PrintArray("NR_Scalaire_FUll :", "X2",X2,ARRAY_LENGTH);
        float_PrintArray("NR_Scalaire_FUll :", "X3",X3,ARRAY_LENGTH);
        float_PrintArray("NR_Scalaire_FUll :", "X4",X4,ARRAY_LENGTH);
        float_PrintArray("NR_Scalaire_FUll :", "X5",X5,ARRAY_LENGTH);
        __android_log_print(ANDROID_LOG_ERROR, "PRINTARRAY", "\n");

        memset(X1,0xFF,sizeof(float)*ARRAY_LENGTH);
        memset(X2,0xFF,sizeof(float)*ARRAY_LENGTH);
        memset(X3,0xFF,sizeof(float)*ARRAY_LENGTH);
        memset(X4,0xFF,sizeof(float)*ARRAY_LENGTH);
        memset(X5,0xFF,sizeof(float)*ARRAY_LENGTH);
        Newton_Raphson_NEON_Full(A,B,C,D,E,F,X1,X2,X3,X4,X5,EPSILON,ARRAY_LENGTH);
        float_PrintArray("NR_NEON_FUll     :", "X1",X1,ARRAY_LENGTH);
        float_PrintArray("NR_NEON_FUll     :", "X2",X2,ARRAY_LENGTH);
        float_PrintArray("NR_NEON_FUll     :", "X3",X3,ARRAY_LENGTH);
        float_PrintArray("NR_NEON_FUll     :", "X4",X4,ARRAY_LENGTH);
        float_PrintArray("NR_NEON_FUll     :", "X5",X5,ARRAY_LENGTH);
    }

}