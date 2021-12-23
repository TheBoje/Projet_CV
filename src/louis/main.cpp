#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/core/types.hpp>

#include <iostream>
#include <string>

int r_val = 0;
int g_val = 0;
int b_val = 0;

static void onMouseRGB(int event, int x, int y, int, void *udata)
{
    switch (event)
    {
    case cv::EVENT_LBUTTONDOWN:
        cv::Mat *image = (cv::Mat *)udata;
        cv::Vec3b val = (*image).at<cv::Vec3b>(y, x);
        std::cout << "RGBValue @ " << x << " " << y << " : " << std::to_string(val[0]) << " " << std::to_string(val[1]) << " " << std::to_string(val[2]) << std::endl;
        break;
    }
}

static void onMouseGRAY(int event, int x, int y, int, void *udata)
{
    switch (event)
    {
    case cv::EVENT_LBUTTONDOWN:
        cv::Mat *image = (cv::Mat *)udata;
        uchar val = (*image).at<uchar>(y, x);
        std::cout << "GValue @ " << x << " " << y << " : " << std::to_string(val) << std::endl;
        break;
    }
}

void printPointedPixel(std::string winname, cv::Mat *image)
{
    switch (image->type())
    {
    case CV_8UC1:
        cv::setMouseCallback(winname, onMouseGRAY, image);
        break;
    case CV_8UC3:
        cv::setMouseCallback(winname, onMouseRGB, image);
        break;
    }
}

cv::Mat histogramme(cv::Mat img, cv::Mat mask = cv::Mat())
{
    cv::Mat result = cv::Mat::zeros({256}, 1, CV_32SC1);

    //	for (int i = 0; i < img.rows; i++) {
    //		for (int j = 0; j < img.cols; j++) {
    //			uchar value = img.at<uchar>(i, j);
    //			result.at<int>(value)++;
    //		}
    //	}

    int channels[] = {0};
    int hisSize[] = {256};
    float sr[] = {0, 256};
    const float *ranges[] = {sr};
    cv::calcHist(&img, 1, channels, mask, result, 1, hisSize, ranges);
    return result;
}

void printHist(cv::Mat hist)
{
    for (int i = 0; i < hist.rows; i++)
    {
        std::cout << hist.at<float>(i, 0) << std::endl;
    }
}

cv::Mat getHistImg(cv::Mat hist)
{
    double minVal = 0;
    double maxVal = 0;
    cv::minMaxLoc(hist, &minVal, &maxVal);
    cv::Mat result = cv::Mat::zeros(256, 256, CV_8UC1);

    for (int i = 0; i < hist.rows; i++)
    {
        if (hist.at<float>(i, 0) != 0)
        {
            cv::Point pt1(0, i);
            cv::Point pt2(cvRound(hist.at<float>(i, 0) * (256 / maxVal)), i);
            cv::rectangle(result, pt1, pt2, 255);
        }
    }

    cv::rotate(result, result, cv::ROTATE_90_COUNTERCLOCKWISE);

    return result;
}

cv::Mat stretchHist(cv::Mat img, int maxPos = 0)
{
    cv::Mat hist = histogramme(img);

    double minPos = 0;

    // calc minpos
    for (int i = 0; i < hist.rows; i++)
    {
        if (hist.at<float>(i, 0) != 0)
        {
            minPos = i;
            break;
        }
    }

    // calv maxpos
    // si maxpos n'a pas été spécifié
    if (maxPos == 0)
    {
        for (int i = hist.rows - 1; i >= 0; i--)
        {
            if (hist.at<float>(i, 0) != 0)
            {
                maxPos = i;
                break;
            }
        }
    }

    std::cout << "minPos : " << minPos << " maxPos : " << maxPos << std::endl;

    for (int i = 0; i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            float val = img.at<uchar>(i, j);
            img.at<uchar>(i, j) = (uchar)(((val - minPos) / (maxPos - minPos)) * 255);
        }
    }

    return img;
}

