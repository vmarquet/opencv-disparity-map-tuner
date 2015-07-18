#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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

    // we load the picture from the file
    QImage left_picture;
    left_picture.load(filename);

    ui->label_image_left->setPixmap(QPixmap::fromImage(left_picture));
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

    // we load the picture from the file
    QImage right_picture;
    right_picture.load(filename);

    ui->label_image_right->setPixmap(QPixmap::fromImage(right_picture));
}
