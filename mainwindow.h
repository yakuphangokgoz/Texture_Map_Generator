#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QPixmap>
#include <QDesktopWidget>
#include <QScreen>
#include <QMainWindow>
#include <QVector3D>
#include <QDir>
#include "grayscale.h"
#include <QTabBar>
#include <QTabWidget>
#include <QLayout>
#include <QPushButton>
#include "normalmap.h"
#include "qclcontext.h"
#include "qclprogram.h"
#include "qclkernel.h"
#include "qclimage.h"
#include <QKeyEvent>
#include <QElapsedTimer>
#include <QApplication>
#include "histogram.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Ui::MainWindow *GetUi();
    void ResetTabValues();
    QChartView *histogramView;
    Histogram pixelHistogram;


private slots:
    void on_brightness_slider_valueChanged(int value);
    void on_contrast_slider_valueChanged(int value);
    void on_invert_displacement_checkbox_stateChanged(int arg1);

    void on_main_tab_widget_currentChanged(int index);

    void on_strength_slider_valueChanged(int value);

    void on_invert_red_checkBox_stateChanged(int arg1);

    void on_invert_green_checkBox_stateChanged(int arg1);

    void on_invert_height_checkBox_stateChanged(int arg1);

    void on_exponent_slider_valueChanged(int value);

    void on_normalmap_blur_slider_valueChanged(int value);

    void on_displacement_blur_slider_valueChanged(int value);

    void on_pushButton_released();

    void on_minimizeButton_released();

    void on_cie_radioButton_toggled(bool checked);

    void on_radioButton_toggled(bool checked);

private:
    Ui::MainWindow *ui;
protected:
     void keyPressEvent(QKeyEvent *event) override;

};
#endif // MAINWINDOW_H