cv::Mat getHistC(cv::Mat hist)
{
    cv::Mat result = cv::Mat::zeros(256, 1, CV_32SC1);
    int count = 0;
    for (int i = 0; i < hist.rows; i++)
    {
        float val = hist.at<float>(i, 0);
        count += val;

        result.at<float>(i, 0) = count;
    }
    return result;
}

cv::Mat applyWhileBalance(const cv::Mat &img, const cv::Mat &zone, const cv::Mat &mask = cv::Mat())
{
    cv::Mat imgBalanced = img.clone();
    cv::Scalar avg = mean(zone, mask);
    std::cout << "here" << std::endl;

    cv::Mat greyPatch(1, 1, CV_8UC3, avg);
    cvtColor(greyPatch, greyPatch, cv::COLOR_BGR2GRAY);

    cv::Scalar avgGrey = mean(greyPatch);

    std::cout << "avgGrey[0] : " << avgGrey[0] << std::endl;

    int nbSaturatedChannel[] = {0, 0, 0};

    double resize[] = {
        avgGrey[0] / (double)avg[0],
        avgGrey[0] / (double)avg[1],
        avgGrey[0] / (double)avg[2]};

    for (int i = 0; i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            cv::Vec3b px = img.at<cv::Vec3b>(i, j);

            for (int channel = 0; channel < 3; channel++)
            {
                px.val[channel] = (px.val[channel] * resize[channel]);
                if (px.val[channel] > 255)
                {
                    px.val[channel] = 255;
                    nbSaturatedChannel[channel]++;
                }
            }

            imgBalanced.at<cv::Vec3b>(i, j) = px;
        }
    }

    for (int channel = 0; channel < 3; channel++)
    {
        if (nbSaturatedChannel[channel] > 0)
        {
            std::cerr << nbSaturatedChannel[channel] << " saturations dans le canal : " << channel << std::endl;
        }
    }

    return imgBalanced;
}

cv::Mat whiteBalance(const cv::Mat &input, const cv::Mat &zone, int min_threshold = 20, int max_threshold = 235)
{
    cv::Mat mask, t1, t2;

    cv::threshold(zone, t1, min_threshold, 1, cv::THRESH_BINARY);
    cv::threshold(zone, t2, max_threshold, 1, cv::THRESH_BINARY_INV);

    mask = t1.mul(t2);

    cv::Mat split[3];

    cv::split(mask, split);

    mask = split[0].mul(split[1].mul(split[2]));

    cv::imshow("mask", mask * 255);

    return applyWhileBalance(input, zone, mask);
}

std::vector<cv::Mat> hist3D(cv::Mat input, cv::Mat mask = cv::Mat())
{
    cv::Mat channels[3];
    cv::split(input, channels);
    std::vector<cv::Mat> vect;
    int maxPos = 0;
    cv::Mat tmp[3];
    cv::split(input, tmp);
    cv::Mat hist;
    for (int j = 0; j < 3; j++)
    {
        hist = histogramme(tmp[j]);
        for (int i = hist.rows - 1; i >= 0; i--)
        {
            if (hist.at<float>(i, 0) != 0 && i >= maxPos)
            {
                maxPos = i;
                std::cout << "maxPos is now : " << maxPos << std::endl;
                break;
            }
        }
    }
    std::cout << "maxPos : " << maxPos << std::endl;

    cv::Mat tmp_array[3];
    for (int i = 0; i < 3; i++)
    {
        cv::Mat tmp;
        cv::equalizeHist(channels[i], tmp);
        cv::Mat tmp2 = stretchHist(channels[i], maxPos);
        // cv::Mat tmp2 = stretchHist(tmp, maxPos);
        tmp_array[i] = tmp2;
        vect.push_back(getHistImg(histogramme(tmp2, mask)));
    }
    cv::Mat array[3];
    std::copy(vect.begin(), vect.end(), array);
    // cv::imshow("hist1", array[0]);
    // cv::imshow("hist2", array[1]);
    // cv::imshow("hist3", array[2]);
    cv::Mat res;
    cv::Mat res_tmp;
    cv::merge(array, 3, res);
    cv::merge(tmp_array, 3, res_tmp);
    // cv::imshow("equalized", res_tmp);
    std::vector<cv::Mat> res_arr;
    res_arr.push_back(res);
    res_arr.push_back(res_tmp);
    return res_arr;
}

