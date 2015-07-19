#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    bmState.state->preFilterSize=41;
    bmState.state->preFilterCap=31;
    bmState.state->SADWindowSize=41;
    bmState.state->minDisparity=-64;
    bmState.state->numberOfDisparities=128;
    bmState.state->textureThreshold=10;
    bmState.state->uniquenessRatio=15;
}

MainWindow::~MainWindow()
{
    delete ui;
}

// method called when the button to change the left image is clicked
// we prompt the user to select an image, and we display it
void MainWindow::on_pushButton_left_clicked()
{
    // we prompt the user with a file dialog,
    // to select the picture file from the left camera
    QString filename = QFileDialog::getOpenFileName(this, "Select left picture file", QDir::homePath(), NULL);
    if (filename.isNull() || filename.isEmpty())
        return;

    // we load the picture from the file, to display it in a QLabel in the GUI
    QImage left_picture;
    left_picture.load(filename);

    ui->label_image_left->setPixmap(QPixmap::fromImage(left_picture));

    // we convert filename from QString to std::string (needed by OpenCV)
    std::string filename_s = filename.toUtf8().constData();

    // we load the picture in the OpenCV Mat format, to compute depth map
    cv::Mat mat = cv::imread(filename_s, CV_LOAD_IMAGE_COLOR);
    cv::cvtColor(mat, mat, CV_BGR2GRAY);  // we convert to gray, needed to compute depth map
    this->left_image = mat;

    compute_depth_map();
}

// method called when the button to change the right image is clicked
// we prompt the user to select an image, and we display it
void MainWindow::on_pushButton_right_clicked()
{
    // we prompt the user with a file dialog,
    // to select the picture file from the left camera
    QString filename = QFileDialog::getOpenFileName(this, "Select right picture file", QDir::homePath(), NULL);
    if (filename.isNull() || filename.isEmpty())
        return;

    // we load the picture from the file, to display it in a QLabel in the GUI
    QImage right_picture;
    right_picture.load(filename);

    ui->label_image_right->setPixmap(QPixmap::fromImage(right_picture));

    // we convert filename from QString to std::string (needed by OpenCV)
    std::string filename_s = filename.toUtf8().constData();

    // we load the picture in the OpenCV Mat format, to compute depth map
    cv::Mat mat = cv::imread(filename_s, CV_LOAD_IMAGE_COLOR);
    cv::cvtColor(mat, mat, CV_BGR2GRAY);  // we convert to gray, needed to compute depth map
    this->right_image = mat;

    compute_depth_map();
}

// we compute the depth map, if both left image and right image have been added
void MainWindow::compute_depth_map() {
    // we check that both images have been loaded
    if (this->left_image.empty() || this->right_image.empty())
        return;

    // we check that both images have the same size (else OpenCV throws an error)
    if (left_image.rows != right_image.rows || left_image.cols != right_image.cols) {
        ui->label_depth_map->setText("Can't compute depth map: left and right images should be the same size");
        return;
    }

    // we compute the depth map
    cv::Mat disparity_16S;  // 16 bits, signed
    bmState(left_image, right_image, disparity_16S);

    // we convert the depth map to a QPixmap, to display it in the QUI
    // first, we need to convert the disparity map to a more regular grayscale format
    // then, we convert to RGB, and finally, we can convert to a QImage and then a QPixmap

    // we normalize the values, so that they all fit in the range [0, 255]
    cv::normalize(disparity_16S, disparity_16S, 0, 255, CV_MINMAX);

    // we convert the values from 16 bits signed to 8 bits unsigned
    cv::Mat disp(disparity_16S.rows, disparity_16S.cols, CV_8UC1);
    for (int i=0; i<disparity_16S.rows; i++)
        for (int j=0; j<disparity_16S.cols; j++)
            disp.at<unsigned char>(i,j) = (unsigned char)disparity_16S.at<short>(i,j);

    // we convert from gray to color
    cv::Mat disp_color;
    cv::cvtColor(disp, disp_color, CV_GRAY2RGB);

    // we finally can convert the image to a QPixmap and display it
    QImage disparity_image = QImage((unsigned char*) disp_color.data, disp_color.cols, disp_color.rows, QImage::Format_RGB888);
    QPixmap disparity_pixmap = QPixmap::fromImage(disparity_image);

    ui->label_depth_map->setPixmap(disparity_pixmap);
}
