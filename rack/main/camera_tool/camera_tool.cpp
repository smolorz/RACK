/*
 * RACK - Robotics Application Construction Kit
 * Copyright (C) 2005-2006 University of Hannover
 *                         Institute for Systems Engineering - RTS
 *                         Professor Bernardo Wagner
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Authors
 *      Marko Reimer <reimer@rts.uni-hannover.de>
 *
 */

#include <main/camera_tool.h>

CameraTool::CameraTool()
{
    gdos = NULL;
    initColorTables();
}

CameraTool::CameraTool(RackMailbox *p_mbx, int gdos_level)
{
    gdos = new GdosMailbox(p_mbx, gdos_level);
    initColorTables();
}

CameraTool::~CameraTool()
{
    if (gdos)
        delete gdos;
}

void CameraTool::initColorTables()
{
    unsigned int i; 
    
//define colors at positions
//  int colorBeacons[]    = {BLACK,      BLUE ,      VIOLET,     RED   ,     ORANGE,      YELLOW,    WHITE}
    int colorBeacons[7]   = {0x00000000, 0x00FF0000, 0x00FF00FF, 0x000000FF, 0x0000C0FF, 0x0000FFFF, 0x00FFFFFF };
    unsigned int colorPositions[7] = {0,          126,         256,        512,        1024,       2048,        4095};

    double colorRed   = 0.0; 
    double colorGreen = 0.0;
    double colorBlue  = 0.0;
    double stepRed  =0.0;
    double stepGreen=0.0;
    double stepBlue =0.0; 
    int    indexId = 0;
      
    //colorLookuptableThermalRed12
    colorLookuptableThermalRed12[0] = colorBeacons[0];
    
    for (i=0; i < sizeof(colorLookuptableThermalRed12)/sizeof(colorLookuptableThermalRed12[0]); i++)
    {
        if (i == colorPositions[indexId])
        {
            colorRed   = (colorBeacons[indexId]&0x00FF0000) >> 16 ;
            colorGreen = (colorBeacons[indexId]&0x0000FF00) >> 8;
            colorBlue  =  colorBeacons[indexId]&0x000000FF;
            stepRed   = (((double) (((colorBeacons[indexId+1]&0x00FF0000) >> 16) - ((colorBeacons[indexId]&0x00FF0000) >> 16) ) ) / (colorPositions[indexId+1]-colorPositions[indexId]));
            stepGreen = (((double) (((colorBeacons[indexId+1]&0x0000FF00) >>  8) - ((colorBeacons[indexId]&0x0000FF00) >>  8) ) ) / (colorPositions[indexId+1]-colorPositions[indexId])); 
            stepBlue  = (((double) (((colorBeacons[indexId+1]&0x000000FF)      ) - ((colorBeacons[indexId]&0x000000FF)      )) ) / (colorPositions[indexId+1]-colorPositions[indexId]));
            indexId++;
            GDOS_DBG_INFO("colorRed=%f colorGreen=%f colorBlue=%f stepRed=%f stepGreen=%f stepBlue=%f indexId=%i\n", colorRed, colorGreen, colorBlue, stepRed, stepGreen, stepBlue, indexId-1);            
        }
        
        colorRed   = colorRed   + stepRed;
        colorGreen = colorGreen + stepGreen;
        colorBlue  = colorBlue  + stepBlue;
        
        colorLookuptableThermalRed12[i] = (( ((int) floor(colorRed))&0xFFFFFFFF) << 16) | (( ((int) floor(colorGreen))&0xFFFFFFFF) << 8) | (( ((int) floor(colorBlue))&0xFFFFFFFF)); 
        
        //printf("colorRed=%f colorGreen=%f colorBlue=%f colorTable=%i \n", colorRed, colorGreen, colorBlue, colorLookuptableThermalRed12[i]);
    }
}

int CameraTool::clip(int in)
{
    if (in < 0)
    {
        return 0;
    }
    else if (in > 255)
    {
        return 255;
    }
    return in;
}