void update(cv::Mat *img)
{
    cv::Mat channels[3];
    cv::split(*img, channels);
    cv::threshold(channels[0], channels[0], b_val, 255, cv::THRESH_BINARY);
    cv::threshold(channels[1], channels[1], g_val, 255, cv::THRESH_BINARY);
    cv::threshold(channels[2], channels[2], r_val, 255, cv::THRESH_BINARY);
    cv::Mat res;
    cv::merge(channels, 3, res);
    cv::imshow("white_str_eq", res);
}

static void on_trackbar_r(int pos, void *ud)
{
    r_val = pos;
    update((cv::Mat *)ud);
}

static void on_trackbar_g(int pos, void *ud)
{
    g_val = pos;
    update((cv::Mat *)ud);
}

static void on_trackbar_b(int pos, void *ud)
{
    b_val = pos;
    update((cv::Mat *)ud);
}

void equalizeHistWithMask(const cv::Mat &src, cv::Mat &dst, cv::Mat mask = cv::Mat())
{
    int cnz = countNonZero(mask);
    if (mask.empty() || (cnz == src.rows * src.cols))
    {
        equalizeHist(src, dst);
        return;
    }

    dst = src.clone();

    // Histogram
    std::vector<int> hist(256, 0);
    for (int r = 0; r < src.rows; ++r)
    {
        for (int c = 0; c < src.cols; ++c)
        {
            if (mask.at<uchar>(r, c))
            {
                hist[src.at<uchar>(r, c)]++;
            }
        }
    }

    // Cumulative histogram
    float scale = 255.f / float(cnz);
    std::vector<uchar> lut(256);
    int sum = 0;
    for (int i = 0; i < hist.size(); ++i)
    {
        sum += hist[i];
        lut[i] = cv::saturate_cast<uchar>(sum * scale);
    }

    // Apply equalization
    for (int r = 0; r < src.rows; ++r)
    {
        for (int c = 0; c < src.cols; ++c)
        {
            if (mask.at<uchar>(r, c))
            {
                dst.at<uchar>(r, c) = lut[src.at<uchar>(r, c)];
            }
        }
    }
}

// cv::Mat getMaskFromImage(const cv::Mat& input, cv::Point3_<uchar> col) {
    
// }

// cv::Mat medianFilter(const cv::Mat & input) {

// }

