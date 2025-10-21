/*~~~~~~~~~~~~~~~
    Includes
~~~~~~~~~~~~~~~*/
// Library includes
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <cmath> 
#include <string>
#include <fstream>
#include <sstream>
#include <deque>
#include <ctime>
#include <opencv2/core/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/calib3d.hpp>
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include <fstream>
#include <chrono>
// Internal includes
#include "../src/polyhuco/polyhuco.h"
#include "../utils/utils.hpp"
#include "colors.hpp"




std::stringstream auxiliarrrr;





// Image width and height
double global_width = 1920;
double global_height = 1080;

// Command ID
std::string command_id = "0";

// Fiducial Input cursor
cv::Point2i fi_input = {0,0};

// Delete commands file
bool deleteCommandFile = true; // Set to true by default to delete the file at the end

/* IMAGE REGIONS */
struct ImageRegion {
    std::string identifier;
    std::vector<cv::Point> points;
    std::string command;
    cv::Scalar color;  // RGBA for drawing with transparency

    ImageRegion(const std::string& id, const std::vector<cv::Point>& pts, const std::string& cmd, const cv::Scalar& clr)
    : identifier(id), points(pts), command(cmd), color(clr) {}
};
// initializeRegions
std::vector<ImageRegion> initializeRegions() {
    std::vector<ImageRegion> regions;
    int margin = 0;

    // Left trapezoid (vertical, thicker, separated from border)
    regions.emplace_back(
        "Region_for_left",
        std::vector<cv::Point>{
            {0, 311},   // Top-left corner
            {435, 311}, // Top-right corner
            {630, (766-311)/3+311}, // Next right point on the top line
            {630, (766-311)/3*2+311}, // Bottom-right corner
            {435, 766}, // Bottom-left next corner
            {0, 766}    // Bottom-left corner
        },
        "left",
        cv::Scalar(255, 0, 0, 128)  // Red semi-transparent
    );

    // Right trapezoid (vertical, thicker, separated from border)
    regions.emplace_back(
        "Region_for_right",
        std::vector<cv::Point>{
            {1920 - 630, (766-311)/3+311},     // Corresponding inward upper point
            {1920 - 435, 311},     // Next inward upper point
            {1920, 311},           // Upper inward point with margin
            {1920, 766},           // Lower point with same x-coordinate
            {1920 - 435, 766},     // Next inward lower point
            {1920 - 630, (766-311)/3*2+311}      // Further inward lower point
        },
        "right",
        cv::Scalar(0, 255, 0, 128)  // Green semi-transparent
    );

    // Bottom trapezoid (horizontal, thicker, separated from border)
    regions.emplace_back(
        "Region_for_down",
        std::vector<cv::Point>{
            {555, 1080},          // Bottom-left corner
            {1364, 1080},        // Bottom-right corner
            {1364, 1080 - 246},  // Upper-right corner (first horizontal level)
            {1214, 1080 - 356},  // Top-right corner (second horizontal level)
            {705, 1080 - 356},   // Top-left corner (second horizontal level)
            {555, 1080 - 246}    // Upper-left corner (first horizontal level)
        },
        "down",
        cv::Scalar(0, 0, 255, 128)  // Blue semi-transparent
    );

    // Top trapezoid (horizontal, thicker, separated from border)
    regions.emplace_back(
        "Region_for_up",
        std::vector<cv::Point>{
            {555, 0},            // Bottom-left corner
            {1364, 0},          // Bottom-right corner
            {1364, 246},        // Upper-right corner (first horizontal level)
            {1214, 356},        // Top-right corner (second horizontal level)
            {705, 356},         // Top-left corner (second horizontal level)
            {555, 246}          // Upper-left corner (first horizontal level)
        },
        "up",
        cv::Scalar(255, 255, 0, 128)  // Yellow semi-transparent
    );


    // Change scene
    regions.emplace_back(
        "Region_for_changeEscene",
        std::vector<cv::Point>{
            {380, 130},        // Top-left corner (shifted slightly left & up)
            {520, 130},        // Top-right corner (expanded width)
            {520, 270},        // Bottom-right corner (expanded height)
            {380, 270}         // Bottom-left corner
        },
        "change_escene",
        cv::Scalar(0, 255, 255, 128)  // Cyan semi-transparent
    );


    // Move nearer
    regions.emplace_back(
        "Region_for_moveNearer",
        std::vector<cv::Point>{
            {380, 880},        // Top-left corner (shifted slightly left & up)
            {520, 880},        // Top-right corner (expanded width)
            {520, 1020},        // Bottom-right corner (expanded height)
            {380, 1020}         // Bottom-left corner
        },
        "move_nearer",
        cv::Scalar(50, 255, 255, 128)  // Cyan semi-transparent
    );


    // Move further
    regions.emplace_back(
        "Region_for_moveFurther",
        std::vector<cv::Point>{
            {180, 880},        // Top-left corner (shifted slightly left & up)
            {320, 880},        // Top-right corner (expanded width)
            {320, 1020},        // Bottom-right corner (expanded height)
            {180, 1020}         // Bottom-left corner
        },
        "move_further",
        cv::Scalar(50, 255, 255, 128)  // Cyan semi-transparent
    );


    // Change language
    regions.emplace_back(
        "Region_for_changeLanguage",
        std::vector<cv::Point>{
            {1400, 880},        // Top-left corner (shifted slightly left & up)
            {1540, 880},        // Top-right corner (expanded width)
            {1540, 1020},        // Bottom-right corner (expanded height)
            {1400, 1020}         // Bottom-left corner
        },
        "change_language",
        cv::Scalar(50, 255, 255, 128)  // Cyan semi-transparent
    );

    return regions;
}
// isPointInRegion
bool isPointInRegion(const cv::Point2f& point, const std::vector<cv::Point>& polygon) {
    return cv::pointPolygonTest(polygon, point, false) >= 0;
}
// drawRegions
void drawRegions(cv::Mat& frame, const std::vector<ImageRegion>& regions) {
    for (const auto& region : regions) {
        cv::Mat overlay;
        frame.copyTo(overlay);
        std::vector<std::vector<cv::Point>> pts = {region.points};
        cv::fillPoly(overlay, pts, region.color);
        cv::addWeighted(overlay, 0.5, frame, 0.5, 0, frame);
    }
}


/**
 * Draws a semitransparent white circle with a black border at the specified point on the given image.
 * @param image Reference to the image where the circle will be drawn.
 * @param centerPoint The point where the circle will be centered.
 */
