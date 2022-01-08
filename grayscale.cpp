#include "grayscale.h"
#include "blurvalues.h"

Grayscale::Grayscale(){
    factor_color = QColor(255, 255, 255, 255);
    SetupAccelerator();
}
Grayscale::Grayscale(QImage imageToProcess)
{
    normalized_brightness = 0;
    normalization_factor = 20.0;
    grayscale = 0;
    current_brightness = 0;
    current_contrast = 0;
    correction_factor = 0;
    isContrastDirty = true;
    isInvert = false;
    isInvertInt = 0;
    factor_color = QColor(255, 255, 255, 255);
    valueMap = QMap<int, int>();
    for(int i = 0; i < 256; i++){
        valueMap[i] = 0;
    }
    current_luminance = 0;
    //original_image = imageToProcess;
    SetupAccelerator();
}

float Grayscale::Truncate(float suspect){
    if(suspect < 0){ suspect = 0;}
    if(suspect > 255){ suspect = 255;}
    return suspect;
}

void Grayscale::ResetAttributes(Ui::MainWindow *ui){
       current_brightness = DEFAULT_BRIGHTNESS;
       ui->brightness_slider->setValue(DEFAULT_BRIGHTNESS);
       current_contrast = DEFAULT_CONTRAST;
       ui->contrast_slider->setValue(DEFAULT_CONTRAST);
       current_radius = DEFAULT_BLUR;
       ui->displacement_blur_slider->setValue(DEFAULT_BLUR);
}

void Grayscale::SetupAccelerator()
{
    if (!context.create(QCLDevice::GPU)){

        qFatal("Could not create OpenCL context");
    }
//    qDebug() << context.commandQueue();
    program = context.buildProgramFromSourceFile(":/img/manipulation.cl");

    weightsBuffer = context.createVector<float>(100);
    offsetsBuffer = context.createVector<float>(100);
    grayscaleKernel = program.createKernel("grayscale");
    hgaussian = program.createKernel("hgaussian");
    vgaussian = program.createKernel("vgaussian");
    legacyContrast = program.createKernel("legacyContrast");

    histogramMap = context.createVector<int>(260);

}

int zero = 0;
int current = 0;
float sum = 0;
float average = 0;
float coefficient = 0;
float size = 0;

QImage Grayscale::AcceleratedProcess(int brightness, int contrast, int isInvertInteger)
{

    srcImage = original_image;

    grayscaleKernel.setGlobalWorkSize(srcImage.size()); //image.size()
    grayscaleKernel.setLocalWorkSize(1, 1); // 8 x 8 is optimal setting
    hgaussian.setGlobalWorkSize(srcImage.size());
    hgaussian.setLocalWorkSize(hgaussian.bestLocalWorkSizeImage2D());
    vgaussian.setGlobalWorkSize(srcImage.size());
    vgaussian.setLocalWorkSize(vgaussian.bestLocalWorkSizeImage2D());
    legacyContrast.setGlobalWorkSize(srcImage.size());
    legacyContrast.setLocalWorkSize(1,1);



    offsetsBuffer.write(blurOffsets[current_radius], blurSizes[current_radius]);
    weightsBuffer.write(blurWeights[current_radius], blurSizes[current_radius]);

    for(int i=0; i< 256; i++){
        histogramMap.write(&zero, 1, i);
    }

    size = (srcImage.width() * srcImage.height());

    srcImageBuffer = context.createImage2DCopy(srcImage, QCLMemoryObject::ReadOnly);
    tmpImageBuffer1 = context.createImage2DDevice(QImage::Format_ARGB32, srcImage.size(), QCLMemoryObject::ReadWrite);
    tmpImageBuffer2 = context.createImage2DDevice(QImage::Format_ARGB32, srcImage.size(), QCLMemoryObject::ReadWrite);
    tmpImageBuffer3 = context.createImage2DDevice(QImage::Format_ARGB32, srcImage.size(), QCLMemoryObject::ReadWrite);
    tmpImageBuffer4 = context.createImage2DDevice(QImage::Format_ARGB32, srcImage.size(), QCLMemoryObject::ReadWrite);
    tmpImageBuffer5 = context.createImage2DDevice(QImage::Format_ARGB32, srcImage.size(), QCLMemoryObject::ReadWrite);

    dstImage = QImage(srcImage.size(), QImage::Format_ARGB32);
    dstImageBuffer = context.createImage2DDevice(dstImage.format(), dstImage.size(), QCLMemoryObject::WriteOnly);

    if(current_radius >0){


        event_grayscale_map = grayscaleKernel(srcImageBuffer, tmpImageBuffer1, current_luminance, brightness, isInvertInteger, histogramMap);
        event_grayscale_map.waitForFinished();

        sum = 0;
        for(int i=0; i< 256; i++){
            histogramMap.read(&current, 1, i);
            sum += (current)*i;
        }

        average = sum / size;

        event_hgaussian = hgaussian(tmpImageBuffer1, tmpImageBuffer2, weightsBuffer, offsetsBuffer, blurSizes[current_radius]);
        event_hgaussian.waitForFinished();
        event_vgaussian = vgaussian(tmpImageBuffer2, tmpImageBuffer3, weightsBuffer, offsetsBuffer, blurSizes[current_radius]);
        event_vgaussian.waitForFinished();
        event_legacyContrast = legacyContrast(tmpImageBuffer3, dstImageBuffer, average/255, (float)contrast/50);
        event_legacyContrast.waitForFinished();




    }
    else{
        event_grayscale_map = grayscaleKernel(srcImageBuffer, dstImageBuffer, current_luminance, brightness, isInvertInteger, histogramMap);
        event_grayscale_map.waitForFinished();
    }


    if(dstImageBuffer.isNull()){
        qDebug() <<"Buffer null";
    }
    dstImageBuffer.read(&dstImage);
    //grayscale_image = dstImage;
    return dstImage;
}