int CameraTool::convertCharUYVY2RGB(uint8_t* outputData, uint8_t* inputData,
                        int width, int height)
{
    int i,j;
    int u,y0,v,y1;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j += 2) //step = 2
        {//handling 4 byte <=> 2 pixel each turn
            u  = (int) (0xff & inputData[2 * (i * width + j) + 0]);
            y0 = (int) (0xff & inputData[2 * (i * width + j) + 1]);
            v  = (int) (0xff & inputData[2 * (i * width + j) + 2]);
            y1 = (int) (0xff & inputData[2 * (i * width + j) + 3]);

            outputData[3 * (i * width + j) + 2] = (uint8_t) clip(( 298 * (y0-16)                 + 409 * (v-128) + 128) >> 8);//bo
            outputData[3 * (i * width + j) + 1] = (uint8_t) clip(( 298 * (y0-16) - 100 * (u-128) - 208 * (v-128) + 128) >> 8);//g0
            outputData[3 * (i * width + j) + 0] = (uint8_t) clip(( 298 * (y0-16) + 516 * (u-128)                 + 128) >> 8);//r0

            outputData[3 * (i * width + j) + 5] = (uint8_t) clip(( 298 * (y1-16)                 + 409 * (v-128) + 128) >> 8);
            outputData[3 * (i * width + j) + 4] = (uint8_t) clip(( 298 * (y1-16) - 100 * (u-128) - 208 * (v-128) + 128) >> 8);
            outputData[3 * (i * width + j) + 3] = (uint8_t) clip(( 298 * (y1-16) + 516 * (u-128)                 + 128) >> 8);
        }
    }
    return 0;
}


int CameraTool::convertCharUYVY2BGR(uint8_t* outputData, uint8_t* inputData,
                               int width, int height)
{
    int i,j;
    int u,y0,v,y1;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j += 2) //step = 2
        {//handling 4 byte <=> 2 pixel each turn
            u  = (int) (0xff & inputData[2 * (i * width + j) + 0]);
            y0 = (int) (0xff & inputData[2 * (i * width + j) + 1]);
            v  = (int) (0xff & inputData[2 * (i * width + j) + 2]);
            y1 = (int) (0xff & inputData[2 * (i * width + j) + 3]);
            outputData[3 * (i * width + j) + 0] = (uint8_t) clip(( 298 * (y0-16)                 + 409 * (v-128) + 128) >> 8);//bo
            outputData[3 * (i * width + j) + 1] = (uint8_t) clip(( 298 * (y0-16) - 100 * (u-128) - 208 * (v-128) + 128) >> 8);//g0
            outputData[3 * (i * width + j) + 2] = (uint8_t) clip(( 298 * (y0-16) + 516 * (u-128)                 + 128) >> 8);//r0

            outputData[3 * (i * width + j) + 3] = (uint8_t) clip(( 298 * (y1-16)                 + 409 * (v-128) + 128) >> 8);
            outputData[3 * (i * width + j) + 4] = (uint8_t) clip(( 298 * (y1-16) - 100 * (u-128) - 208 * (v-128) + 128) >> 8);
            outputData[3 * (i * width + j) + 5] = (uint8_t) clip(( 298 * (y1-16) + 516 * (u-128)                 + 128) >> 8);
        }
    }
    return 0;
}


int CameraTool::convertCharUYVY2Gray(uint8_t* outputData, uint8_t* inputData,
                                int width, int height)
{
    int i,j;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++ )
        {
            outputData[i * width + j] = inputData[(2 * (i * width + j)) + 1];
        }
    }
    return 0;
}

int CameraTool::convertCharBGR2RGB(uint8_t* outputData, uint8_t* inputData,
                                int width, int height)
{
    int i,j;
    uint8_t swap;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++ )
        {
            swap                                = inputData[(i * width + j) * 3];//swap = input_B
            outputData[(i * width + j) * 3]     = inputData[(i * width + j) * 3 + 2];
            outputData[(i * width + j) * 3 +1 ] = inputData[(i * width + j) * 3 + 1];
            outputData[(i * width + j) * 3 + 2] = swap;
        }
    }
    return 0;
}


