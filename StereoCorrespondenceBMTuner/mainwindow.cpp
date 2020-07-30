#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtDebug>

QDebug operator<<(QDebug debug, const cv::Size &size)
{
	QDebugStateSaver saver(debug);
	debug.nospace() << '(' << size.width << ", " << size.height << ')';

	return debug;
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// the default values used in OpenCV are defined here:
	// https://github.com/Itseez/opencv/blob/master/modules/calib3d/src/stereobm.cpp
	bmState.preFilterSize = 41;  // must be an odd between 5 and 255
	bmState.preFilterCap = 31;  // must be within 1 and 63
	bmState.SADWindowSize = 41;  // must be odd, be within 5..255 and be not larger than image width or height
	bmState.minDisparity = -64;
	bmState.numberOfDisparities = 128;  // must be > 0 and divisible by 16
	bmState.textureThreshold = 10;  // must be non-negative
	bmState.uniquenessRatio = 15;  // must be non-negative
	bmState.speckleWindowSize = 0;
	bmState.speckleRange = 0;
	bmState.disp12MaxDiff = -1;

	// we override the default values defined in the UI file with Qt Designer
	// to the ones defined above
	ui->horizontalSlider_pre_filter_size->setValue(bmState.preFilterSize);
	ui->horizontalSlider_pre_filter_cap->setValue(bmState.preFilterCap);
	ui->horizontalSlider_SAD_window_size->setValue(bmState.SADWindowSize);
	ui->horizontalSlider_min_disparity->setValue(bmState.minDisparity);
	ui->horizontalSlider_num_of_disparity->setValue(bmState.numberOfDisparities);
	ui->horizontalSlider_texture_threshold->setValue(bmState.textureThreshold);
	ui->horizontalSlider_uniqueness_ratio->setValue(bmState.uniquenessRatio);
	ui->horizontalSlider_speckle_window_size->setValue(bmState.speckleWindowSize);
	ui->horizontalSlider_speckle_range->setValue(bmState.speckleRange);
	ui->horizontalSlider_disp_12_max_diff->setValue(bmState.disp12MaxDiff);
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
	QString filename = QFileDialog::getOpenFileName(this, "Select left picture file", QDir::homePath(), nullptr);
	if (filename.isNull() || filename.isEmpty())
		return;

	///// Qt display stuff

	// we load the picture from the file, to display it in a QLabel in the GUI
	QImage left_picture;
	left_picture.load(filename);

	// some computation to resize the image if it is too big to fit in the GUI
	QPixmap left_pixmap = QPixmap::fromImage(left_picture);
	int max_width  = std::min(ui->label_image_left->maximumWidth(), left_picture.width());
	int max_height = std::min(ui->label_image_left->maximumHeight(), left_picture.height());
	ui->label_image_left->setPixmap(left_pixmap.scaled(max_width, max_height, Qt::KeepAspectRatio));

	set_SADWindowSize();  // the SAD window size parameter depends on the size of the image

	///// OpenCV disparity map computation

	// we convert filename from QString to std::string (needed by OpenCV)
	std::string filename_s = filename.toUtf8().constData();

	// we load the picture in the OpenCV Mat format, to compute depth map
	cv::Mat mat = cv::imread(filename_s, cv::IMREAD_COLOR );
	cv::cvtColor(mat, mat, cv::COLOR_BGR2GRAY);  // we convert to gray, needed to compute depth map
	this->left_image = mat;

	compute_depth_map();
}

// method called when the button to change the right image is clicked
// we prompt the user to select an image, and we display it
void MainWindow::on_pushButton_right_clicked()
{
	// we prompt the user with a file dialog,
	// to select the picture file from the left camera
	QString filename = QFileDialog::getOpenFileName(this, "Select right picture file", QDir::homePath(), nullptr);
	if (filename.isNull() || filename.isEmpty())
		return;

	///// Qt display stuff

	// we load the picture from the file, to display it in a QLabel in the GUI
	QImage right_picture;
	right_picture.load(filename);

	// some computation to resize the image if it is too big to fit in the GUI
	QPixmap right_pixmap = QPixmap::fromImage(right_picture);
	int max_width  = std::min(ui->label_image_right->maximumWidth(), right_picture.width());
	int max_height = std::min(ui->label_image_right->maximumHeight(), right_picture.height());
	ui->label_image_right->setPixmap(right_pixmap.scaled(max_width, max_height, Qt::KeepAspectRatio));

	set_SADWindowSize();  // the SAD window size parameter depends on the size of the image

	///// OpenCV disparity map computation

	// we convert filename from QString to std::string (needed by OpenCV)
	std::string filename_s = filename.toUtf8().constData();

	// we load the picture in the OpenCV Mat format, to compute depth map
	cv::Mat mat = cv::imread(filename_s, cv::IMREAD_COLOR);
	cv::cvtColor(mat, mat, cv::COLOR_BGR2GRAY);  // we convert to gray, needed to compute depth map
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

	matcher = cv::StereoSGBM::create(bmState.minDisparity, bmState.numberOfDisparities,
									 bmState.preFilterSize,8*9*left_image.channels(),32*9*left_image.channels(),
									 bmState.disp12MaxDiff, bmState.preFilterCap,
									 bmState.uniquenessRatio,
									 bmState.speckleWindowSize, bmState.speckleRange);
	// we compute the depth map
	cv::Mat disparity_16S;  // 16 bits, signed
	matcher->compute(left_image, right_image, disparity_16S);

	// we convert the depth map to a QPixmap, to display it in the QUI
	// first, we need to convert the disparity map to a more regular grayscale format
	// then, we convert to RGB, and finally, we can convert to a QImage and then a QPixmap

	// we normalize the values, so that they all fit in the range [0, 255]
	cv::normalize(disparity_16S, disparity_16S, 0, 255, cv::NORM_MINMAX);

	// we convert the values from 16 bits signed to 8 bits unsigned
	cv::Mat disp(disparity_16S.rows, disparity_16S.cols, CV_8UC1);
	disparity_16S.convertTo(disp, CV_8UC1);

	// we convert from gray to color
	//cv::Mat disp_color;
	//cv::cvtColor(disp, disp_color, cv::COLOR_GRAY2RGB);

	// we finally can convert the image to a QPixmap and display it
	//QImage disparity_image = QImage((unsigned char*) disp_color.data, disp_color.cols, disp_color.rows, QImage::Format_RGB888);
	QImage disparity_image = QImage((unsigned char*) disp.data, disp.cols, disp.rows, QImage::Format_Grayscale8);

	// some computation to resize the image if it is too big to fit in the GUI
	int max_width  = std::min(ui->label_depth_map->maximumWidth(),  disparity_image.width());
	int max_height = std::min(ui->label_depth_map->maximumHeight(), disparity_image.height());

	disparity_image = disparity_image.scaled(max_width, max_height, Qt::KeepAspectRatio);
	QPixmap disparity_pixmap = QPixmap::fromImage(disparity_image);

	qDebug() << Q_FUNC_INFO << left_image.size() << right_image.size() << disparity_image.size();

	ui->label_depth_map->setPixmap(disparity_pixmap);

}