void drawSemitransparentCircle(cv::Mat& image, const cv::Point2f& centerPoint) {
    int radius = 20; // Radius of the circle
    double alpha = 0.5; // Transparency factor

    // Create a temporary image to draw the circle
    cv::Mat overlay;
    image.copyTo(overlay);

    // Draw a solid white circle with reduced opacity
    cv::circle(overlay, centerPoint, radius, cv::Scalar(255, 255, 255), -1); // -1 means filled circle

    // Blend the overlay with the original image
    cv::addWeighted(overlay, alpha, image, 1 - alpha, 0, image);

    // Optionally, add a border to the circle for better visibility
    cv::circle(image, centerPoint, radius, cv::Scalar(255, 255, 255), 5); // Black border
}




// reprojectPoseCenterToImage
cv::Point2f reprojectPoseCenterToImage(const cv::Mat& rvec, const cv::Mat& tvec, const cv::Mat& K, const cv::Mat& D) {
    // Assuming the center of the pose in world coordinates is at the origin (0, 0, 0)
    std::vector<cv::Point3f> objectPoints = { cv::Point3f(0.0f, 0.0f, 0.0f) };
    std::vector<cv::Point2f> imagePoints;

    // Project the 3D point to the 2D image plane
    cv::projectPoints(objectPoints, rvec, tvec, K, D, imagePoints);

    // Return the first point which is the center
    return imagePoints[0];
}



// Assuming THRESHOLD is defined somewhere in your code.
const double THRESHOLD = 15.0;  // Example threshold value.


// Function to compute the direction of rotation with respect to camera's perspective
std::string computeDirection(const cv::Vec3d& rotationChange, const cv::Vec3d& currentOrientation) {
    std::string direction = "";
    double horizontalChange = std::abs(rotationChange[1]); // yaw change around y-axis
    double verticalChange = std::abs(rotationChange[0]); // pitch change around x-axis

    // Initialize variables to find the maximum rotation exceeding the threshold
    double maxChange = 0;
    int maxAxis = -1; // -1 indicates no axis has exceeded the threshold

    // Check horizontal changes (yaw)
    if (horizontalChange > THRESHOLD) {
        maxChange = horizontalChange;
        maxAxis = 1; // Indicate yaw axis
    }
    // Check vertical changes (pitch)
    if (verticalChange > THRESHOLD && verticalChange > maxChange) {
        maxChange = verticalChange;
        maxAxis = 0; // Indicate pitch axis
    }

    // Determine the direction based on the axis with the maximum change
    // Adjusting for object's current orientation
    switch (maxAxis) {
        case 0: // Pitch
            direction = (rotationChange[0] > 0) ? "move_nearer" : "move_further";
            break;
        case 1: // Yaw
            direction = (rotationChange[1] > 0) ? "move_nearer" : "move_further";
            break;
        default:
            direction = "none"; // No change exceeded the threshold
            break;
    }

    return direction;
}



// Global variable to track whether the file should be truncated
bool isFirstRun = true;

void writeDirectionToFile(const std::string& direction) {
    // Determine the opening mode based on whether it's the first run
    std::ios_base::openmode mode = isFirstRun ? std::ios::trunc : std::ios::app;

    // Open the file with the determined mode
    std::ofstream outFile("/home/paul/commands.txt", mode);

    if (outFile.is_open()) {
        outFile << fi_input.x << "," << fi_input.y << "|" << command_id << ": " << direction << std::endl;
        outFile.close();
    } else {
        std::cerr << "Failed to open the file for writing." << std::endl;
    }

    // After the first write, set isFirstRun to false to append on subsequent writes
    isFirstRun = false;
}




std::deque<cv::Vec3d> last_rotation_vectors;

cv::Vec3d computeAverageRotationChange(const cv::Mat& current_rvec) {
    cv::Mat current_R;
    cv::Rodrigues(current_rvec, current_R);

    // Make sure we have a reference frame to compare to
    if (last_rotation_vectors.empty()) {
        last_rotation_vectors.push_back(current_rvec);
        return cv::Vec3d(0, 0, 0); // No rotation relative to itself
    }

    cv::Vec3d average_rotation_change(0, 0, 0);
    int count = 0;

    // Get the reference rotation matrix from the first element in the deque
    cv::Mat reference_R;
    cv::Rodrigues(last_rotation_vectors.front(), reference_R);
    cv::Mat reference_R_inv = reference_R.t(); // The inverse of a rotation matrix is its transpose

    // Compute rotations relative to the first stored rotation
    for (const auto& past_rvec : last_rotation_vectors) {
        cv::Mat past_R;
        cv::Rodrigues(past_rvec, past_R);

        // Compute relative rotation matrix using the inverse of the reference frame
        cv::Mat R_relative = reference_R_inv * past_R;

        // Convert relative rotation matrix back to rotation vector
        cv::Vec3d rotation_change;
        cv::Rodrigues(R_relative, rotation_change);

        // Accumulate and count for averaging
        average_rotation_change += rotation_change;
        count++;
    }

    // Add the current rotation vector to the deque
    last_rotation_vectors.push_back(current_rvec);
    if (last_rotation_vectors.size() > 5) {
        last_rotation_vectors.pop_front();  // Maintain only the latest five rotations
    }

    // If there were past vectors, compute the average
    if (count > 0) {
        average_rotation_change *= (1.0 / count);
    }

    // Convert radians to degrees for easier interpretation
    average_rotation_change *= (180.0 / CV_PI);

    return average_rotation_change;
}





// printAlert
void printAlert(cv::String message){
    auxiliarrrr<<BRED<<"[!] "+message+" [!]"<<RESET<<std::endl;
}

using namespace std;

// Parameters
const cv::String keys =
"{help h usage ? | | Print this message}"
// Verboses
"{v | | Activate verbose mode. Default: Deactivated.}"
"{vdebug | | Activate verbose debug mode. Default: Deactivated.}"
"{vframe | | Activate verbose frame mode. Default: Deactivated.}"
// Minimal options
"{minimal | | Activate minimal mode. Default: Deactivated.}"
"{noTerminal | | Deactivate terminal printing. Default: Activated.}"
// Detection mode
"{detectionMode |1| Detection mode. 1: Cucomarker. 2: Aruco}"
// Input
"{inputImage         |<none>| Image to be processed}"
"{inputVideo         |<none>| Video to be processed}"
// Mandatory params
"{@cameraParams |<none>| Camera parameters.}"
"{@markerSize |<none>| Real size of the marker.}"
"{@mapPath |<none>| Path for the map.}"
// Marker params
"{markerParams        |<none>| Markers parameters. Default: none.yml}"
"{idsRange | | Defines the id range, in order to determine the id looking for range, and can be like '1-7'. Default: none. }"
// Outputs
    // Video
    "{outputVideoPath |output.avi| Path for the output video. Default: output.avi. Recommended format: avi.}"
    "{noOutputVideo | | Generate output video. Default: yes.}"
    // Poses file
    "{outputPosePath |poses.txt| Output filename of pose path}"
    "{generatePosesOutputFile | | Generate poses output file. Default: Deactivated.}"
    // Distances file
    "{outputDistancePath |distances.txt| Define the distances output path. Default: none. }"
    "{generateDistancesOutputFile | | Generate distances output file. Default: Deactivated.}"