int CameraTool::convertCharMono82RGBThermalRed(uint8_t* outputData, uint8_t* inputData,
                                int width, int height)
{
    int i,j;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++ )
        {
            outputData[(i * width + j) * 3    ] = (colorLookuptableThermalRed12[inputData[i * width + j]] & 0x00ff0000) >> 16;
            outputData[(i * width + j) * 3 + 1] = (colorLookuptableThermalRed12[inputData[i * width + j]] & 0x0000ff00) >>  8;
            outputData[(i * width + j) * 3 + 2] = (colorLookuptableThermalRed12[inputData[i * width + j]] & 0x000000ff);
        }
    }
    return 0;
}


int CameraTool::convertCharMono122RGBThermalRed(uint8_t* outputData, uint8_t* inputData,
                                int width, int height)
{
    int i,j;
    short inputColor; 

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++ )
        {
            inputColor = (inputData[(i * width + j) * 2] << 8) | inputData[(i * width + j) * 2 + 1];
            if(inputColor > 4095) 
            {   
                GDOS_ERROR("inputColor:%i >4095! \n", inputColor); 
                printf("inputColor:%i >4095! \n", inputColor);                 
                printf("i:%i j:%i input:%x %x \n",i,j,inputData[(i * width + j) * 2], inputData[(i * width + j) * 2 + 1] ); 
                return -EINVAL;
            }
            
            outputData[(i * width + j) * 3    ] = (colorLookuptableThermalRed12[inputColor] & 0x00ff0000) >> 16;
            outputData[(i * width + j) * 3 + 1] = (colorLookuptableThermalRed12[inputColor] & 0x0000ff00) >>  8;
            outputData[(i * width + j) * 3 + 2] = (colorLookuptableThermalRed12[inputColor] & 0x000000ff);
        }
    }
    
    return 0;
}

int CameraTool::histogramStrechMono8(uint8_t* outputData, uint8_t* inputData,
                                int width, int height, int bottomPercentage, int ceilingPercentage)
{
    int i,j;
    int bottomPixel, ceilingPixel;
    int bottomCount, ceilingCount;
    int bottomIndex, ceilingIndex; 
    double multiplicationFactor; 
    int histogram[256];
    
    memset(histogram, 0, 256 * sizeof(int));

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++ )
        {
            histogram[inputData[i*width+j]]++;
        }
    }
    bottomPixel  = (int) floor(bottomPercentage  * width * height / 100);
    ceilingPixel = (int) floor(ceilingPercentage * width * height / 100);

    bottomCount = 0; 
    ceilingCount = 0; 
    
    bottomIndex = 0;
    for (i = 0; i < 256; i++)
    {        
        bottomCount += histogram[i];
        if (bottomCount > bottomPixel) 
        {
            bottomIndex = i;
            break; 
        }
    }

    ceilingIndex = 255;
    for (i = 255; i > 0; i--)
    {        
        ceilingCount += histogram[i];
        if (ceilingCount > ceilingPixel) 
        {
            ceilingIndex = i;
            break; 
        }
    }

    multiplicationFactor = ((double) 255) / (ceilingIndex - bottomIndex);
    
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++ )
        {
            if (inputData[i*width+j] < bottomIndex)
            {
                outputData[i*width+j] = 0;
            } 
            else if (inputData[i*width+j] > ceilingIndex)
            {
                outputData[i*width+j] = 255;
            }
            else
            {
                if (floor(((double)inputData[i*width+j] - (double)bottomIndex) * multiplicationFactor) > 255) 
                {
                    outputData[i*width+j] = 255;
                }
                else
                {
                    outputData[i*width+j] = (uint8_t) floor(((double)inputData[i*width+j] - (double)bottomIndex) * multiplicationFactor);
                }                
            }
        }
    }
    GDOS_DBG_INFO("Lower bound:%i upper bound:%i multFactor:%f\n", bottomIndex, ceilingIndex,  multiplicationFactor);    
    return 0;
}