/////////////////// Sliders management (callbacks and constraints) //////////////////////

///// pre filter size

// must be an odd number
void MainWindow::on_horizontalSlider_pre_filter_size_valueChanged(int value)
{
	if ((value % 2) == 0) {
		value -= 1;
		ui->horizontalSlider_pre_filter_size->setValue(value);
	}

	bmState.preFilterSize = value;
	compute_depth_map();
}

///// pre filter cap

void MainWindow::on_horizontalSlider_pre_filter_cap_valueChanged(int value)
{
	bmState.preFilterCap = value;
	compute_depth_map();
}

///// SAD window size

// the SAD Window size should always be smaller than the size of the images
// so when a new image is loaded, we set the maximum value for the slider
void MainWindow::set_SADWindowSize() {
	int value = 255;  // max value allowed

	// we check that the value is not bigger than the size of the pictures
	if (! left_image.empty())
		value = std::min(value, std::min(left_image.cols, left_image.rows));
	if (! right_image.empty())
		value = std::min(value, std::min(right_image.cols, right_image.rows));

	// we check that the value is >= 5
	value = std::max(value, 5);

	ui->horizontalSlider_SAD_window_size->setMaximum(value);
}

// must be an odd number
void MainWindow::on_horizontalSlider_SAD_window_size_valueChanged(int value)
{
	if ((value % 2) == 0) {
		value -= 1;
		ui->horizontalSlider_SAD_window_size->setValue(value);
	}

	bmState.SADWindowSize = value;
	compute_depth_map();
}

///// Minimum disparity

void MainWindow::on_horizontalSlider_min_disparity_valueChanged(int value)
{
	bmState.minDisparity = value;
	compute_depth_map();
}

///// Number of disparities

// callback when slider for number of disparities is moved
// we must allow only values that are divisible by 16
void MainWindow::on_horizontalSlider_num_of_disparity_sliderMoved(int value)
{
	set_num_of_disparity_slider_to_multiple_16(value);
}

// callback when slider for number of disparities is changed
// (for the case when the slider is not moved (just a click), because then the callback above is not called)
// we must allow only values that are divisible by 16
void MainWindow::on_horizontalSlider_num_of_disparity_valueChanged(int value)
{
	set_num_of_disparity_slider_to_multiple_16(value);
}

void MainWindow::set_num_of_disparity_slider_to_multiple_16(int value) {
	if ((value % 16) != 0) {
		value -= (value % 16);
		ui->horizontalSlider_num_of_disparity->setValue(value);
	}

	bmState.numberOfDisparities = value;
	compute_depth_map();
}

///// Texture threshold

void MainWindow::on_horizontalSlider_texture_threshold_valueChanged(int value)
{
	bmState.textureThreshold = value;
	compute_depth_map();
}

///// Uniqueness ratio

void MainWindow::on_horizontalSlider_uniqueness_ratio_valueChanged(int value)
{
	bmState.uniquenessRatio = value;
	compute_depth_map();
}

///// Speckle window size

void MainWindow::on_horizontalSlider_speckle_window_size_valueChanged(int value)
{
	bmState.speckleWindowSize = value;
	compute_depth_map();
}

///// Speckle range

void MainWindow::on_horizontalSlider_speckle_range_valueChanged(int value)
{
	bmState.speckleRange = value;
	compute_depth_map();
}

///// Disparity 12 maximum difference

void MainWindow::on_horizontalSlider_disp_12_max_diff_valueChanged(int value)
{
	bmState.disp12MaxDiff = value;
	compute_depth_map();
}