// Thresholds
"{thresNDet |0| Threshold for the minimun number of detected markers to count as a valid pose.}"
"{thresArea |0| Threshold for the minimun average computed area to count as a valid pose.}"
"{thresAngle |0| Threshold for the minimun average computed angle to count as a valid pose.}"
"{thresDiff |0| Threshold for the minimun average computed angle to count as a valid pose.}"
// Refinement
"{applyRefinement | | Applies refinement since beginning of execution. Starting with look for markers = 1. Default: Deactivated.}"
"{lookForMarkers |<none>| Establishes look for markers value. Default: None.}"
"{checkCandidates |<none>| Establishes the mandatory checking with candidates for the looked markers. Default: Deactivated.}"
    // Options
    "{subPixSize |3| Size of the kernel of cornerSubPix. Default: 3 .}"
    "{subPixRep |12| Number of repetitions of cornerSubPix. Default: 12 .}"
    "{subPixEpsilon |0.005| Value of Epsilon for cornerSubPiex. Default: 0.005 .}"
// Pose estimation
"{solvePNPMethod |0| Defines the method to compute the pose of the polyhuco. 0: ITERATIVE, 1: EPNP, 2: P3P, 3: DLS, 4: UPNP, 5: AP3P, 6: IPPE, 7: IPPE_SQUARE, 8: SQPNP, 9: MAX_COUNT. Default: 0.}" 
// Valid frames
"{framesRanges | | Defines the frames range to process. It can be like '1,3-8,13' or like 'frames.txt' where the content of the file is the same changing the , for new line character. Default: none. }"
// Scaling for map
"{scaleForMap |1| ... .}"
// Pause options
"{startingPaused | | Start the processing in a pause, and with the help extended. }"
// Printing on image information
"{lineStartingShift |0| Line starting point. By default the first possible line. }"
;
// Main
int main(int argc, char**argv){
    /*~~~~~~~~~~~~~~
        Styles
    ~~~~~~~~~~~~~~*/
    cv::Scalar frameTextColor = cv::Scalar(88,2,237);


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Reading the parameters
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    cv::CommandLineParser parser(argc, argv, keys);
    parser.about("Developer Fiducial object example application.");
    if(parser.has("help")){
        parser.printMessage();
        return 0;
    }
    if(argc < 3){
        parser.printMessage(); return -1;
    }


    // No terminal
    bool terminal=true; if(parser.has("noTerminal")){terminal=false;}
    if(!terminal){
        streambuf* orig_buf = cout.rdbuf();
        cout.rdbuf(NULL);
    }
    // Parameters
    auxiliarrrr<<std::endl;
    auxiliarrrr<<BGREEN<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Parameters: ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<RESET<<std::endl;
    // Verboses
    bool verbose=false; if(parser.has("v")){verbose=true;}
    bool verboseDebug=false; if(parser.has("vdebug")){verboseDebug=true;}
    bool verboseFrame=false; if(parser.has("vframe")){verboseFrame=true;}
    auxiliarrrr<<CYAN<<"- Verbose: "<<RESET<<verbose<<std::endl;
    auxiliarrrr<<CYAN<<"- Verbose debug: "<<RESET<<verboseDebug<<std::endl;
    auxiliarrrr<<CYAN<<"- Verbose frame: "<<RESET<<verboseFrame<<std::endl;
    // Minimal 
    bool minimal=false; if(parser.has("minimal")){minimal=true;}
    auxiliarrrr<<CYAN<<"- Minimal: "<<RESET<<minimal<<std::endl;
    auxiliarrrr<<CYAN<<"- Terminal: "<<RESET<<terminal<<std::endl;
    if(minimal){
        verbose = false;
        verboseDebug = false;
        verboseFrame = false;
    }
    // Detection mode
    int detectionMode = parser.get<int>("detectionMode");
    auxiliarrrr<<MAGENTA<<"- Detection mode: "<<RESET<<detectionMode<<std::endl;
    // Others
    bool isImage=false; std::string inputImagePath = ""; if(parser.has("inputImage")){ inputImagePath = parser.get<std::string>("inputImage"); isImage=true; };
    bool isVideo=false; std::string inputVideoPath = ""; if(parser.has("inputVideo")){ inputVideoPath = parser.get<std::string>("inputVideo"); isVideo=true; };
    auto cameraParamsPath = parser.get<std::string>("@cameraParams");
    double markerSize = parser.get<double>("@markerSize");
    cv::String mapPath = parser.get<std::string>("@mapPath");
    auto markersParamsPath = parser.get<std::string>("markerParams");
    auxiliarrrr<<GREEN<<"- Image path: "<<RESET<<inputImagePath<<std::endl;
    auxiliarrrr<<GREEN<<"- Video path: "<<RESET<<inputVideoPath<<std::endl;
    auxiliarrrr<<GREEN<<"- Camera params path: "<<RESET<<cameraParamsPath<<std::endl;
    auxiliarrrr<<GREEN<<"- Marker size: "<<RESET<<markerSize<<std::endl;
    auxiliarrrr<<GREEN<<"- Map path: "<<RESET<<mapPath<<std::endl;
    auxiliarrrr<<GREEN<<"- Markers params path: "<<RESET<<markersParamsPath<<std::endl;
    // Output
        // Video
        auto outputVideoPath = parser.get<std::string>("outputVideoPath");
        bool noOutputVideo = false; if(parser.has("noOutputVideo")){noOutputVideo=true;}
        auxiliarrrr<<GREEN<<"- Output video path: "<<RESET<<outputVideoPath<<std::endl;
        auxiliarrrr<<GREEN<<"\t- Generate output video?: "<<RESET<<!noOutputVideo<<std::endl;
        // Poses
        cv::String outputPosePath = parser.get<std::string>("outputPosePath");
        bool generatePosesOutputFile=false; if(parser.has("generatePosesOutputFile")){generatePosesOutputFile=true;}
            // Exists?
            if(generatePosesOutputFile){
                if(std::ifstream(outputPosePath).good()){
                    system(("rm "+outputPosePath).c_str());
                }
                system(("touch "+outputPosePath).c_str());
            }
        auxiliarrrr<<GREEN<<"- Output poses path: "<<RESET<<outputPosePath<<std::endl;
        auxiliarrrr<<GREEN<<"\t- Generate output poses file?: "<<RESET<<generatePosesOutputFile<<std::endl;
        // Distances
        cv::String outputDistancePath = parser.get<std::string>("outputDistancePath");
        bool generateDistancesOutputFile=false; if(parser.has("generateDistancesOutputFile")){generateDistancesOutputFile=true;}
            // Exists?
            if(generateDistancesOutputFile){
                if(std::ifstream(outputDistancePath).good()){
                    system(("rm "+outputDistancePath).c_str());
                }
                system(("touch "+outputDistancePath).c_str());
            }
        auxiliarrrr<<GREEN<<"- Output distances path: "<<RESET<<outputDistancePath<<std::endl;
        auxiliarrrr<<GREEN<<"\t- Generate output distances file?: "<<RESET<<generateDistancesOutputFile<<std::endl;
    // Refinement
    int lookForMarkers = 0;
    bool applyRefinement = false;
    if(parser.has("applyRefinement")){
        applyRefinement = true;
        lookForMarkers = 1;
    }
    auxiliarrrr<<CYAN<<"- Apply refinement: "<<RESET<<applyRefinement<<std::endl;
    // Look for markers
    if(parser.has("lookForMarkers")){
        lookForMarkers = parser.get<int>("lookForMarkers");
    }
    auxiliarrrr<<MAGENTA<<"- Look for markers: "<<RESET<<lookForMarkers<<std::endl;
    // checkCandidates
    bool checkCandidates = false;
    if(parser.has("checkCandidates")){
        checkCandidates = true;
    }
    auxiliarrrr<<CYAN<<"- Check candidates: "<<RESET<<checkCandidates<<std::endl;
    // Thresholds
    double thresNDet = parser.get<double>("thresNDet");
    double thresArea = parser.get<double>("thresArea");
    double thresAngle = parser.get<double>("thresAngle");
    double thresDiff = parser.get<double>("thresDiff");
    auxiliarrrr<<BLUE<<"- Threshold minimum num of detections: "<<RESET<<thresNDet<<std::endl;
    auxiliarrrr<<BLUE<<"- Threshold minimun average area: "<<RESET<<thresArea<<std::endl;
    auxiliarrrr<<BLUE<<"- Threshold minimum average angle: "<<RESET<<thresAngle<<std::endl;
    auxiliarrrr<<BLUE<<"- Threshold maximun difference between poses: "<<RESET<<thresDiff<<std::endl;
    // Pose computation option
    int solvePNPMethod = parser.get<int>("solvePNPMethod");
    auxiliarrrr<<BLUE<<"- Solve PNP Method: "<<RESET<<solvePNPMethod<<std::endl;
    // CornerSubPix options
    double subPixSize = parser.get<double>("subPixSize");
    double subPixRep = parser.get<double>("subPixRep");
    double subPixEpsilon = parser.get<double>("subPixEpsilon");
    auxiliarrrr<<BLUE<<"- CornerSubPix subPixSize: "<<RESET<<subPixSize<<std::endl;
    auxiliarrrr<<BLUE<<"- CornerSubPix subPixRep: "<<RESET<<subPixRep<<std::endl;
    auxiliarrrr<<BLUE<<"- CornerSubPix subPixEpsilon: "<<RESET<<subPixEpsilon<<std::endl;
    // Id range
    int id_min = -1;
    int id_max = -1;
    if(parser.has("idsRange")){
        std::string range = parser.get<std::string>("idsRange");
        int pos = -1;
        for(int spos = 0; spos < range.length(); spos++){
            if(range[spos] == '-'){
                pos = spos;
                break;
            }
        }
        if(pos != -1){
            id_min = std::stoi(  range.substr(0, pos)  );
            id_max = std::stoi(  range.substr(pos+1, range.length())  );
        }
    }
    auxiliarrrr<<GREEN<<"- Id min: "<<RESET<<id_min<<std::endl;
    auxiliarrrr<<GREEN<<"- Id max: "<<RESET<<id_max<<std::endl;
    // Frames range
    std::vector<int> valid_frames;
    if(parser.has("framesRanges")){
        std::string range = parser.get<std::string>("framesRanges");
        std::vector<std::string> splits;
        // File content or directly parametrised
        FILE *file = fopen(range.c_str(), "r");
        if(!file){ // PARAMETRISED CONTENT
            // Different ranges or values?
            std::vector<int> commas;
            commas.push_back(-1);
            for(int spos = 0; spos < range.length(); spos++){
                if(range[spos] == ','){
                    commas.push_back(spos);
                }
            }
            // String substrs
            commas.push_back(range.length());
            for(int c = 0; c < (commas.size()-1); c++){
                auxiliarrrr<<"pos: "<<c<<" |"<<commas[c]+1<<" - "<<commas[c+1]<<std::endl;
                std::string split = range.substr(commas[c]+1, commas[c+1]-commas[c]-1); splits.push_back(split);
            }
        }
        else{ // FILE CONTENT
            std::ifstream framesFile(range);
            std::string line;
            while(std::getline(framesFile, line)){
                if(line != "\n"){
                    line.erase(remove(line.begin(), line.end(), '\n'), line.end());
                    splits.push_back(line);
                }
            }
        }
        // Valid frames processing
        auxiliarrrr<<std::endl;
        for(int s = 0; s < splits.size(); s++){
            int pos = -1;
            for(int sc = 0; sc < splits[s].size(); sc++){
                if(splits[s][sc] == '-'){
                    pos = sc;
                }
            }
            if(pos == -1){ // No range
                int v = std::stoi(splits[s]); 
                valid_frames.push_back(v);
            }
            else{ // Range
                int min = std::stoi( splits[s].substr(0, pos) );
                int max = std::stoi( splits[s].substr(pos+1, splits[s].length()) );
                for(int v = min; v <= max; v++){
                    valid_frames.push_back(v);
                }
            }
        }
    }
    // Scale to map
    double scaleForMap = parser.get<double>("scaleForMap");
    auxiliarrrr<<MAGENTA<<"- Scale for map: "<<RESET<<scaleForMap<<std::endl;
    // Refinement
    bool showUserInputsMessages = false;
    bool startingPaused = false;
    if(parser.has("startingPaused")){
        startingPaused = true;
        showUserInputsMessages = true;
    }
    auxiliarrrr<<MAGENTA<<"- Starting paused: "<<RESET<<startingPaused<<std::endl;
    // lineStartingShift
    int lineStartingShift = parser.get<int>("lineStartingShift");
    auxiliarrrr<<GREEN<<"- Line starting point: "<<RESET<<lineStartingShift<<std::endl;
    auxiliarrrr<<BGREEN<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"<<RESET<<std::endl<<std::endl;
    //if(!minimal) cv::waitKey(0);


    /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Polyhuco configuration
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
    std::string polyhucoName = "Polyhuco figure";
    std::string markersTypeName = "Cucomarker";
    polyhuco::Polyhuco polyhucoProcessor = polyhuco::Polyhuco(polyhucoName,
        12,
        markersTypeName,
        markerSize,
        detectionMode,
        markersParamsPath,
        mapPath,
        verbose,
        verboseDebug,
        verboseFrame,
        solvePNPMethod,
        subPixSize,
        subPixRep,
        subPixEpsilon,
        id_min,
        id_max,
        outputDistancePath,
        scaleForMap,
        checkCandidates
    );
    polyhucoProcessor.setThresAngle(thresAngle);
    polyhucoProcessor.setThresNDet(thresNDet);
    polyhucoProcessor.setThresArea(thresArea);
    polyhucoProcessor.setThresDiff(thresDiff);
    polyhucoProcessor.setVerboses(false, false, false);


    // Initialize image regions
    std::vector<ImageRegion> regions = initializeRegions();



    if(isVideo){
        /*~~~~~~~~~~~~~~~~~~~~~~
            Sources setting
        ~~~~~~~~~~~~~~~~~~~~~~*/
        // Input video or camera
        cv::VideoCapture cap;
        double waitTime = 1;
        auto TheInputVideo = inputVideoPath;
        // read from camera or from  file
        if (TheInputVideo.find("live") != string::npos)
        {
            int vIdx = 0;
            // check if the :idx is here
            char cad[100];
            if (TheInputVideo.find(":") != string::npos)
            {
                std::replace(TheInputVideo.begin(), TheInputVideo.end(), ':', ' ');
                sscanf(TheInputVideo.c_str(), "%s %d", cad, &vIdx);
            }
            cout << "Opening camera index " << vIdx << endl;
            cap.open(vIdx);
            cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','J','P','G'));
            cap.set(cv::CAP_PROP_FRAME_WIDTH,1920);
            cap.set(cv::CAP_PROP_FRAME_HEIGHT,1080);
            waitTime = 10;
            isVideo=true;
        }
        else{
            cap = cv::VideoCapture(inputVideoPath);
            if(!cap.isOpened()){
                auxiliarrrr<<"Cannot open the video file. \n";
                return -1;
            }
            cap.set(cv::CAP_PROP_FRAME_WIDTH,1920);
            cap.set(cv::CAP_PROP_FRAME_HEIGHT,1080);
            // Input camera
            double waitTime = 1;
            auto TheInputVideo = inputVideoPath;
        }
        // check video is open
        if (!cap.isOpened())
            throw std::runtime_error("Could not open video");
        // Video properties
        int videoWidth = 1920;//cap.get(cv::CAP_PROP_FRAME_WIDTH); global_width = videoWidth;
        int videoHeight = 1080;//cap.get(cv::CAP_PROP_FRAME_HEIGHT); global_height = videoHeight;
        auxiliarrrr<<"Video width: "<<videoWidth<<std::endl;
        auxiliarrrr<<"Video height: "<<videoHeight<<std::endl;
        cv::Size frameSize = cv::Size(videoWidth, videoHeight);


        /*~~~~~~~~~~~~~~~~~~~
            Output files
        ~~~~~~~~~~~~~~~~~~~*/
        // Poses output file
        ofstream posesFile;
        if(generatePosesOutputFile){
            posesFile.open(outputPosePath);
        }
        // Distances output file
        ofstream distancesFile;
        if(generateDistancesOutputFile){
            distancesFile.open(outputDistancePath);
        }



        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
            Generate output video preparation
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        cv::VideoWriter outputVideo;
        if(!noOutputVideo){
            outputVideo.open(outputVideoPath, cv::VideoWriter::fourcc('P', 'I', 'M', '1'), cap.get(cv::CAP_PROP_FPS), frameSize, true);
        }



        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~
            Processing the video
        ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        double zoom = 100;
        int processed_frames = 0;
        TimerAvrg detectionFPS, systemFPS;
        double valueSystemFPS = 0.0;
        std::vector<cv::Mat> outputFrames;
        bool continueProcessing = true;
        int detectionType = detectionMode; cv::String detectionTypeNames[3] = {"None", "Cucomarker", "Aruco"};
        int drawingType = 1; cv::String drawingTypeNames[5] = {"No drawing", "Minimal drawing", "3D drawing (Local pose)", "Normal drawing (global pose)", "3D drawing (global pose)"};
        bool skippedFrame = false;
        cv::String lookForMarkersNames[4] = {"None", "Only good", "Both good and bad", "3D Only good"};
        if(!minimal) cv::namedWindow("Processed frame");
        cv::Mat last_valid_pose_rvec;
        while(continueProcessing){
            systemFPS.start();

            /*~~~~~~~~~~~~~~~~~~~~
                Frame obtaining
            ~~~~~~~~~~~~~~~~~~~~*/
            // Obtaining
            cv::Mat frame;
            if (!cap.read(frame)){
                auxiliarrrr<<"\n Cannot read a new frame of the video\n";
                break;
            }
            if (!frame.empty() && (frame.cols != 1920 || frame.rows != 1080)) {
                // Resize the frame to 1920x1080 resolution if it doesn't match
                cv::resize(frame, frame, cv::Size(1920, 1080));
            }
            // Print information on terminal
            if(!minimal){
                auxiliarrrr<<std::endl<<BGREEN<<"~+~~~~~~~~~~~~~~~~~~~~ Frame: "<<processed_frames<<" ~~~~~~~~~~~~~~~~~~~~+~"<<RESET<<std::endl;
            }
            else{
                auxiliarrrr<<"Processing frame "<<processed_frames<<"..."<<std::endl;
            }
            // Print frame identification on frame
            int step=15;
            int linePoint=15; for(int i = 0; i < lineStartingShift; i++){linePoint += step;}
            if(!minimal) cv::putText(frame, "Frame: "+std::to_string(processed_frames), cv::Point2f(5,linePoint), cv::FONT_HERSHEY_PLAIN, 1, frameTextColor, 1, false); linePoint+=step;
            // Print zoom information on frame
            if(verboseFrame){
                if(!minimal) cv::putText(frame, "| Zoom: "+std::to_string(zoom)+" %", cv::Point2f(110,linePoint-step), cv::FONT_HERSHEY_PLAIN, 1, frameTextColor, 1, false);
            }


            /*~~~~~~~~~~~~~~~~~~~~~~~~~~
                Polyhuco processing
            ~~~~~~~~~~~~~~~~~~~~~~~~~~*/
            detectionFPS.start();
            cv::Mat processedFrame = frame.clone();
            polyhuco::foundInFrame foundInformation = polyhucoProcessor.findInFrame(processedFrame, cameraParamsPath, drawingType, lookForMarkers, applyRefinement); 
            auxiliarrrr<<"LookingMarkers: "<<lookForMarkers<<std::endl;
            auxiliarrrr<<"ApplyRefinement: "<<applyRefinement<<std::endl;
            
            
            std::vector<int> markersId = foundInformation.detectionInfo.foundMarkersIds;
            processedFrame = foundInformation.outputFrame.clone();
            detectionFPS.stop();


            // Draw image regions
            drawRegions(processedFrame, regions);

            /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                Printing some frame information
            ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
            // Print detection information
            double detectionTime = detectionFPS.getAvrg()*1000;
            if(verbose) auxiliarrrr<<"> Detection type: "<<detectionType<<" < ("<<detectionTypeNames[detectionType]<<")"<<std::endl;
            if(verbose) auxiliarrrr<<"> Drawing type: "<<drawingType<<" < ("<<drawingTypeNames[drawingType]<<")"<<std::endl;
            if(verbose) auxiliarrrr<<"> Detection fps: "<<detectionTime<<" |System fps: "<<valueSystemFPS<<std::endl;
            if(verbose) auxiliarrrr<<"> Nº of detected markers: "<<markersId.size()<<std::endl;
            if(verbose) auxiliarrrr<<"> Looking for markers: "<<lookForMarkers<<" < ("+lookForMarkersNames[lookForMarkers]+")"<<std::endl;
            if(verbose) auxiliarrrr<<"> Apply refinement: "<<applyRefinement<<std::endl;
            if(verboseFrame){
                if(!minimal) cv::putText(processedFrame, "Detection type: "+std::to_string(detectionType)+" < ("+detectionTypeNames[detectionType]+")", cv::Point2f(5,linePoint), cv::FONT_HERSHEY_PLAIN, 1, frameTextColor, 1, false); linePoint+=step;
                if(!minimal) cv::putText(processedFrame, "Drawing type: "+std::to_string(drawingType)+" < ("+drawingTypeNames[drawingType]+")", cv::Point2f(5,linePoint), cv::FONT_HERSHEY_PLAIN, 1, frameTextColor, 1, false); linePoint+=step;
                if(!minimal) cv::putText(processedFrame, "Detection fps: "+std::to_string(detectionTime)+" |System fps: "+std::to_string(valueSystemFPS), cv::Point2f(5,linePoint), cv::FONT_HERSHEY_PLAIN, 1, frameTextColor, 1, false); linePoint+=step;
                if(!minimal) cv::putText(processedFrame, "N. of detected markers: "+std::to_string(markersId.size()), cv::Point2f(5,linePoint), cv::FONT_HERSHEY_PLAIN, 1, frameTextColor, 1, false); linePoint+=step;
                if(!minimal) cv::putText(processedFrame, "Looking for markers: "+std::to_string(lookForMarkers)+" < ("+lookForMarkersNames[lookForMarkers]+")", cv::Point2f(5,linePoint), cv::FONT_HERSHEY_PLAIN, 1, frameTextColor, 1, false); linePoint+=step;
                if(!minimal) cv::putText(processedFrame, "Apply refinement: "+std::to_string(applyRefinement), cv::Point2f(5,linePoint), cv::FONT_HERSHEY_PLAIN, 1, frameTextColor, 1, false); linePoint+=step;
            }



            // Print detected markers ids
            std::string markersIdsString = "-";
            for(int i = 0; i < markersId.size(); i++){
                if (i == 0)
                    markersIdsString = "[ ";
                markersIdsString += std::to_string(markersId[i]);
                if (i != (markersId.size()-1))
                    markersIdsString += " | ";
                else
                    markersIdsString += " ]";
            }
            if(verbose) auxiliarrrr<<"> Detected markers ids: "<<markersIdsString<<std::endl;
            if(verboseFrame){
                if(!minimal) cv::putText(processedFrame, "Detected markers ids: "+markersIdsString, cv::Point2f(5,linePoint), cv::FONT_HERSHEY_PLAIN, 1, frameTextColor, 1, false); linePoint+=step;
            }


            /*~~~~~~~~~~~~~~~~~~~~~~
                Pose validation
            ~~~~~~~~~~~~~~~~~~~~~~*/
            if(foundInformation.found){

                if(last_valid_pose_rvec.empty()){
                    last_valid_pose_rvec = foundInformation.detectionInfo.selectedPose.rvec.clone();
                }
                else{
                    // Threshold
                    double THRESHOLD = 4;
                    // Direction
                    std::string direction = "";


                    /* POSE IMAGE POSITION */
                        cv::Point2f centerPoint = reprojectPoseCenterToImage(
                            foundInformation.detectionInfo.selectedPose.rvec.clone(),
                            foundInformation.detectionInfo.selectedPose.tvec.clone(),
                            foundInformation.detectionInfo.K,  // This should be defined and initialized earlier in your program
                            foundInformation.detectionInfo.D  // Assuming no distortion for simplicity, otherwise provide the distortion coefficients matrix
                        );
                        fi_input = centerPoint;
                        // Display the center point on the frame
                        std::string centerPos = "Center: (" + std::to_string((int)centerPoint.x) + ", " + std::to_string((int)centerPoint.y) + ")";
                        if(!minimal) cv::putText(processedFrame, centerPos, cv::Point2f(10, linePoint), cv::FONT_HERSHEY_PLAIN, 1, frameTextColor, 1, false); linePoint+=15;
                        // Draw a semitransparent white circle at the center point
                        drawSemitransparentCircle(processedFrame, centerPoint);


                    /* IMAGE REGIONS */
                    for (const auto& region : regions) {
                        if (isPointInRegion(centerPoint, region.points)) {
                            // Save direction
                            direction = region.command;
                            // Optionally display the command on the frame
                            std::string cmdText = "Command by Image region: " + direction;
                            if(!minimal) cv::putText(processedFrame, cmdText, cv::Point2f(10, linePoint + 15), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255), 1, false);
                            linePoint += 15;
                        }
                    }
                    if(direction == ""){
                        direction = "none";
                    }


                    /* ROTATIONS CHANGES */
                        cv::Vec3d rotationChange = computeAverageRotationChange(foundInformation.detectionInfo.selectedPose.rvec.clone());
                        auxiliarrrr<<"> Rotation change [0]: "<<rotationChange[0];
                        auxiliarrrr<<"> Rotation change [1]: "<<rotationChange[1];
                        // Determine the direction of the rotation
                        // DEACTIVATED
                        //direction = direction+" "+computeDirection(rotationChange, foundInformation.detectionInfo.selectedPose.rvec.clone());
                        if(!minimal) cv::putText(processedFrame, "Direction: "+direction, cv::Point2f(10,linePoint), cv::FONT_HERSHEY_PLAIN, 1, frameTextColor, 1, false); linePoint+=step;
                        // Debugging Output
                        auxiliarrrr << "Rotation Change: [" << rotationChange[0] << ", " << rotationChange[1] << ", " << rotationChange[2] << "]" << std::endl;
                        auxiliarrrr << "Determined Direction: " << direction << std::endl;


                    // Write the result to a file
                    //writeDirectionToFile(direction);
                    std::cout<< fi_input.x << "," << fi_input.y << "|" << command_id << ": " << direction << std::endl;

                    // Update the last valid pose
                    last_valid_pose_rvec = foundInformation.detectionInfo.selectedPose.rvec.clone();
                }

                /*~~~~~~~~~~~~~~~~~~~~~~~~~~
                    Output managing
                ~~~~~~~~~~~~~~~~~~~~~~~~~~*/
                // Condition to check if the frame id is a valid frame (Included in the valid frames ranges set by parameter. By default: All)
                bool validFrameId = (valid_frames.size() == 0);
                if(validFrameId == false){
                    for(int vf = 0; vf < valid_frames.size(); vf++){
                        if(processed_frames == valid_frames[vf]){
                            validFrameId = true;
                        }
                    }
                }
                // Poses approving
                bool approved_nonRefinedInfo = foundInformation.detectionInfo.woRefinement.poseApproved;
                bool approved_RefinedInfo = false; if(applyRefinement){ approved_RefinedInfo = foundInformation.detectionInfo.wRefinement.poseApproved; }
                auxiliarrrr<<"Refined approved? "<<approved_RefinedInfo<<std::endl;
                auxiliarrrr<<"nonRefined approved? "<<approved_nonRefinedInfo<<std::endl;
                // Extracting the selected pose: [refined, non-refined, none]
                if(validFrameId){
                    auxiliarrrr<<"Managing outputs..."<<std::endl;
                    cv::Mat tvec; 
                    cv::Mat rvec;
                    // There is a valid pose?
                    std::string valid_pose_id = "None";
                    if(foundInformation.detectionInfo.poseIsSelected){ // Approved refined pose?
                        valid_pose_id = foundInformation.detectionInfo.selectedPoseId;
                        auxiliarrrr<<BGREEN<<"[ "<<Symbols_Check<<GREEN<<" Approved the pose: "<<BGREEN<<valid_pose_id<<" ]"<<RESET<<std::endl;
                        tvec = foundInformation.detectionInfo.selectedPose.tvec.clone();
                        rvec = foundInformation.detectionInfo.selectedPose.rvec.clone();
                    }
                    else{
                        auxiliarrrr<<BRED<<"[ "<<Symbols_Cross<<" No pose was approved ]"<<RESET<<std::endl;
                    }
                    // Write valid pose if there is any
                    if( valid_pose_id != "None" ){
                        auxiliarrrr<<GREEN<<"\tWriting in output the pose: "<<RESET<<valid_pose_id<<std::endl;
                        if(verboseDebug) auxiliarrrr<<"Tvec: "<<tvec<<std::endl;
                        if(verboseDebug) auxiliarrrr<<"Rvec: "<<rvec<<std::endl;
                        /*~~~~~~~~~~~~~~~~~~~~~~~~~
                            Writing pose file
                        ~~~~~~~~~~~~~~~~~~~~~~~~~*/
                        if(generatePosesOutputFile){ // Poses output file?
                            //Compute quaternions
                            std::string poseLine = getPoseLine(processed_frames, rvec, tvec, verboseDebug);
                            posesFile << poseLine;  
                            if(verbose) auxiliarrrr<<BOLD<<"\t\t "<<Symbols_Check<<" Pose line written: "<<RESET<<poseLine;
                        }
                        /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                            Writing distances file
                        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
                        if(generateDistancesOutputFile){ // Distances output file?
                            // Computing
                            double distanceToObject = cv::norm(tvec, cv::NORM_L2);
                                // Not used -> double angleToObject = cv::norm(rvec, cv::NORM_L2);
                            std::string distanceStr = std::to_string(distanceToObject);
                            // Writing
                            std::string distanceLine = distanceStr+"\n";
                            distancesFile << distanceLine;
                            if(verbose) auxiliarrrr<<BOLD<<"\t\t "<<Symbols_Check<<" Distance line written: "<<RESET<<distanceLine;
                        }
                    }
                    else{
                        auxiliarrrr<<RED<<"\tNor the woRefinement and wRefinement poses have been approved"<<RESET<<std::endl;
                        auxiliarrrr<<RED<<"\t\t- No pose in output file and no distance in output file"<<RESET<<std::endl;
                    }
                }
                auxiliarrrr<<"\n\n"<<std::endl;
            }



            /*~~~~~~~~~~~~~~~~~~~~~~~~~~
                User help managing
            ~~~~~~~~~~~~~~~~~~~~~~~~~~*/
            if(verboseFrame){
                cv::putText(processedFrame, "Pulse 'h' to show/hide help", cv::Point2f(5,linePoint), cv::FONT_HERSHEY_PLAIN, 1, frameTextColor, 1, false); linePoint+=step;
            }
            // User inputs
            std::vector<cv::String> userInputsOptions;
            userInputsOptions.push_back("User input options: ");
            userInputsOptions.push_back("   Escape: Finishes the execution");
            userInputsOptions.push_back("   d: Changes detection mode");
            userInputsOptions.push_back("      - 1: "+detectionTypeNames[1]);
            userInputsOptions.push_back("      - 2: "+detectionTypeNames[2]);
            userInputsOptions.push_back("   3: Changes drawing type");
            userInputsOptions.push_back("      - 0: "+drawingTypeNames[0]);
            userInputsOptions.push_back("      - 1: "+drawingTypeNames[1]);
            userInputsOptions.push_back("      - 2: "+drawingTypeNames[2]);
            userInputsOptions.push_back("      - 3: "+drawingTypeNames[3]);
            userInputsOptions.push_back("      - 4: "+drawingTypeNames[4]);
            userInputsOptions.push_back("   r: Activates/Deactivates applying of looked markers");
            userInputsOptions.push_back("   s: Takes screenshot");
            userInputsOptions.push_back("   v: Activates/Deactivates verbose frame");
            userInputsOptions.push_back("   -: Zooms out");
            userInputsOptions.push_back("   +: Zooms in");
            userInputsOptions.push_back("   p: Pauses/Resumes processing");
            userInputsOptions.push_back("   n: Skips to the next frame (Only in pause mode)");
            userInputsOptions.push_back("   l: Looks for some good markers");
            userInputsOptions.push_back("      - 0: "+lookForMarkersNames[0]);
            userInputsOptions.push_back("      - 1: "+lookForMarkersNames[1]);
            userInputsOptions.push_back("      - 2: "+lookForMarkersNames[2]);
            userInputsOptions.push_back("      - 3: "+lookForMarkersNames[3]);
            userInputsOptions.push_back("   h: Shows/Hides help");
            // Print options on frame
            if(verboseFrame && showUserInputsMessages){
                for(int o = 0; o < userInputsOptions.size(); o++){
                    if(!minimal) cv::putText(processedFrame, userInputsOptions[o], cv::Point2f(5,linePoint), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(245, 200, 66), 1, false); linePoint+=step;
                }
            }

            // Show the results
            if (!minimal) {
                cv::Mat zoomedProcessedFrame;
                cv::resize(processedFrame, zoomedProcessedFrame, cv::Size(processedFrame.cols * (zoom/100.0), processedFrame.rows * (zoom/100.0)));
                cv::Mat flippedFrame;
                cv::flip(zoomedProcessedFrame, flippedFrame, 1); // 1 indicates a horizontal flip
                cv::imshow("Processed frame", flippedFrame);
            }

            // User input
            int key = 0;
            if(!minimal) key = cv::waitKey(1);
            // if((processed_frames == 1) && (startingPaused)){
            //     key == 112;
            // }
            // [ Escape ]
            if(key == 27){
                break;
            }
            // [ d ]
            if(key == 100){
                if(detectionType == 1){
                    detectionType = 2;
                }
                else if(detectionType == 2){
                    detectionType = 1;
                }
            }
            // [ 3 ]
            if(key == 51){
                if(drawingType < 4){
                    drawingType += 1;
                }
                else{
                    drawingType = 1;
                }
            }
            // [ r ]
            if(key == 114){
                applyRefinement = !applyRefinement;
            }
            // [ s ]
            if (key == 115) {
                std::string ssname = "screenshot.png";
                cv::imwrite(ssname, processedFrame);
            }
            // [ v ]
            if(key == 118){
                verboseFrame = !verboseFrame;
                polyhucoProcessor.setVerboses(verbose, verboseDebug, verboseFrame);
            }
            // [ - ]
            if(key == 45){
                if(zoom >= 20)
                    zoom -= 10;
                else
                    printAlert("Cannot zoom out less than 10%");
            }
            // [ + ]
            if(key == 43){
                if(zoom < 500)
                    zoom += 10;
                else
                    printAlert("Cannot zoom in more than 500%");
            }
            // [ l ]
            if(key == 108){
                if(lookForMarkers < 3){
                    lookForMarkers += 1;
                }
                else{
                    lookForMarkers = 0;
                }
            }
            // [ h ]
            if(key == 104){
                showUserInputsMessages = !showUserInputsMessages;
            }
            // [ p ]
            if(((processed_frames == 0) && (startingPaused)) or ((key == 112) or (skippedFrame == true))){
                skippedFrame = false;
                while(1){
                    int key = cv::waitKey(1);
                    if(key == 112){ // [ p ]
                        break;
                    }
                    if(key == 110){
                        skippedFrame = true;
                        break;
                    }
                    if(key == 27){ // [ Escape ]
                        continueProcessing = false;
                        break;
                    }
                    if(key == 100){ // [ d ]
                        if(detectionType == 1){
                            detectionType = 2;
                        }
                        else if(detectionType == 2){
                            detectionType = 1;
                        }
                    }
                    if(key == 51){ // [ 3 ]
                        if(drawingType < 4){
                            drawingType += 1;
                        }
                        else{
                            drawingType = 1;
                        }
                    }
                    if(key == 114){ // [ r ]
                        applyRefinement = !applyRefinement;
                    }
                    if(key == 115){ // [ s ]
                        std::string ssname = "screenshot.png";
                        cv::imwrite(ssname, processedFrame);
                    }
                    if(key == 118){ // [ v ]
                        verboseFrame = !verboseFrame;
                        polyhucoProcessor.setVerboses(verbose, verboseDebug, verboseFrame);
                    }
                    if(key == 45){ // [ - ]
                        if(zoom >= 20)
                            zoom -= 10;
                        else
                            printAlert("Cannot zoom out less than 10%");
                    }
                    if(key == 43){ // [ + ]
                        if(zoom < 500)
                            zoom += 10;
                        else
                            printAlert("Cannot zoom in more than 500%");
                    }
                    if(key == 108){ // [ l ]
                        if(lookForMarkers < 3){
                            lookForMarkers += 1;
                        }
                        else{
                            lookForMarkers = 0;
                        }
                    }
                    if(key == 104){ // [ h ]
                        showUserInputsMessages = !showUserInputsMessages;
                    }
                }
            }
            //auxiliarrrr<<"Actived key: "<<key<<std::endl;


            // End of processing each frame
            processed_frames++; command_id = std::to_string(processed_frames);
            if(!noOutputVideo){
                outputVideo << processedFrame;
            }
            systemFPS.stop(); valueSystemFPS = systemFPS.getAvrg()*1000;
        }
        // Close frame window
        if(!minimal) cv::destroyAllWindows();


        // Close output poses file if needed
        if(generatePosesOutputFile){
            posesFile.close();
        }

    }


    // Delete commands file
    if (deleteCommandFile) {
        if (std::remove("/home/paul/commands.txt") == 0) {
            auxiliarrrr << "File /home/paul/commands.txt successfully deleted." << std::endl;
        } else {
            std::cerr << "Error: Could not delete the file /home/paul/commands.txt." << std::endl;
        }
    }



    // End of execution
    return 0;
}
