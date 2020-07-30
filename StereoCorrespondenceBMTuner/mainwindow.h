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


typedef struct CvStereoBMState
{
	// pre-filtering (normalization of input images)
	int preFilterType; // =CV_STEREO_BM_NORMALIZED_RESPONSE now
	int preFilterSize; // averaging window size: ~5x5..21x21
	int preFilterCap; // the output of pre-filtering is clipped by [-preFilterCap,preFilterCap]

	// correspondence using Sum of Absolute Difference (SAD)
	int SADWindowSize; // ~5x5..21x21
	int minDisparity;  // minimum disparity (can be negative)
	int numberOfDisparities; // maximum disparity - minimum disparity (> 0)

	// post-filtering
	int textureThreshold;  // the disparity is only computed for pixels
						   // with textured enough neighborhood
	int uniquenessRatio;   // accept the computed disparity d* only if
						   // SAD(d) >= SAD(d*)*(1 + uniquenessRatio/100.)
						   // for any d != d*+/-1 within the search range.
	int speckleWindowSize; // disparity variation window
	int speckleRange; // acceptable range of variation in window

	int trySmallerWindows; // if 1, the results may be more accurate,
						   // at the expense of slower processing
	cv::Rect roi1, roi2;
	int disp12MaxDiff;

	// temporary buffers
	cv::Mat* preFilteredImg0;
	cv::Mat* preFilteredImg1;
	cv::Mat* slidingSumBuf;
	cv::Mat* cost;
	cv::Mat* disp;
} CvStereoBMState;


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:
	void on_pushButton_left_clicked();

	void on_pushButton_right_clicked();

	void on_horizontalSlider_num_of_disparity_sliderMoved(int position);

	void on_horizontalSlider_num_of_disparity_valueChanged(int value);

	void on_horizontalSlider_SAD_window_size_valueChanged(int value);

	void on_horizontalSlider_pre_filter_size_valueChanged(int value);

	void on_horizontalSlider_pre_filter_cap_valueChanged(int value);

	void on_horizontalSlider_min_disparity_valueChanged(int value);

	void on_horizontalSlider_texture_threshold_valueChanged(int value);

	void on_horizontalSlider_uniqueness_ratio_valueChanged(int value);

	void on_horizontalSlider_speckle_window_size_valueChanged(int value);

	void on_horizontalSlider_speckle_range_valueChanged(int value);

	void on_horizontalSlider_disp_12_max_diff_valueChanged(int value);

private:
	// the UI object, to access the UI elements created with Qt Designer
	Ui::MainWindow *ui;

	// the left and right pictures, converted to OpenCV Mat format
	cv::Mat left_image;
	cv::Mat right_image;

	// the object that holds the parameters for the block-matching algorithm
	CvStereoBMState bmState;
	//cv::StereoBMParams bmState;
	cv::Ptr<cv::StereoSGBM> matcher = cv::StereoSGBM::create();

	void compute_depth_map();  // compute depth map with OpenCV

	// functions to manage constraints on sliders
	void set_SADWindowSize();  // manage max value of SADWindowSize slider
	void set_num_of_disparity_slider_to_multiple_16(int position);
};

#endif // MAINWINDOW_H
