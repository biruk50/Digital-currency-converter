#include <opencv2/opencv.hpp>
#include <iostream>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <string>
#include <cstdlib>
#include <curl/curl.h>
#include <regex>

using namespace std;
using namespace cv;

string api_key = "LH955NFH1QXRXOBN";

// Constants for char detection
const int MIN_PIXEL_AREA = 100;
const int MIN_PIXEL_WIDTH = 5;
const int MIN_PIXEL_HEIGHT = 10;
const double MIN_ASPECT_RATIO = 0.2;
const double MAX_ASPECT_RATIO = 2.0;

int actual_number;

struct PossibleChar {
    Rect boundingRect;
    double dblAspectRatio;
};
// Struct to store the API response

struct ApiResponse {
    string data;
    CURLcode result;
};
//functions for the currency converter
double RealTimeCurrencyExchangeRate(const string& from_currency, const string& to_currency, const string& api_key);
size_t WriteCallback(void* contents, size_t size, size_t nmemb, ApiResponse* response);
ApiResponse PerformGetRequest(const string& url);

bool checkIfPossibleChar(PossibleChar& possibleChar, const Mat& image);

string getCurrencyCode(int index);
int main() {
     cout<<"\t\t\t***********************************************************************"<<endl;
	 cout<<"\t\t\t                       WELCOME TO Our Digital Currency Converter                    "<<endl;
     cout<<"\t\t\t***********************************************************************"<<endl<<endl<<endl;
     int i,j;
     string from_currency,to_currency;
     cout<< "Enter the number that corresponds to the currency you want to be detected:\n"
            "1.United States Dollar(USD)\n2.Ethiopian Birr(ETB)\n3.Euro(EUR)\n4.Japanese Yen(JPY)\n5.Russian Ruble(RUB)\n\n";
     cin >> i;

     from_currency = getCurrencyCode(i);
     cout<< "Enter the number that corresponds to the currency you want converted to:\n"
             "1.United States Dollar(USD)\n2.Ethiopian Birr(ETB)\n3.Euro(EUR)\n4.Japanese Yen(JPY)\n5.Russian Ruble(RUB)\n";
     cin >> j;
     to_currency = getCurrencyCode(j);

     double rate = RealTimeCurrencyExchangeRate(from_currency, to_currency, api_key);
     cout << rate;
     
    VideoCapture video(1);

    while (true) {
        Mat img;
        video.read(img);

        // Convert the frame to grayscale
        Mat gray;
        cvtColor(img, gray, COLOR_BGR2GRAY);

        // Apply thresholding to create a binary image
        Mat binary;
        threshold(gray, binary, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);

        // Find contours in the binary image
        vector<vector<Point>> contours;
        findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        // Vector to store possible characters
        vector<PossibleChar> possibleChars;

   
        // Iterate through contours
        for (const auto& contour : contours) {
            // Create a PossibleChar structure only if the contour is valid
            if (contour.size() > 0) {
                PossibleChar possibleChar;
                possibleChar.boundingRect = boundingRect(contour);
                possibleChar.dblAspectRatio = static_cast<double>(possibleChar.boundingRect.width) /
                    static_cast<double>(possibleChar.boundingRect.height);

                // Check if it's a possible char
                if (checkIfPossibleChar(possibleChar, img)) {
                    possibleChars.push_back(possibleChar);
                }
            }
        }

        if (!possibleChars.empty()) {
            // Find the boundingRect with the longest matching characters
            Rect longestBoundingRect = possibleChars[0].boundingRect;

            for (const auto& possibleChar : possibleChars) {
                // Compare boundingRects and keep the one with the longest matching characters
                if (possibleChar.boundingRect.width > longestBoundingRect.width) {
                    longestBoundingRect = possibleChar.boundingRect;
                }
            }
            rectangle(img, longestBoundingRect, Scalar(0, 255, 0), 2);
            // OCR recognition on the region of interest (longestBoundingRect)
            tesseract::TessBaseAPI api;
            api.Init(NULL, "eng", tesseract::OEM_LSTM_ONLY);
            api.SetImage(img(longestBoundingRect).data, longestBoundingRect.width, longestBoundingRect.height, 3, img.step);
            api.SetVariable("tessedit_char_whitelist", "0123456789.");
            char* outText = api.GetUTF8Text();
            actual_number = atoi(outText);
            double changed_number = actual_number * rate;

            putText(img, to_string(actual_number) + " " + from_currency + " is " + to_string(changed_number) + " " + to_currency,
                Point(10, 40), FONT_HERSHEY_COMPLEX, 1, Scalar(0, 0, 255), 1);
            namedWindow("Digit Detection", WINDOW_NORMAL);
            resizeWindow("Digit Detection", 800, 600);  // Adjust the size as needed
            imshow("Digit Detection", img);
            if (actual_number > 0) {
                cout << actual_number << " " << from_currency << " is " << changed_number << " " << to_currency << endl;
            }
            delete[] outText;
            api.End();

        }
        // Break the loop if the 'Esc' key is pressed
        waitKey(1);
    }
    return 0;
}

