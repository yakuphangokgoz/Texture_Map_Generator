#ifndef NORMALMAP_H
#define NORMALMAP_H
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include <QImage>
#include <QElapsedTimer>
#include <QDebug>
#include <QColor>
#include <QVector3D>
#include <math.h>
#include <QtGui>
#include "qclcontext.h"
#include "qclprogram.h"
#include "qclkernel.h"
#include "qclimage.h"
#include "qclevent.h"


#define DEFAULT_STRENGTH 8
#define DEFAULT_EXPONENT 5
#define DEFAULT_BLUR 0

class NormalMap
{
public:

    QElapsedTimer timer;
    QImage grayscale_image;
    QColor currentPixelColor;
    QColor newColor;
    float current_strength;
    float current_exponent;
    int isInvertRed;
    int isInvertGreen;
    int isInvertHeight;

    QCLContext context;
    QCLProgram program;
    QCLKernel normalMapKernel;
    QCLKernel hgaussian;
    QCLKernel vgaussian;
    QImage srcImage;
    QImage dstImage;
    QCLImage2D srcImageBuffer;
    QCLImage2D tmpImageBuffer1;
    QCLImage2D tmpImageBuffer2;
    QCLImage2D dstImageBuffer;
    QColor factor_color;
    QCLVector<float> weightsBuffer;
    QCLVector<float> offsetsBuffer;
    int current_radius;

    QCLEvent event_normal_map;
    QCLEvent event_hgaussian;
    QCLEvent event_vgaussian;

    NormalMap();
    NormalMap(QImage imageToProcess);
    void ResetAttributes(Ui::MainWindow *ui);
    void GenerateNormalMap(QImage imageToProcess, Ui::MainWindow *ui, float strength = 8, float exponent = 1, int isInvertRed = 0, int isInvertGreen = 0, int isInvertHeight = 0);
    QImage GenerateBorderedCopy(QImage imageToProcess);
    float FindNormalComponent(QString axis, int x_pos, int y_pos, float strength);

    void SetupAccelerator();
    QImage AcceleratedProcess(float strength, float exponent, int isInvertRed, int isInvertGreen, int isInvertHeight);
};

#endif // NORMALMAP_H
