#ifndef SETTING_H
#define SETTING_H

#include <iostream>
#include <sstream>
#include <time.h>
#include <stdio.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif

using namespace cv;
using namespace std;

class Setting
{
public:
    Setting();
    ~Setting();
    enum Pattern { NOT_EXISTING, CHESSBOARD, CIRCLES_GRID, ASYMMETRIC_CIRCLES_GRID };
    enum InputType {INVALID, CAMERA, VIDEO_FILE, IMAGE_LIST};
    enum { DETECTION = 0, CAPTURING = 1, CALIBRATED = 2 };

    void write(FileStorage& fs) const;
    void read(const FileNode& node);
    void interprate();
    Mat nextImage();
    static bool readStringList( const string& filename, vector<string>& l );
    static void read(const FileNode& node, Setting& x, const Setting& default_value);
    static bool isListOfImages( const string& filename);
    static double computeReprojectionErrors( const vector<vector<Point3f> >& objectPoints,
                                             const vector<vector<Point2f> >& imagePoints,
                                             const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                             const Mat& cameraMatrix , const Mat& distCoeffs,
                                             vector<float>& perViewErrors);
    static void calcBoardCornerPositions(Size boardSize, float squareSize, vector<Point3f>& corners,
                                         Pattern patternType /*= Setting::CHESSBOARD*/);
    static bool runCalibration( Setting& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                                vector<vector<Point2f> > imagePoints, vector<Mat>& rvecs, vector<Mat>& tvecs,
                                vector<float>& reprojErrs,  double& totalAvgErr);
    static void saveCameraParams( Setting& s, Size& imageSize, Mat& cameraMatrix, Mat& distCoeffs,
                                  const vector<Mat>& rvecs, const vector<Mat>& tvecs,
                                  const vector<float>& reprojErrs, const vector<vector<Point2f> >& imagePoints,
                                  double totalAvgErr );
    bool runCalibrationAndSave(Setting& s, Size imageSize, Mat&  cameraMatrix, Mat& distCoeffs,vector<vector<Point2f> > imagePoints);


public:
    Size boardSize;            // The size of the board -> Number of items by width and height
    Pattern calibrationPattern;// One of the Chessboard, circles, or asymmetric circle pattern
    float squareSize;          // The size of a square in your defined unit (point, millimeter,etc).
    int nrFrames;              // The number of frames to use from the input for calibration
    float aspectRatio;         // The aspect ratio
    int delay;                 // In case of a video input
    bool bwritePoints;         //  Write detected feature points
    bool bwriteExtrinsics;     // Write extrinsic parameters
    bool calibZeroTangentDist; // Assume zero tangential distortion
    bool calibFixPrincipalPoint;// Fix the principal point at the center
    bool flipVertical;          // Flip the captured images around the horizontal axis
    string outputFileName;      // The name of the file where to write
    bool showUndistorsed;       // Show undistorted images after calibration
    string input;               // The input ->



    int cameraID;
    vector<string> imageList;
    int atImageList;
    VideoCapture inputCapture;
    InputType inputType;
    bool goodInput;
    int flag;


private:
    string patternToUse;

};

#endif // SETTING_H