void TP1()
{
    std::string winname = "main";
    std::string winname2 = "main2";
    std::string winname3 = "main3";
    std::string winname4 = "main4";
    std::string imageName("img.png");

    cv::Mat image, grayImage;
    image = cv::imread(imageName, cv::IMREAD_COLOR);
    cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
    // cv::namedWindow(winname, cv::WINDOW_AUTOSIZE);
    // cv::namedWindow(winname2, cv::WINDOW_AUTOSIZE);
    //	cv::Mat hist = histogramme(grayImage);
    //	cv::Mat imgHist = getHistImg(hist);
    cv::Mat equalizedImg = stretchHist(grayImage);
    //	cv::Mat equalizedHist = getHistImg(histogramme(equalizedImg));

    cv::Mat mask = cv::Mat::zeros(equalizedImg.rows + 2, equalizedImg.cols + 2, CV_8UC1);
    cv::floodFill(equalizedImg, mask, cv::Point(0, 0), 1, 0, cv::Scalar(), cv::Scalar(), cv::FLOODFILL_MASK_ONLY);
    cv::floodFill(equalizedImg, mask, cv::Point(equalizedImg.cols - 1, 0), 1, 0, cv::Scalar(), cv::Scalar(), cv::FLOODFILL_MASK_ONLY);
    cv::floodFill(equalizedImg, mask, cv::Point(0, equalizedImg.rows - 1), 1, 0, cv::Scalar(), cv::Scalar(), cv::FLOODFILL_MASK_ONLY);
    cv::floodFill(equalizedImg, mask, cv::Point(equalizedImg.cols - 1, equalizedImg.rows - 1), 1, 0, cv::Scalar(), cv::Scalar(), cv::FLOODFILL_MASK_ONLY);

    mask = 1 - mask;
    mask.adjustROI(-1, -1, -1, -1);

    // cv::Mat hist2D = getHistImg(histogramme(equalizedImg, mask));

    // cv::Mat equalizedImage = cv::Mat();
    // cv::equalizeHist(grayImage, equalizedImage);
    // cv::imshow("image_gray", grayImage);
    // cv::imshow("gray_equalized", equalizedImage);

    // cv::Mat equalizedImgHist = getHistImg(histogramme(equalizedImage, mask));

    // cv::Mat histC = getHistC(histogramme(equalizedImage));
    // printHist(histC);
    // cv::imshow("histC", getHistImg(histC));
    // cv::imshow(winname3, equalizedImgHist);
    // cv::imshow(winname4, equalizedImage);

    cv::Mat res_white = whiteBalance(image, image(cv::Rect(0, 0, 200, 200)));
    // cv::Mat res_white = whiteBalance(image, image(cv::Rect(image.cols - 200, image.rows - 200, 200, 200)));
    // cv::Mat res_white = whiteBalance(image, image, 10, 240);
    // cv::imshow(winname, image);
    // cv::imshow("res_white", res_white);

    // cv::Mat hist3D_white = hist3D(res_white, mask);
    // std::vector<cv::Mat> arr_white = hist3D(res_white, mask);

    // cv::imshow("hist3D_white_mask", arr_white[0]);
    // cv::imshow("white_str_eq", arr_white[1]);
    // cv::createTrackbar("B", "white_str_eq", &b_val, 255, on_trackbar_b, &arr_white[1]);
    // cv::createTrackbar("R", "white_str_eq", &r_val, 255, on_trackbar_r, &arr_white[1]);
    // cv::createTrackbar("G", "white_str_eq", &g_val, 255, on_trackbar_g, &arr_white[1]);

    // cv::imshow("hist3D_white", hist3D(res_white));
    // cv::imshow("hist3D_default", hist3D(image));
    // cv::imshow("hist3D_default_mask", hist3D(image, mask));

    // cv::imshow("white_str_eq", res_white);
    // cv::createTrackbar("B", "white_str_eq", &b_val, 255, on_trackbar_b, &res_white);
    // cv::createTrackbar("G", "white_str_eq", &g_val, 255, on_trackbar_g, &res_white);
    // cv::createTrackbar("R", "white_str_eq", &r_val, 255, on_trackbar_r, &res_white);

    cv::Mat white_hls;
    cv::cvtColor(res_white, white_hls, cv::COLOR_BGR2HLS_FULL);
    // cv::imshow("white", res_white);
    // cv::imshow("white_HLS", white_hls);
    cv::Mat white_hls_split[3];
    cv::split(white_hls, white_hls_split);

    cv::Mat hist_hls[3];
    for (int i = 0; i < 3; i++)
    {
        hist_hls[i] = getHistImg(histogramme(white_hls_split[i], mask));
        // cv::imshow("split_" + std::to_string(i), white_hls_split[i]);
        // cv::imshow("hist_" + std::to_string(i), hist_hls[i]);
    }

    // Equalisation d'histogramme L
    cv::Mat equalised_channels_hls[3];
    equalised_channels_hls[0] = white_hls_split[0];
    equalised_channels_hls[2] = white_hls_split[2];
    cv::Mat res_eq_mask;
    equalizeHistWithMask(white_hls_split[1], res_eq_mask, mask);
    equalizeHistWithMask(white_hls_split[1], equalised_channels_hls[1]);
    cv::Mat equalised_hls_no_mask;
    cv::merge(equalised_channels_hls, 3, equalised_hls_no_mask);
    cv::Mat to_show;
    cv::cvtColor(equalised_hls_no_mask, to_show, cv::COLOR_HLS2BGR_FULL);
    cv::imshow("white", white_hls);
    cv::imshow("equalised_hLs_no_mask", to_show);
    cv::Mat equalised_hls_mask;
    cv::Mat equalised_channels_hls_mask[3];
    equalised_channels_hls_mask[0] = equalised_channels_hls[0];
    equalised_channels_hls_mask[1] = res_eq_mask;
    equalised_channels_hls_mask[2] = equalised_channels_hls[2];
    cv::merge(equalised_channels_hls_mask, 3, equalised_hls_mask);
    cv::Mat to_show2;
    cv::cvtColor(equalised_hls_mask, to_show2, cv::COLOR_HLS2BGR_FULL);
    cv::imshow("equalised_hLs_mask", to_show2);
    cv::Mat hist_hls_eq = getHistImg(histogramme(equalised_channels_hls_mask[1], mask));
    cv::imshow("hist_hls_eq", hist_hls_eq);
}

