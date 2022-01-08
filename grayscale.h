#ifndef GRAYSCALE_H
#define GRAYSCALE_H
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include <QImage>
#include <QElapsedTimer>
#include <QDebug>
#include <QColor>
#include "qclcontext.h"
#include "qclprogram.h"
#include "qclkernel.h"
#include "qclimage.h"
#include "qclplatform.h"
#include <QMap>
#include "histogram.h"

#define DEFAULT_BRIGHTNESS 0
#define DEFAULT_CONTRAST 0
#define DEFAULT_BLUR 0

class Grayscale
{
public:
    QImage original_image;
    QElapsedTimer timer;
    QImage grayscale_image;
    QColor currentPixelColor;
    float normalized_brightness;
    float normalization_factor;
    float normalized_contrast;
    float grayscale;
    QColor newPixelColor;
    int current_brightness;
    int current_contrast;
    float correction_factor;
    bool isContrastDirty;
    bool isInvert;
    int isInvertInt;


    QCLContext context;
    QCLProgram program;

    QCLKernel grayscaleKernel;
    QCLKernel hgaussian;
    QCLKernel vgaussian;
    QCLKernel histogram;
    QCLKernel legacyContrast;


    QImage srcImage;
    QImage dstImage;
    QCLImage2D srcImageBuffer;
    QCLImage2D tmpImageBuffer1;
    QCLImage2D tmpImageBuffer2;
    QCLImage2D tmpImageBuffer3;
    QCLImage2D tmpImageBuffer4;
    QCLImage2D tmpImageBuffer5;
    QCLImage2D dstImageBuffer;
    QColor factor_color;
    QCLVector<float> weightsBuffer;
    QCLVector<float> offsetsBuffer;
    int current_radius;
    int current_luminance; //0 = CIE, 1 = YCbCr

    QCLVector<int> histogramMap;
    QMap<int, int> valueMap;

    QCLEvent event_grayscale_map;
    QCLEvent event_hgaussian;
    QCLEvent event_vgaussian;
    QCLEvent event_histogram;
    QCLEvent event_legacyContrast;


    int pixelAmount;

    Grayscale();
    Grayscale(QImage imageToProcess);
    void ResetAttributes(Ui::MainWindow *ui);
    void GenerateGrayScaleMap(QImage imageToProcess, Ui::MainWindow *ui, int brightness = 0, int contrast = 0, bool invert = false);
    float CalculateCorrectionFactor(float contrast);
    float CalculateContrast(float color_value);
    float Truncate(float suspect);



    void SetupAccelerator();
    QImage AcceleratedProcess(int brightness = 0, int contrast = 0, int isInvertInteger = 0);
    Grayscale* operator=(Grayscale* other);
};

#endif // GRAYSCALE_H
