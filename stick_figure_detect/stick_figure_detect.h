#include<opencv2/core.hpp>

class StickFigure{
public:
    StickFigure();
    bool is_stick_figure(cv::Mat src, int threshold);
    cv::Mat to_binary_stick(int threshold);
private:
    cv::Mat src;
    cv::Mat hist[3]; 
    int  max_count_black[6] = {0};
    int  max_count_white[6] = {0};
};

