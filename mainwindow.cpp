#include "mainwindow.h"



QString sampleImageName = "sample.jpg";
QString sampleImagePath = "";
QImage sample_image;


Grayscale grayscaleObject(sample_image);
NormalMap normalMapObject(sample_image);



QString tabBarStyleSheet = "QTabBar::tab { color: white; font-weight:bold; font-size: 10px; height: 50px; margin: 5px; border-radius:10px;"
"background-image: url(:/img/img/tab_background2.png);}"
"QTabBar::tab:selected { height: 50px; margin: 5px; border-radius:10px;"
"background-image: url(:/img/img/tab_background_pressed.png);}"
"QTabBar::tab:hover { height: 50px; margin: 5px; border-radius:10px;"
"background-image: url(:/img/img/tab_background_hover.png);}";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{


    sampleImagePath = QDir(QCoreApplication::applicationDirPath()).filePath(sampleImageName);
    sample_image = QImage(sampleImagePath);

    grayscaleObject.original_image = sample_image;



    QScreen *screen = QGuiApplication::primaryScreen();
    QRect  screenGeometry = screen->geometry();
    int screen_height = screenGeometry.height();
    int screen_width = screenGeometry.width();
    float resizeFactor = 50 * screen_width/screen_height;

    float window_width = screen_width - resizeFactor;
    float window_height = screen_height - resizeFactor;




    this->setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->setupUi(this);
    ui->label_brightness->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->label_contrast->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->brightness_slider->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->main_tab_widget->setAttribute(Qt::WA_TranslucentBackground, true);
    ui->chart_view->setAttribute(Qt::WA_TranslucentBackground, true);




    pixelHistogram = Histogram();

    this->setGeometry( (int)(screen_width/2 - window_width/2), int(screen_height/2 - window_height/2), (int)window_width, (int)window_height);
    ui->frame->setGeometry(0,0,window_width, window_height);
    ui->main_tab_widget->setGeometry((int)(window_width/2 - window_width/6.08),(int)(window_height/2 - window_height/2.56),(int)(window_width/3.04), (int)(window_height/1.28));
    ui->main_tab_widget->tabBar()->setExpanding(true);

    ui->main_tab_widget->tabBar()->setStyleSheet(tabBarStyleSheet);

    QPixmap original_sample_pic(sampleImagePath);
    int w = ui->label_original_pic->width();
    int h = ui->label_original_pic->height();
    ui->label_original_pic->setPixmap(original_sample_pic.scaled(w, h, Qt::KeepAspectRatio));
    grayscaleObject.GenerateGrayScaleMap(grayscaleObject.original_image, ui);
    grayscaleObject.grayscale_image.save("sample_grayscale.jpg");
    normalMapObject.grayscale_image = grayscaleObject.grayscale_image;
    pixelHistogram.SetChartData(grayscaleObject.histogramMap);

    histogramView = new QChartView(pixelHistogram.histogram);
    histogramView->setRenderHint(QPainter::Antialiasing);
    histogramView->setParent(ui->chart_view);
    histogramView->setGeometry(0, 0, ui->chart_view->geometry().width(), ui->chart_view->geometry().height());
    pixelHistogram.histogram->setMaximumWidth(ui->chart_view->geometry().width());
}

MainWindow::~MainWindow()
{
    delete ui;
}

Ui::MainWindow *MainWindow::GetUi()
{
    return ui;
}

void MainWindow::ResetTabValues(){
    grayscaleObject.ResetAttributes(ui);
    normalMapObject.ResetAttributes(ui);

}


void MainWindow::on_brightness_slider_valueChanged(int value)
{
    grayscaleObject.GenerateGrayScaleMap(sample_image, ui, value, grayscaleObject.current_contrast, grayscaleObject.isInvert);
    ui->label_brightness->setText("Brightness: " + QString::number(value));
    pixelHistogram.SetChartData(grayscaleObject.histogramMap);


}

void MainWindow::on_contrast_slider_valueChanged(int value)
{
    grayscaleObject.GenerateGrayScaleMap(sample_image, ui, grayscaleObject.current_brightness, value, grayscaleObject.isInvert);
    ui->label_contrast->setText("Contrast: " + QString::number(value));
    pixelHistogram.SetChartData(grayscaleObject.histogramMap);

}