int CameraTool::histogramStrechMono12(unsigned short* outputData, unsigned short* inputData,
                                int width, int height, int bottomPercentage, int ceilingPercentage)
{
    int i,j;
    int bottomPixel, ceilingPixel;
    int bottomCount, ceilingCount;
    int bottomIndex, ceilingIndex; 
    double multiplicationFactor; 
    int histogram[4096];
    
    memset(histogram, 0, 4096 * sizeof(int));

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++ )
        {
            if (inputData[i*width+j] > 4095) 
            {
                GDOS_ERROR("Input[i:%i-j:%i]=%i value for 12 bit histogram > 4095\n",i, j, inputData[i*width+j]);
                
            }else 
            {
                histogram[inputData[i*width+j]]++;
            }
        }
    }
    bottomPixel  = (int) floor(bottomPercentage  * width * height / 100);
    ceilingPixel = (int) floor(ceilingPercentage * width * height / 100);

    bottomCount = 0; 
    ceilingCount = 0; 
    
    bottomIndex = 0;
    for (i = 0; i < 4096; i++)
    {        
        bottomCount += histogram[i];
        if (bottomCount > bottomPixel) 
        {
            bottomIndex = i;
            break; 
        }
    }

    ceilingIndex = 4095;
    for (i = 4095; i > 0; i--)
    {        
        ceilingCount += histogram[i];
        if (ceilingCount > ceilingPixel) 
        {
            ceilingIndex = i;
            break; 
        }
    }

    multiplicationFactor = ((double) 4095) / (ceilingIndex - bottomIndex);
    
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++ )
        {
            if (inputData[i*width+j] < bottomIndex)
            {
                outputData[i*width+j] = 0;
            } 
            else if (inputData[i*width+j] > ceilingIndex)
            {
                outputData[i*width+j] = 4095;
            }
            else
            {
                if (floor(((double)inputData[i*width+j] - (double)bottomIndex) * multiplicationFactor) > 4095) 
                {
                    outputData[i*width+j] = 4095;
                }
                else
                {
                    outputData[i*width+j] = (unsigned short) floor(((double)inputData[i*width+j] - (double)bottomIndex) * multiplicationFactor);
                }                
            }
        }
    }
    GDOS_DBG_INFO("Lower bound:%i upper bound:%i multFactor:%f\n", bottomIndex, ceilingIndex,  multiplicationFactor);    
    return 0;
}

int CameraTool::histogramStrechToMono8(uint8_t *outputData, unsigned short *inputData,
                                int width, int height, int bottomPercentage, int ceilingPercentage,
                                int topMargin, int bottomMargin, int leftMargin, int rightMargin)
{
    int i,j;
    int bottomPixel, ceilingPixel;
    int bottomCount, ceilingCount;
    int bottomIndex, ceilingIndex; 
    double multiplicationFactor; 
    int histogram[4096];
    
    memset(histogram, 0, 4096 * sizeof(int));

    for (i = topMargin; i < height - bottomMargin; i++)
    {
        for (j = leftMargin; j < width - rightMargin; j++ )
        {
            histogram[inputData[i*width+j]]++;
        }
    }
    bottomPixel  = (int) floor(bottomPercentage  * width * height / 100);
    ceilingPixel = (int) floor(ceilingPercentage * width * height / 100);

    bottomCount = 0; 
    ceilingCount = 0; 
    
    bottomIndex = 0;
    for (i = 0; i < 4096; i++)
    {        
        bottomCount += histogram[i];
        if (bottomCount > bottomPixel) 
        {
            bottomIndex = i;
            break; 
        }
    }

    ceilingIndex = 4095;
    for (i = 4095; i > 0; i--)
    {        
        ceilingCount += histogram[i];
        if (ceilingCount > ceilingPixel) 
        {
            ceilingIndex = i;
            break; 
        }
    }

    multiplicationFactor = ((double) 255) / (ceilingIndex - bottomIndex);
    
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++ )
        {
            if (inputData[i*width+j] < bottomIndex)
            {
                outputData[i*width+j] = 0;
            } 
            else if (inputData[i*width+j] > ceilingIndex)
            {
                outputData[i*width+j] = 255;
            }
            else
            {
                if (floor(((double)inputData[i*width+j] - (double)bottomIndex) * multiplicationFactor) > 255) 
                {
                    outputData[i*width+j] = 255;
                }
                else
                {
                    outputData[i*width+j] = (uint8_t) floor(((double)inputData[i*width+j] - (double)bottomIndex) * multiplicationFactor);
                }                
            }
        }
    }
    GDOS_DBG_INFO("Lower bound:%i upper bound:%i multFactor:%f\n", bottomIndex, ceilingIndex,  multiplicationFactor);    
    return 0;
}
/*
int CameraTool::ctMedianFilter8Bit(uint8_t* outputData, uint8_t* inputData, int width, int height, int radius, int channels, int maxMemUsage)
{
//    void ctmf(
//        const unsigned char* src, unsigned char* dst,
//        int width, int height,
//        int src_step_row, int dst_step_row,
//        int r, int channels, unsigned long memsize
//        );
    ctmf(inputData, outputData, width, height, width, width, radius, channels, maxMemUsage);
    return 0; 
}*/