void TP2()
{
    std::string imageName("img2.png");

    cv::Mat image, blur;
    image = cv::imread(imageName, cv::IMREAD_COLOR);
    cv::imshow("default", image);
    // cv::blur(image, blur, cv::Size(3, 3));
    // cv::blur(image, blur, cv::Size(5, 5));
    // cv::blur(image, blur, cv::Size(7, 7));
    // cv::blur(blur, blur, cv::Size(3, 3));
    // cv::GaussianBlur(image, blur, cv::Size(7, 7), 5);
    // cv::bilateralFilter(image, blur, 10, 200, 200);

    cv::medianBlur(image, blur, 3);
    cv::medianBlur(blur, blur, 3);
    cv::medianBlur(blur, blur, 3);

    cv::Mat grayImage;
    cv::cvtColor(blur, grayImage, cv::COLOR_BGR2GRAY);
    cv::Mat equalizedImg = stretchHist(grayImage);
    cv::Mat mask = cv::Mat::zeros(equalizedImg.rows + 2, equalizedImg.cols + 2, CV_8UC1);
    cv::floodFill(equalizedImg, mask, cv::Point(0, 0), 1, 0, cv::Scalar(), cv::Scalar(), cv::FLOODFILL_MASK_ONLY);
    cv::floodFill(equalizedImg, mask, cv::Point(equalizedImg.cols - 1, 0), 1, 0, cv::Scalar(), cv::Scalar(), cv::FLOODFILL_MASK_ONLY);
    cv::floodFill(equalizedImg, mask, cv::Point(0, equalizedImg.rows - 1), 1, 0, cv::Scalar(), cv::Scalar(), cv::FLOODFILL_MASK_ONLY);
    cv::floodFill(equalizedImg, mask, cv::Point(equalizedImg.cols - 1, equalizedImg.rows - 1), 1, 0, cv::Scalar(), cv::Scalar(), cv::FLOODFILL_MASK_ONLY);

    mask = 1 - mask;
    mask.adjustROI(-1, -1, -1, -1);
    // Histogramme 3D
    cv::Mat channels[3];
    cv::split(image, channels);
    channels[0] = getHistImg(histogramme(channels[0], mask));
    channels[1] = getHistImg(histogramme(channels[1], mask));
    channels[2] = getHistImg(histogramme(channels[2], mask));
    cv::Mat res;
    cv::merge(channels, 3, res);
    cv::imshow("hist blured", res);
    cv::imshow("blured", blur);
}

int main()
{
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_SILENT);

    TP2();

    cv::waitKey(0);

    system("pause");
    return EXIT_SUCCESS;
}
