#include<opencv2/core.hpp>
#include<memory>
#include<vector>
#include<string>
struct SPoint{
    uchar x;
    uchar y;
};

class StickCompress{
private:
    std::vector<int> box_;
    std::vector<cv::Point> tmp_stick_point_;  
public:
    cv::Point offset = {0,0};
    std::vector<uint16_t> stick_point;  
    int img_width;
    int img_height;

    StickCompress();
    StickCompress(std::string file_name);
    
    void reset();
    void push_stick(int x, int y);
    bool scale_to_size(int  width ,int height);
    std::vector<char> to_buf();
    void save(std::string file_name);
    cv::Mat to_cv_mat();
};

class StickFigure{
    class StickFigureImpl;
    std::unique_ptr<StickFigureImpl> impl_;
public:
    StickFigure();
    ~StickFigure();
    bool is_stick_figure(cv::Mat src, int threshold);
    cv::Mat to_binary_stick(int threshold);
    StickCompress& compress();
private:
};

