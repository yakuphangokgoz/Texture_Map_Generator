#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable

const sampler_t samp = CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_LINEAR;


float calculateCorrectionFactor(float contrast){

    return (259*(contrast + 255)) / (255*(259 - contrast));
}

float setRange(float factor ,float color_value){

        return factor*(color_value - 0.5f) + 0.5f;
}

__kernel void grayscale( __read_only image2d_t srcImage, __write_only image2d_t dstImage, int luminance, int brightness, int isInvertInt, __global int *histogramMap)
{
    int2 pos = (int2)(get_global_id(0), get_global_id(1));

    float4 srcColor = read_imagef(srcImage, samp, pos);
    float gray = 0;

    if(luminance == 0){
    gray = srcColor.x * 2126.0f / 10000.0f + srcColor.y * 7152.0f / 10000.0f + srcColor.z * 722.0f / 10000.0f;
    }

    if(luminance == 1){
    gray = srcColor.x * 299.0f / 1000.0f + srcColor.y * 587.0f / 1000.0f + srcColor.z * 114.0f / 1000.0f;
    }

    //float factor = calculateCorrectionFactor(contrast);
    float pixelColor = gray;




    pixelColor += brightness/100.0f;

    if(isInvertInt){
        pixelColor = 1.0f - pixelColor;
    }

    float4 pixel = (float4)(pixelColor, pixelColor, pixelColor,  srcColor.w);
    //pixel.xyz += pixel.xyz*brightness/20.0f;
    pixel = clamp(pixel, 0, 1.0f);

    int pixelHistogramKey = (int)floor(pixel.x * 255);
    int histogramMapValue = histogramMap[pixelHistogramKey];
    atomic_add(&histogramMap[pixelHistogramKey], 1);

    write_imagef(dstImage, pos, pixel);

}

float4 normalizeVector(float4 vector){

    float sum = sqrt(pow(vector.x,2) + pow(vector.y,2) + pow(vector.z,2));
    return (float4)(vector.x/sum, vector.y/sum, vector.z/sum, 1.0f);
}

int boolIntCompatibility(int value){

    if(value == 0){ return 1;}
    else if(value > 0){return -1;}
    else{return 0;}

}

__kernel void normalMap(__read_only image2d_t srcImage, __write_only image2d_t dstImage, float strength, float exponent, int isInvertRed, int isInvertGreen, int isInvertHeight){

    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    int2 posNE = (int2)(get_global_id(0)+1, get_global_id(1)-1);
    int2 posNW = (int2)(get_global_id(0)-1, get_global_id(1)-1);
    int2 posE = (int2)(get_global_id(0)+1, get_global_id(1));
    int2 posW = (int2)(get_global_id(0)-1, get_global_id(1));
    int2 posSE = (int2)(get_global_id(0)+1, get_global_id(1)+1);
    int2 posSW = (int2)(get_global_id(0)-1, get_global_id(1)+1);
    int2 posN = (int2)(get_global_id(0), get_global_id(1)-1);
    int2 posS = (int2)(get_global_id(0), get_global_id(1)+1);


    float4 srcColor = read_imagef(srcImage, samp, pos);
	
    float4 normalVector = (float4)(0,0,1.0f,1.0f);
    normalVector.x = (float)pow(strength, exponent)  * boolIntCompatibility(isInvertRed) * boolIntCompatibility(isInvertHeight) *
                                  ( read_imagef(srcImage, samp, posNE).x  -  read_imagef(srcImage, samp, posNW).x +
                                2*( read_imagef(srcImage, samp, posE).x - read_imagef(srcImage, samp, posW).x ) +
                                    read_imagef(srcImage, samp, posSE).x  -  read_imagef(srcImage, samp, posSW).x
                                   );

    normalVector.y = (float)pow(strength, exponent)  * boolIntCompatibility(isInvertGreen) * boolIntCompatibility(isInvertHeight) *
                                   ( read_imagef(srcImage, samp, posSW).x  -  read_imagef(srcImage, samp, posNW).x +
                                 2*( read_imagef(srcImage, samp, posS).x - read_imagef(srcImage, samp, posN).x ) +
                                     read_imagef(srcImage, samp, posSE).x  -  read_imagef(srcImage, samp, posNE).x
                                    );
    //normalVector.z = sqrt(1 - (pow((2*normalVector.x - 1),2) + pow((2*normalVector.y - 1),2))) / 2 + 0.5f;
    //normalVector.z = (float)sqrt(1.0f - (float)clamp((float)(normalVector.x + normalVector.y), 0.0f, 1.0f));
    //normalVector.z = sqrt(1.0f - (pow(normalVector.x, 2) + pow(normalVector.y, 2)));

    normalVector = normalizeVector(normalVector);

    normalVector = normalVector * 0.5f + 0.5f;



    write_imagef(dstImage, pos, normalVector);


}

__kernel void hgaussian(__read_only image2d_t srcImage,
                        __write_only image2d_t dstImage,
                        __global __read_only float *weights,
                        __global __read_only float *offsets,
                        const int numWeights)
{
    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float2 srcpos = (float2)(get_global_id(0), get_global_id(1));
    int index;
    float4 pixel = (float4)(0, 0, 0, 0);
    for (index = 0; index < numWeights; ++index) {
        pixel += read_imagef(srcImage, samp,
                             srcpos + (float2)(offsets[index], 0.0f)) *
                 weights[index];
    }
    write_imagef(dstImage, pos, clamp(pixel, 0.0f, 1.0f));
}

__kernel void vgaussian(__read_only image2d_t srcImage,
                        __write_only image2d_t dstImage,
                        __global __read_only float *weights,
                        __global __read_only float *offsets,
                        const int numWeights)
{
    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float2 srcpos = (float2)(get_global_id(0), get_global_id(1));
    int index;
    float4 pixel = (float4)(0, 0, 0, 0);
    for (index = 0; index < numWeights; ++index) {
        pixel += read_imagef(srcImage, samp,
                             srcpos + (float2)(0.0f, offsets[index])) *
                 weights[index];
    }
    write_imagef(dstImage, pos, clamp(pixel, 0.0f, 1.0f));
}


__kernel void legacyContrast(__read_only image2d_t srcImage, __write_only image2d_t dstImage, float average, float coefficient){

    int2 pos = (int2)(get_global_id(0), get_global_id(1));
    float4 srcColor = read_imagef(srcImage, samp, pos);
    float4 pixel = srcColor;

    if(srcColor.x < average){
        pixel.xyz -= average*coefficient;
    }

    if(srcColor.x > average){
        pixel.xyz += average*coefficient;
    }

    write_imagef(dstImage, pos, clamp(pixel, 0.0f, 1.0f));

}

