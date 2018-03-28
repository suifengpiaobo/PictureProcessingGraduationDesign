#include <jni.h>
#include <opencv2/core/cvdef.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include<vector>

using namespace cv;
using namespace std;
#include <string>
#include <iostream>

extern Mat RGBToLab(Mat &m);

#define CLIP_RANGE(value, min, max)  ( (value) > (max) ? (max) : (((value) < (min)) ? (min) : (value)) )
#define COLOR_RANGE(value)  CLIP_RANGE(value, 0, 255)
extern int adjustBrightnessContrast(InputArray src, OutputArray dst, int brightness, int contrast);
extern void adjustHSL(Mat& img, Mat& aImg, int  hue, int saturation, int lightness);
extern int imageCrop(InputArray src, OutputArray dst, Rect rect);
extern int cvAdd4cMat_q(cv::Mat &dst, cv::Mat &scr, double scale);

extern "C"
JNIEXPORT void JNICALL
Java_com_example_whensunset_pictureprocessinggraduationdesign_pictureProcessing_CutMyConsumer_cut(
        JNIEnv *env, jobject instance, jlong in_mat_addr, jlong out_mat_addr, jint x, jint y,
        jint width, jint height) {

    Mat& oldMat = *(Mat *) in_mat_addr;
    Mat& newMat = *(Mat *) out_mat_addr;
    Rect rect(x , y , width , height);
    imageCrop(oldMat , newMat , rect);
    return;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_whensunset_pictureprocessinggraduationdesign_pictureProcessing_FlipMyConsumer_flip(
        JNIEnv *env, jclass type, jlong src_nativeObj, jlong dst_nativeObj, jint flipCode) {
    Mat& oldMat = *(Mat *) src_nativeObj;
    Mat& newMat = *(Mat *) dst_nativeObj;

    flip(oldMat , newMat , flipCode);
    return;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_whensunset_pictureprocessinggraduationdesign_pictureProcessing_RotateMyConsumer_rotate(
        JNIEnv *env, jobject instance, jlong in_mat_addr, jlong out_mat_addr, jdouble angle,
        jdouble scale) {

    Mat& oldMat = *(Mat *) in_mat_addr;
    Mat& newMat = *(Mat *) out_mat_addr;

    Point2f center(oldMat.cols / 2, oldMat.rows / 2);
    Mat rot = getRotationMatrix2D(center, angle, scale);

    Rect bbox = RotatedRect(center, oldMat.size(), angle).boundingRect();

    rot.at<double>(0, 2) += bbox.width / 2.0 - center.x;
    rot.at<double>(1, 2) += bbox.height / 2.0 - center.y;

    warpAffine(oldMat , newMat , rot, bbox.size());
    return;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_whensunset_pictureprocessinggraduationdesign_pictureProcessing_WhiteBalanceMyConsumer_whiteBalance(
        JNIEnv *env, jobject instance, jlong in_mat_addr, jlong out_mat_addr) {

    Mat& oldMat = *(Mat *) in_mat_addr;
    Mat& newMat = *(Mat *) out_mat_addr;
    vector<Mat> imageRGB;

    //RGB三通道分离
    split(oldMat, imageRGB);

    //求原始图像的RGB分量的均值
    double R, G, B;
    B = mean(imageRGB[0])[0];
    G = mean(imageRGB[1])[0];
    R = mean(imageRGB[2])[0];

    //需要调整的RGB分量的增益
    double KR, KG, KB;
    KB = (R + G + B) / (3 * B);
    KG = (R + G + B) / (3 * G);
    KR = (R + G + B) / (3 * R);

    //调整RGB三个通道各自的值
    imageRGB[0] = imageRGB[0] * KB;
    imageRGB[1] = imageRGB[1] * KG;
    imageRGB[2] = imageRGB[2] * KR;

    //RGB三通道图像合并
    merge(imageRGB, newMat);
    return;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_whensunset_pictureprocessinggraduationdesign_pictureProcessing_PictureParamMyConsumer_pictureParamChange(
        JNIEnv *env, jobject instance, jlong in_mat_addr, jlong out_mat_addr, jint brightness,
        jint contrast, jint saturation, jint tonal) {
    Mat& oldMat = *(Mat *) in_mat_addr;
    Mat middleMat;
    Mat& newMat = *(Mat *) out_mat_addr;

    adjustBrightnessContrast(oldMat , middleMat , brightness , contrast);
    adjustHSL(middleMat , newMat , tonal , saturation , 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_whensunset_pictureprocessinggraduationdesign_pictureProcessing_PictureFrameMyConsumer_mixed(
        JNIEnv *env, jobject instance, jlong in_mat_addr, jlong insert_mat_addr , jlong out_mat_addr , jint x, jint y,
        jint width, jint height) {
    Mat& oldMat = *(Mat *) in_mat_addr;
    Mat& insertMat = *(Mat *) insert_mat_addr;
    Mat middleMat = Mat::zeros(height , width , CV_8UC3);
    Mat& newMat = *(Mat *) out_mat_addr;

    oldMat.copyTo(newMat);
    resize(insertMat , middleMat , middleMat.size() , 0 , 0 , INTER_LINEAR);

    Mat ROI = newMat(Rect(x , y , middleMat.cols , middleMat.rows));

    if(middleMat.channels() == 3) {
        middleMat.copyTo(ROI);
    } else if(middleMat.channels() == 4) {
        cvAdd4cMat_q(ROI , middleMat , 1.0);
    }
}

int cvAdd4cMat_q(cv::Mat &dst, cv::Mat &scr, double scale)
{
    if (dst.channels() != 3 || scr.channels() != 4)
    {
        return true;
    }
    if (scale < 0.01)
        return false;
    std::vector<cv::Mat>scr_channels;
    std::vector<cv::Mat>dstt_channels;
    split(scr, scr_channels);
    split(dst, dstt_channels);
    CV_Assert(scr_channels.size() == 4 && dstt_channels.size() == 3);

    if (scale < 1)
    {
        scr_channels[3] *= scale;
        scale = 1;
    }
    for (int i = 0; i < 3; i++)
    {
        dstt_channels[i] = dstt_channels[i].mul(255.0 / scale - scr_channels[3], scale / 255.0);
        dstt_channels[i] += scr_channels[i].mul(scr_channels[3], scale / 255.0);
    }
    merge(dstt_channels, dst);
    return true;
}

int imageCrop(InputArray src, OutputArray dst, Rect rect)
{
    Mat input = src.getMat();
    if( input.empty() ) {


        return -1;
    }

    //计算剪切区域：  剪切Rect与源图像所在Rect的交集
    Rect srcRect(0, 0, input.cols, input.rows);
    rect = rect & srcRect;
    if ( rect.width <= 0  || rect.height <= 0 ) return -2;

    //创建结果图像
    dst.create(Size(rect.width, rect.height), src.type());
    Mat output = dst.getMat();
    if ( output.empty() ) return -1;

    try {
        //复制源图像的剪切区域 到结果图像
        input(rect).copyTo( output );
        return 0;
    } catch (...) {
        return -3;
    }
}

/**
 * Adjust Brightness and Contrast
 *
 * @param src [in] InputArray
 * @param dst [out] OutputArray
 * @param brightness [in] integer, value range [-255, 255]
 * @param contrast [in] integer, value range [-255, 255]
 *
 * @return 0 if success, else return error code
 */
int adjustBrightnessContrast(InputArray src, OutputArray dst, int brightness, int contrast)
{
    Mat input = src.getMat();
    if( input.empty() ) {
        return -1;
    }

    dst.create(src.size(), src.type());
    Mat output = dst.getMat();

    brightness = CLIP_RANGE(brightness, -255, 255);
    contrast = CLIP_RANGE(contrast, -255, 255);

    /**
    Algorithm of Brightness Contrast transformation
    The formula is:
        y = [x - 127.5 * (1 - B)] * k + 127.5 * (1 + B);

        x is the input pixel value
        y is the output pixel value
        B is brightness, value range is [-1,1]
        k is used to adjust contrast
            k = tan( (45 + 44 * c) / 180 * PI );
            c is contrast, value range is [-1,1]
    */

    double B = brightness / 255.;
    double c = contrast / 255. ;
    double k = tan( (45 + 44 * c) / 180 * M_PI );

    Mat lookupTable(1, 256, CV_8U);
    uchar *p = lookupTable.data;
    for (int i = 0; i < 256; i++)
        p[i] = COLOR_RANGE( (i - 127.5 * (1 - B)) * k + 127.5 * (1 + B) );

    LUT(input, lookupTable, output);



    return 0;
}

void adjustHSL(Mat& img, Mat& aImg, int  hue, int saturation, int lightness)
{
    if ( aImg.empty())
        aImg.create(img.rows, img.cols, img.type());

    Mat temp;
    temp.create(img.rows, img.cols, img.type());

    cvtColor(img, temp, CV_RGB2HSV);

    int i, j;
    Size size = img.size();
    int chns = img.channels();

    if (temp.isContinuous())
    {
        size.width *= size.height;
        size.height = 1;
    }

    // 验证参数范围
    if ( hue<-180 )
        hue = -180;

    if ( saturation<-255)
        saturation = -255;

    if ( lightness<-255 )
        lightness = -255;

    if ( hue>180)
        hue = 180;

    if ( saturation>255)
        saturation = 255;

    if ( lightness>255)
        lightness = 255;


    for (  i= 0; i<size.height; ++i)
    {
        unsigned char* src = static_cast<unsigned char*>(temp.data+temp.step*i);
        for (  j=0; j<size.width; ++j)
        {
            float val = src[j*chns]+hue;
            if ( val < 0) val = 0.0;
            if ( val > 180 ) val = 180;
            src[j*chns] = val;

            val = src[j*chns+1] + saturation;
            if ( val < 0) val = 0;
            if ( val > 255 ) val = 255;
            src[j*chns+1] = val;

            val = src[j*chns+2] + lightness;
            if ( val < 0) val = 0;
            if ( val > 255 ) val = 255;
            src[j*chns+2] = val;
        }
    }

    cvtColor(temp, aImg, CV_HSV2RGB);
    if ( temp.empty())
        temp.release();

}

void resize(Mat& in , Mat& out , int width , int height) {
}


