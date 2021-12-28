#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <setting.h>
#include <qfiledialog.h>

using namespace std;
using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    Setting s;
    vector<vector<Point2f> > imagePoints;
    Mat cameraMatrix, distCoeffs;
    Size imageSize;
    clock_t prevTimestamp = 0;
    const char ESC_KEY = 27;
    int mode;
    vector<Mat> rvecs, tvecs;
    vector<float> reprojErrs;
    double totalAvgErr = 0;
    bool ok=false;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void actionLoad_XML_clicked();

private slots:
    void clearLayout();
    QPixmap Mat2QPixmap(const cv::Mat &mat);
    void DetectionImage();
    void on_actionLoad_triggered();

    void on_actionLoad_XML_triggered();

    void on_actionClose_triggered();

    void on_actionClose_2_triggered();

    void on_actionCapture_triggered();

    void on_actionShow_undistorted_image_triggered();

    void on_actionCalibrate_triggered();

    void on_actionShow_Calibrated_Camera_triggered();

    void on_actionSafe_YML_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
