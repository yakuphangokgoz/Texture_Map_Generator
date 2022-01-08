#include "normalmap.h"
#include "blurvalues.h"

//QImage grayscale_image;
QImage normalmap_image;
QImage bordered_image;

//QColor currentPixelColor(0,0,0,1);
QColor newColor;

QVector3D normal_vector(0,0,1);
QVector3D normalized_normal_vector(0,0,1);



NormalMap::NormalMap()
{
    current_strength = 8;
    current_exponent = 1;
    isInvertRed = 0;
    isInvertGreen = 0;
    isInvertHeight = 0;
    current_radius = 1;
    SetupAccelerator();
}

NormalMap::NormalMap(QImage imageToProcess)
{
    current_strength = 8;
    current_exponent = 1;
    isInvertRed = 0;
    isInvertGreen = 0;
    isInvertHeight = 0;
    current_radius = 1;
    grayscale_image = imageToProcess;
    SetupAccelerator();
}

QImage NormalMap::GenerateBorderedCopy(QImage imageToProcess)
{
    int width = imageToProcess.width();
    int height = imageToProcess.height();
    bordered_image = QImage(width+2, height+2, imageToProcess.format());
    for(int i = 0; i<width+1; i++){
        for(int j = 0; j<height+1; j++){

            if(i == 0 || j == 0 || i == width || j == height){
                bordered_image.setPixelColor(j,i, QColor(255,255,255,255));
            }
            else{
                bordered_image.setPixelColor(j,i,imageToProcess.pixelColor(j,i));
            }
        }
    }
    bordered_image.save("bordered.jpg");
    return bordered_image;
}

float NormalMap::FindNormalComponent(QString axis, int x_pos, int y_pos, float strength)
{
    if(axis == "x"){
        return  strength * -(bordered_image.pixelColor(y_pos-1,x_pos+1).red() - bordered_image.pixelColor(y_pos-1,x_pos-1).red() +
                 2*(bordered_image.pixelColor(y_pos,x_pos+1).red() - bordered_image.pixelColor(y_pos,x_pos-1).red()) +
                 bordered_image.pixelColor(y_pos+1,x_pos+1).red() - bordered_image.pixelColor(y_pos+1,x_pos-1).red())/256.0f;
    }

    else if(axis == "y"){
        return  strength * -(bordered_image.pixelColor(y_pos-1,x_pos-1).red() - bordered_image.pixelColor(y_pos+1,x_pos-1).red() +
                 2*(bordered_image.pixelColor(y_pos-1,x_pos).red() - bordered_image.pixelColor(y_pos+1,x_pos).red()) +
                 bordered_image.pixelColor(y_pos-1,x_pos+1).red() - bordered_image.pixelColor(y_pos+1,x_pos+1).red())/256.0f;

    }
    return NULL;
}

void NormalMap::ResetAttributes(Ui::MainWindow *ui){
       current_strength = DEFAULT_STRENGTH;
       ui->strength_slider->setValue(DEFAULT_STRENGTH);
       current_exponent = DEFAULT_EXPONENT;
       ui->exponent_slider->setValue(DEFAULT_EXPONENT);
       current_radius = DEFAULT_BLUR;
       ui->normalmap_blur_slider->setValue(DEFAULT_BLUR);
}

void NormalMap::SetupAccelerator()
{
    if (!context.create(QCLDevice::GPU)){

        qFatal("Could not create OpenCL context");
    }

    program = context.buildProgramFromSourceFile(":/img/manipulation.cl");
    weightsBuffer = context.createVector<float>(100);
    offsetsBuffer = context.createVector<float>(100);

    normalMapKernel = program.createKernel("normalMap");
    hgaussian = program.createKernel("hgaussian");
    vgaussian = program.createKernel("vgaussian");

}