string getCurrencyCode(int index) {
    switch (index) {
    case 1: return "USD";
    case 2: return "ETB";
    case 3: return "EUR";
    case 4: return "JPY";
    case 5: return "RUB";
    default: return "Invalid";
    }
}

double RealTimeCurrencyExchangeRate(const string& from_currency, const string& to_currency, const string& api_key) {
    // Base URL for Alpha Vantage API
    string base_url = "https://www.alphavantage.co/query?function=CURRENCY_EXCHANGE_RATE";

    // Complete URL with parameters
    string main_url = base_url + "&from_currency=" + from_currency +
        "&to_currency=" + to_currency + "&apikey=" + api_key;

    // Perform the GET request
    ApiResponse apiResponse = PerformGetRequest(main_url);

    // Check for errors
    if (apiResponse.result != CURLE_OK) {
        cerr << "Error performing curl request: " << curl_easy_strerror(apiResponse.result) << endl;
    }
    else {
        //cout << "Result before parsing the JSON data:\n" << apiResponse.data << endl;
        // Parse JSON data here (you may use a JSON library or parse manually)
        regex pattern("\"5\\. Exchange Rate\": \"([^\"]+)\"");
        smatch match;
        if (regex_search(apiResponse.data, match, pattern)) {
            // Extract the "Exchange Rate" value from the regex match
            string exchangeRateString = match[1].str();
            double exchangeRate = stod(exchangeRateString);
            return exchangeRate;
        }
    }
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, ApiResponse* response) {
    size_t totalSize = size * nmemb;
    response->data.append((char*)contents, totalSize);
    return totalSize;
}

// Function to perform a GET request and return the API response
ApiResponse PerformGetRequest(const string& url) {
    CURL* curl = curl_easy_init();
    ApiResponse apiResponse;

    if (curl) {
        // Set libcurl options
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Set callback function for writing response data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &apiResponse);

        // Perform the request
        apiResponse.result = curl_easy_perform(curl);

        // Clean up libcurl
        curl_easy_cleanup(curl);
    }
    else {
        cerr << "Error initializing libcurl" << endl;
    }
    return apiResponse;
}

bool checkIfPossibleChar(PossibleChar& possibleChar, const Mat& image) {
    // Check if the boundingRect is within the image boundaries
    if (possibleChar.boundingRect.x < 0 || possibleChar.boundingRect.y < 0 ||
        possibleChar.boundingRect.x + possibleChar.boundingRect.width > image.cols ||
        possibleChar.boundingRect.y + possibleChar.boundingRect.height > image.rows) {
        return false;
    }

    return (possibleChar.boundingRect.area() > MIN_PIXEL_AREA &&
        possibleChar.boundingRect.width > MIN_PIXEL_WIDTH && possibleChar.boundingRect.height > MIN_PIXEL_HEIGHT &&
        MIN_ASPECT_RATIO < possibleChar.dblAspectRatio && possibleChar.dblAspectRatio < MAX_ASPECT_RATIO);
}
