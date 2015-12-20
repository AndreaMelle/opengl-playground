#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <limits.h>

#define NUM_POINTS 512

void test_pack_2_float_1_uint()
{
    //we have 2 32-bit floats in [0, 1]
    //we want to pack them in one 32-bit unsigned integer
    //allocating 16-bit to each float
    
    float points[NUM_POINTS];
    unsigned int packed[NUM_POINTS / 2];
    
    float MAX_SHORT_F = 65535.0f;
    
    for(int i = 0; i < NUM_POINTS; ++i)
    {
        points[i] = (float)rand() / (float)RAND_MAX;
    }
    
    for(int i = 0; i < NUM_POINTS; i+=2)
    {
        int index = floorf(i / 2);
        packed[index]  = (unsigned int)(points[i + 0] * MAX_SHORT_F + 0.5f);
        packed[index] |= (unsigned int)(points[i + 1] * MAX_SHORT_F  + 0.5f) << 16;
    }
    
    float avg_error = 0;
    
    for(int i = 0; i < NUM_POINTS; i+=2)
    {
        int index = floorf(i / 2);
        
        // mask out all the stuff above 16-bit with 0xFFFF
        float a_ = (float)(packed[index] & 0xFFFF) / MAX_SHORT_F;
        float b_ = (float)(packed[index] >> 16) / MAX_SHORT_F;
        
        printf("%f %f \n", points[i + 0], a_);
        printf("%f %f \n", points[i + 1], b_);
        
        avg_error += sqrtf((points[i + 0] - a_) * (points[i + 0] - a_));
        avg_error += sqrtf((points[i + 1] - b_) * (points[i + 1] - b_));
    }
    
    avg_error /= (float)NUM_POINTS;
    
    printf("average error %f\n", avg_error);
}

union uif
{
    float f;
    unsigned int ui;
} uif;

void test_pack_2_float_1_uint_alt()
{
    union uif a, b;
    a.f = 0.05367f;
    b.f = 0.76352f;
    
    //printf("%u", a.ui);
    
    a.ui = a.ui >> 16;
    b.ui = b.ui >> 16;
    
    unsigned int packed = a.ui;
    packed |= b.ui << 16;
    
    a.ui = (packed & 0xFFFF);
    b.ui = (packed >> 16);
    
    a.ui = a.ui << 16;
    b.ui = b.ui << 16;
    
    printf("%f\n", a.f);
    printf("%f\n", b.f);
}

void test_pack_1_uint_2_ushort()
{
    unsigned int a = UINT_MAX - 3;
    
    unsigned short a_low = (unsigned short)(a & 0xFFFF);
    unsigned short a_high = (unsigned short)(a >> 16);
    
    unsigned int unpacked = ((unsigned int)a_high << 16) + (unsigned int)a_low;
    
    printf("%u\n%u\n", a, unpacked);
}

int main(int argc, char** argv)
{
    srand (time(NULL));
    
    //test_pack_2_float_1_uint();
    //test_pack_2_float_1_uint_alt();
    
    test_pack_1_uint_2_ushort();
    
    
    return 0;
}