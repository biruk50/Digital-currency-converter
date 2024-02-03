
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

using namespace std;
using namespace cv;

int main() {
    VideoCapture video(1);
    if (!video.isOpened()) {
        cerr << "Error: Unable to open camera." << endl;
        return -1;
    }

    int max = 0;        // Variable to store the maximum face count
    Mat maxImage;       // Mat to store the image with the maximum face count

    // Set the initial time
    auto startTime = chrono::steady_clock::now();

    while (true) {
        // Check if 3 seconds have elapsed
        auto currentTime = chrono::steady_clock::now();
        auto elapsedTime = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();

        if (elapsedTime >= 3) {
            // Capture a photo every 3 seconds
            Mat img;
            CascadeClassifier facedetect;
            facedetect.load("haarcascade_frontalface_alt.xml");
            video.read(img);

            vector<Rect> faces;
            facedetect.detectMultiScale(img, faces, 1.3, 5);

            // Update the maximum face count
            if (faces.size() > max) {
                max = faces.size();
                maxImage = img.clone();  // Save the image with the maximum face count
            }

            for (int i = 0; i < faces.size(); i++) {
                rectangle(img, faces[i].tl(), faces[i].br(), Scalar(50, 50, 255), 3);
            }

            // Display the current and maximum face count
            putText(img, "Current: " + to_string(faces.size()) + " Max: " + to_string(max),
                Point(10, 40), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 0), 1);

            namedWindow("Digit Detection", WINDOW_NORMAL);
            resizeWindow("Digit Detection", 800, 600);  // Adjust the size as needed
            imshow("Digit Detection", img);

            // Reset the timer
            startTime = chrono::steady_clock::now();
        }

        // Break the loop if the 'Esc' key is pressed
        if (waitKey(1) == 27) {
            // Save the image with the maximum face count
            string path = "C:/Users/localuser/source/repos/currency";
            if (!maxImage.empty()) {
                imwrite(path + "/max_face_count_image.jpg", maxImage);
                cout << "Image with the maximum face count saved as max_face_count_image.jpg" << endl;
            }
            break;
        }
    }

    return 0;
}