int CameraTool::qsMedianFilter16Bit(short *outputData, short *inputData, int width, int height, int radius)
{
    int i,j, k, l;
    int minIndex;
    short  swapValue; 
    short  arr[radius];
    
    memset(outputData, 0, width * height * sizeof(short));
    GDOS_DBG_INFO("doing Median filtering with radius:%i\n", radius);
    
    for (i = 0; i < height; i++)
    {
        for (j = radius/2; j < width-radius/2; j++ )
        {
            memcpy(arr, &(inputData[i*width+j-radius/2]), radius*sizeof(short));  
    
            for (k=0; k <= radius/2; k++)
            {
                minIndex  = k;
                swapValue = arr[k];
                
                for (l = k+1; l < radius; l++)
                {
                    if (arr[l] < arr[minIndex]) {minIndex = l;}    
                }
        
                arr[k] = arr[minIndex]; 
                arr[minIndex] = swapValue;
            }
            outputData[i*width+j] = arr[radius/2];
       }
        
        //correct first and last pixel per row
        for (j = 0; j < radius/2; j++ )
        {
            outputData[i*width+j] = outputData[i*width+radius/2];
            outputData[i*width+j+width-radius/2] = outputData[i*width+width-radius/2-1];
        }
    }

    return 0;
}

int CameraTool::lowPassFilter16Bit(short *outputData, short *inputData, int width, int height, int radius)
{

    int i,j, k, l;
    int numElem, sideLength;
    double sum;
    
    if (radius == 0) 
    {
        memcpy(outputData, inputData, sizeof(short) * width * height);
        return 0; 
    }
    
    GDOS_DBG_INFO("doing low pass filtering with radius:%i\n", radius);
    
    sideLength = 2 * radius + 1;
    numElem    = sideLength * sideLength;
        
    memset(outputData, 0, width * height * sizeof(short));
    
    for (i = radius; i < height-radius; i++)
    {
        for (j = radius; j < width-radius; j++ )
        {
            sum = 0; 
            for (k=0; k < sideLength; k++)
            {
                for (l=0; l < sideLength; l++)
                {
                    sum += inputData[(i-radius+k)*width + j - radius + l];
                }
            }
            outputData[i*width+j] = (short) floor(sum / numElem);
        }
    
        //error correction
        for (j = 0; j < radius; j++ )
        {
            outputData[i*width+j] = outputData[i*width+radius];
            outputData[i*width+j+width-radius] = outputData[i*width+width-radius-1];
        }
    }
    
    //error correction
    for (i=0; i < radius; i++)
    {
        for (j = 0; j < width; j++ )
        {
            outputData[i*width+j] = outputData[radius*width+j];//first row dublicated
            outputData[(height-i-1)*width+j] = outputData[(height-radius-1)*width+j];//last row dublicated
        }
    }
    return 0;
}