QImage NormalMap::AcceleratedProcess(float strength, float exponent, int isInvertRed, int isInvertGreen, int isInvertHeight)
{

    srcImage = grayscale_image;

    normalMapKernel.setGlobalWorkSize(srcImage.size()); //image.size()
    normalMapKernel.setLocalWorkSize(hgaussian.bestLocalWorkSizeImage2D()); // 8 x 8 is optimal setting
    hgaussian.setGlobalWorkSize(srcImage.size());
    hgaussian.setLocalWorkSize(hgaussian.bestLocalWorkSizeImage2D());
    vgaussian.setGlobalWorkSize(srcImage.size());
    vgaussian.setLocalWorkSize(vgaussian.bestLocalWorkSizeImage2D());

    offsetsBuffer.write(blurOffsets[current_radius], blurSizes[current_radius]);
    weightsBuffer.write(blurWeights[current_radius], blurSizes[current_radius]);

    srcImageBuffer = context.createImage2DCopy(srcImage, QCLMemoryObject::ReadOnly);
    tmpImageBuffer1 = context.createImage2DDevice(QImage::Format_ARGB32, srcImage.size(), QCLMemoryObject::ReadWrite);
    tmpImageBuffer2 = context.createImage2DDevice(QImage::Format_ARGB32, srcImage.size(), QCLMemoryObject::ReadWrite);

    dstImage = QImage(srcImage.size(), QImage::Format_ARGB32);
    dstImageBuffer = context.createImage2DDevice(dstImage.format(), dstImage.size(), QCLMemoryObject::WriteOnly);

    if(current_radius > 0){
        event_normal_map = normalMapKernel(srcImageBuffer, tmpImageBuffer1, strength, exponent, isInvertRed, isInvertGreen, isInvertHeight);
        event_normal_map.waitForFinished();
        event_hgaussian = hgaussian(tmpImageBuffer1, tmpImageBuffer2, weightsBuffer, offsetsBuffer, blurSizes[current_radius]);
        event_hgaussian.waitForFinished();
        event_vgaussian = vgaussian(tmpImageBuffer2, dstImageBuffer, weightsBuffer, offsetsBuffer, blurSizes[current_radius]);
        event_vgaussian.waitForFinished();
    }
    else{
        event_normal_map = normalMapKernel(srcImageBuffer, dstImageBuffer, strength, exponent, isInvertRed, isInvertGreen, isInvertHeight);
        event_normal_map.waitForFinished();
    }

    dstImageBuffer.read(&dstImage);
    normalmap_image = dstImage;
    return dstImage;
}

void NormalMap::GenerateNormalMap(QImage imageToProcess, Ui::MainWindow *ui, float strength, float exponent, int invertRed, int invertGreen, int invertHeight)
{
    timer.start();
    //bordered_image = GenerateBorderedCopy(imageToProcess);
    //normalmap_image = imageToProcess;
    current_strength = strength;
    current_exponent = exponent;
    grayscale_image = imageToProcess;
    isInvertRed = invertRed;
    isInvertGreen = invertGreen;
    isInvertHeight = invertHeight;
    /* for(int i = 1; i<bordered_image.width()-2; i++){
        for(int j = 1; j<bordered_image.height()-2; j++){

            normal_vector.setX(FindNormalComponent("x", i, j, strength));
            normal_vector.setY(FindNormalComponent("y", i, j, strength));
            normal_vector.setZ(1);
            normalized_normal_vector = normal_vector.normalized();
            normalized_normal_vector = normalized_normal_vector*0.5f + QVector3D(0.5f, 0.5f, 0.5f);
            //qDebug() << normal_vector << "  ---  " << normalized_normal_vector;
            normalmap_image.setPixelColor(j,i, QColor( (int) floor(normalized_normal_vector.x()*255), (int) floor(normalized_normal_vector.y()*255), (int) floor(normalized_normal_vector.z()*255), 255  ));



        }
    }*/


    normalmap_image = AcceleratedProcess(strength, exponent, isInvertRed, isInvertGreen, isInvertHeight);
    qDebug() << "Ellapsed time is " << timer.elapsed();
    QPixmap processedPic = QPixmap(normalmap_image.width(), normalmap_image.height());
    processedPic.convertFromImage(normalmap_image);
    ui->label_processed_pic->setPixmap(processedPic.scaled(ui->label_processed_pic->width(), ui->label_processed_pic->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

}


