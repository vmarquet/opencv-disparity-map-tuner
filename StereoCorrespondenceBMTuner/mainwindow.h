#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>

#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QMainWindow>
#include <QFileDialog>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_left_clicked();

    void on_pushButton_right_clicked();

private:
    // the UI object, to access the UI elements created with Qt Designer
    Ui::MainWindow *ui;

    // the left and right pictures, converted to OpenCV Mat format
    cv::Mat left_image;
    cv::Mat right_image;

    // the object that holds the parameters for the block-matching algorithm
    cv::StereoBM bmState;

    void compute_depth_map();
};

#endif // MAINWINDOW_H