void MainWindow::on_displacement_blur_slider_valueChanged(int value)
{
    grayscaleObject.current_radius = value;
    grayscaleObject.GenerateGrayScaleMap(sample_image, ui, grayscaleObject.current_brightness, grayscaleObject.current_contrast, grayscaleObject.isInvert);
    ui->label_displacement_blur->setText("Blur: " + QString::number(value));
    pixelHistogram.SetChartData(grayscaleObject.histogramMap);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{

    if(event->key() == Qt::Key_K)
    {
        QApplication::quit();
    }
    if(event->key() == Qt::Key_M)
    {
        pixelHistogram.histogram->zoomOut();
    }
    if(event->key() == Qt::Key_N)
    {
        pixelHistogram.histogram->zoomIn();
    }



}


void MainWindow::on_invert_displacement_checkbox_stateChanged(int arg1)
{
    if(arg1 == 0){
        grayscaleObject.GenerateGrayScaleMap(sample_image, ui, grayscaleObject.current_brightness, grayscaleObject.current_contrast, false);
    }
    else{
        grayscaleObject.GenerateGrayScaleMap(sample_image, ui, grayscaleObject.current_brightness, grayscaleObject.current_contrast, true);
    }
    pixelHistogram.SetChartData(grayscaleObject.histogramMap);
}

void MainWindow::on_main_tab_widget_currentChanged(int index)
{
    ResetTabValues();
    switch (index) {
        case 0:
            grayscaleObject.GenerateGrayScaleMap(grayscaleObject.original_image, ui);

            break;
        case 1:
            normalMapObject.GenerateNormalMap(grayscaleObject.grayscale_image, ui);

            break;
    }

}

void MainWindow::on_strength_slider_valueChanged(int value)
{
    normalMapObject.GenerateNormalMap(grayscaleObject.grayscale_image, ui, value, normalMapObject.current_exponent, normalMapObject.isInvertRed, normalMapObject.isInvertGreen, normalMapObject.isInvertHeight);
    ui->label_strength->setText("Strength: " + QString::number(value));
}

void MainWindow::on_exponent_slider_valueChanged(int value)
{
    float temp = value/5.0f;
    normalMapObject.GenerateNormalMap(grayscaleObject.grayscale_image, ui, normalMapObject.current_strength, temp, normalMapObject.isInvertRed, normalMapObject.isInvertGreen, normalMapObject.isInvertHeight);
    ui->label_exponent->setText("Exponent: " + QString::number(temp));
}

void MainWindow::on_normalmap_blur_slider_valueChanged(int value)
{
    normalMapObject.current_radius = value;
    normalMapObject.GenerateNormalMap(grayscaleObject.grayscale_image, ui, normalMapObject.current_strength, normalMapObject.current_exponent, normalMapObject.isInvertRed, normalMapObject.isInvertGreen, normalMapObject.isInvertHeight);
    ui->label_normalmap_blur->setText("Blur: " + QString::number(value));
}


void MainWindow::on_invert_red_checkBox_stateChanged(int arg1)
{
    if(arg1 == 0){
        normalMapObject.GenerateNormalMap(grayscaleObject.grayscale_image, ui, normalMapObject.current_strength, normalMapObject.current_exponent, 0, normalMapObject.isInvertGreen, normalMapObject.isInvertHeight);
    }
    else{
        normalMapObject.GenerateNormalMap(grayscaleObject.grayscale_image, ui, normalMapObject.current_strength, normalMapObject.current_exponent, 1, normalMapObject.isInvertGreen, normalMapObject.isInvertHeight);
    }
}

void MainWindow::on_invert_green_checkBox_stateChanged(int arg1)
{
    if(arg1 == 0){
        normalMapObject.GenerateNormalMap(grayscaleObject.grayscale_image, ui, normalMapObject.current_strength, normalMapObject.current_exponent, normalMapObject.isInvertRed, 0, normalMapObject.isInvertHeight);
    }
    else{
        normalMapObject.GenerateNormalMap(grayscaleObject.grayscale_image, ui, normalMapObject.current_strength, normalMapObject.current_exponent, normalMapObject.isInvertRed, 1, normalMapObject.isInvertHeight);
    }
}

void MainWindow::on_invert_height_checkBox_stateChanged(int arg1)
{
    if(arg1 == 0){
        normalMapObject.GenerateNormalMap(grayscaleObject.grayscale_image, ui, normalMapObject.current_strength, normalMapObject.current_exponent, normalMapObject.isInvertRed, normalMapObject.isInvertGreen, 0);
    }
    else{
        normalMapObject.GenerateNormalMap(grayscaleObject.grayscale_image, ui, normalMapObject.current_strength, normalMapObject.current_exponent, normalMapObject.isInvertRed, normalMapObject.isInvertGreen, 1);
    }
}



void MainWindow::on_pushButton_released()
{

    QApplication::quit();
}

void MainWindow::on_minimizeButton_released()
{
    this->setWindowState(Qt::WindowMinimized);
}

void MainWindow::on_cie_radioButton_toggled(bool checked)
{
    if(checked){
        grayscaleObject.current_luminance = 0;
        grayscaleObject.GenerateGrayScaleMap(grayscaleObject.grayscale_image, ui, grayscaleObject.current_brightness, grayscaleObject.current_contrast, grayscaleObject.isInvert);
    }
}

void MainWindow::on_radioButton_toggled(bool checked)
{
    if(checked){
        grayscaleObject.current_luminance = 1;
        grayscaleObject.GenerateGrayScaleMap(grayscaleObject.grayscale_image, ui, grayscaleObject.current_brightness, grayscaleObject.current_contrast, grayscaleObject.isInvert);
    }
}