Grayscale *Grayscale::operator=(Grayscale* other)
{
    this->original_image = other->original_image;
    return this;
}



float Grayscale::CalculateCorrectionFactor(float contrast){

    return (259*(contrast + 255)) / (255*(259 - contrast));
}

float Grayscale::CalculateContrast(float color_value){

        return Truncate(correction_factor*(color_value - 128) + 128);
}

void Grayscale::GenerateGrayScaleMap(QImage imageToProcess, Ui::MainWindow *ui, int brightness, int contrast, bool invert){

    timer.start();

    if(brightness != current_brightness){
        current_brightness = brightness;
    }
    if(contrast != current_contrast){
        current_contrast = contrast;
        isContrastDirty = true;
    }

    if(invert != isInvert){
        isInvert = invert;
    }

    isInvert = invert ? invert : isInvert;

    isInvertInt = invert ? 1 : 0;

    normalized_brightness = brightness/normalization_factor;
    if(isContrastDirty){
        correction_factor = CalculateCorrectionFactor(contrast*2.55);
        isContrastDirty = false;
    }
    /*
    grayscale_image = imageToProcess;

    for(int i=0; i < imageToProcess.width(); i++){
        for(int j=0; j < imageToProcess.height(); j++){
            currentPixelColor = imageToProcess.pixelColor(j,i);
            if(invert){
                grayscale = 255.0-(CalculateContrast(currentPixelColor.red())*0.299 + CalculateContrast(currentPixelColor.green())*0.587 + CalculateContrast(currentPixelColor.blue())*0.114);
                //qDebug() << "Inverted";
            }
            else{
                grayscale = CalculateContrast(currentPixelColor.red())*0.299 + CalculateContrast(currentPixelColor.green())*0.587 + CalculateContrast(currentPixelColor.blue())*0.114;
            }
            grayscale = Truncate(grayscale + grayscale*normalized_brightness);

            newPixelColor = QColor(grayscale, grayscale, grayscale, currentPixelColor.alpha());
            grayscale_image.setPixelColor(j, i, newPixelColor);

        }

    }
*/
    grayscale_image = AcceleratedProcess(brightness, contrast*2.55, isInvertInt);
    //qDebug() << "Ellapsed time is " << timer.elapsed();
    QPixmap processedPic = QPixmap(ui->label_processed_pic->width(), ui->label_processed_pic->height());
    processedPic.convertFromImage(grayscale_image);
    ui->label_processed_pic->setPixmap(processedPic.scaled(ui->label_processed_pic->width(), ui->label_processed_pic->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    //grayscale_image.save("sample_grayscale.jpg");


}

